/*
        Thymio-II Firmware

        Copyright (C) 2011 Philippe Retornaz <philippe dot retornaz at epfl dot ch>,
        Mobots group (http://mobots.epfl.ch), Robotics system laboratory (http://lsro.epfl.ch)
        EPFL Ecole polytechnique federale de Lausanne (http://www.epfl.ch)

        See authors.txt for more details about other contributors.

        This program is free software: you can redistribute it and/or modify
        it under the terms of the GNU Lesser General Public License as published
        by the Free Software Foundation, version 3 of the License.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU Lesser General Public License for more details.

        You should have received a copy of the GNU Lesser General Public License
        along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <types/types.h>
#include <skel-usb.h>

#include "ground_ir.h"

#define PULSE_L _LATD6
#define PULSE_R _LATE4

#define CALIB_HISTERES 20
#define DEFAULT_CALIB 0x7FFF

static unsigned char prox_calib_max_counter[2];

static int _calib(int value, int i) {
	int ret;
	
	if (settings.prox_ground_max[i] == DEFAULT_CALIB)//default value sould be low for max
		settings.prox_ground_max[i]=0;
	if(value + CALIB_HISTERES > settings.prox_ground_max[i]) {
		if(++prox_calib_max_counter[i] > 3) {
			if(value > settings.prox_ground_max[i]) {
				settings.prox_ground_max[i] = value;
				set_save_settings();
			} else
				prox_calib_max_counter[i] = 0;
		}
	} else
		prox_calib_max_counter[i] = 0;
	// Cast to unsigned so the compiler optimise by a shift
	// We checked that the value was positive, so it's safe.

	if (settings.prox_ground_max[i] == 0)
		ret = value;
	else
		ret =(long)value * 1024 / (settings.prox_ground_max[i]);

	if(ret < 0)
		ret = 0;

	return ret;
}

static int perform_calib(unsigned int raw, int i) {
	int value;
	if(raw > 32767)
		return 0; // Sanity check
	
	value = raw;

	if(settings.prox_ground_max[i] >= 0) {
			// On the fly recalibration
			return _calib(value,i);
	} else {
			// Calibration disabled
			return value;
	}
}

void ground_ir_new(unsigned int r, unsigned int l, unsigned int time) {
	switch(time) {
	// +50 to be phase shifted from the horizontal prox & motor
		case 50:
			vmVariables.ground_ambiant[0] = r;
			vmVariables.ground_ambiant[1] = l;
			PULSE_R = 1;
			break;
		case 53:
			vmVariables.ground_reflected[0] = r;
			vmVariables.ground_delta[0] = perform_calib(r - vmVariables.ground_ambiant[0],0);
			PULSE_R = 0;
			PULSE_L = 1;
			break;
		case 56: 
			vmVariables.ground_reflected[1] = l;
			vmVariables.ground_delta[1] = perform_calib(l - vmVariables.ground_ambiant[1],1);
			PULSE_L = 0;

			SET_EVENT(EVENT_PROX);
			
			break;
	}	
}

void ground_ir_shutdown(void) {
	PULSE_L = 0;
	PULSE_R = 0;
}
