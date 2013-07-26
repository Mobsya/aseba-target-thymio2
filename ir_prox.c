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

#include "ir_prox.h"
#include "regulator.h"
#include "sound.h"

#include <skel-usb.h>


#define N_SENSORS 7
#define DEFAULT_CALIB 5000


static unsigned char update_calib;

// I don't want gcc to optimise too much, force noinline
static int __attribute((noinline)) ic_bufne(int ic) {
	IC1CON1BITS * ic_ptr =(IC1CON1BITS *) &IC1CON1bits + 4*ic; // Memory map: con1, con2, buf, tmr
	return ic_ptr->ICBNE;
}

static unsigned int __attribute((noinline)) ic_buf(int ic) {
	return *((&IC1BUF) + ic*4);
}

static void __attribute((noinline)) ic_reset_timer(int ic) {
	IC1CON2BITS * ic_ptr = (IC1CON2BITS *) &IC1CON2bits + 4 * ic;
	ic_ptr->TRIGSTAT = 0;
}

static int perform_calib(int raw, int i) {
        if(settings.prox_min[i] > 0) {
                // On the fly recalibration
                if(raw < settings.prox_min[i]) {
                        settings.prox_min[i] = raw;
                        update_calib = 1;
                }
                // Cast to unsigned so the compiler optimise by a shift
                // We checked that the value was positive, so it's safe.
                return raw - (3*((unsigned int) settings.prox_min[i])) / 4 + 800;

        } else {
                // Calibration disabled
                return raw;
        }
}

void ir_prox_mesure(void) {
	int temp[2];
	int i;	
	for(i = 0; i < N_SENSORS; i++) {
		if(ic_bufne(i)) {
			temp[0] = ic_buf(i);
			if(ic_bufne(i)) {
				temp[1] = ic_buf(i);
							
				// Validity check
				switch (i) {
				case 0 ... 4:
					if(temp[0] < 1560 && temp[0] > 1100) 
						vmVariables.prox[i] = perform_calib(temp[1] - temp[0],i);
					else
						vmVariables.prox[i] = 0;
					break;
				case 5 ... 6:
					if(temp[0] < 2550 && temp[0] > 2080)
						vmVariables.prox[i] = perform_calib(temp[1] - temp[0],i);
					else
						vmVariables.prox[i] = 0;
					break;
				}
				
				// Make sure we have an empty buffer ...
				while(ic_bufne(i))
					ic_buf(i);
			} else 
				vmVariables.prox[i] = 0;
		} else {
                        vmVariables.prox[i] = 0;
		}
		
		ic_reset_timer(i);
	}

	SET_EVENT(EVENT_PROX);
	
	// Retrigger everything ...
	OC7CON1 = 0;
	OC8CON1 = 0;
	OC7CON1bits.OCTSEL = 0x7; // CPU clock
	
	OC7CON1bits.OCM = 4;
	
	OC8CON1 = OC7CON1;
	
	// IR sensors are producing noise when switched on.
	// tell the mic to ignore the next sample
	sound_ignore_next_sample();
}


void prox_init(void) {
        int i;
	va_get();

        // brand-new robots will have a settings to 0
        // Thus put the maximum minimum value in order to start the calibration
        // from a valid point.
        for(i = 0; i < N_SENSORS; i++)
            if(settings.prox_min[i] == 0) {
                settings.prox_min[i] = DEFAULT_CALIB;
            }

	
	// IC configuration
	
	IC1CON1bits.ICSIDL = 0;
	IC1CON1bits.ICTSEL = 7; // Fcy as clock source

	IC1CON2bits.TRIGSTAT = 0; // Don't start the timer
	IC1CON2bits.ICTRIG = 1; // Start the timer based on the source
	
	IC1CON2bits.SYNCSEL = 7; // Trigger source is OC7

	IC1CON1bits.ICM = 0x1; // capture rising and falling ...
	
	IC2CON2 = IC1CON2; // IR2
	IC2CON1 = IC1CON1; 
	
	IC3CON2 = IC1CON2; // IR3
	IC3CON1 = IC1CON1;
	
	IC4CON2 = IC1CON2; // IR4
	IC4CON1 = IC1CON1;
	
	IC5CON2 = IC1CON2; // IR5
	IC5CON1 = IC1CON1;
	
	IC6CON2 = IC1CON2; // TODO FIXME use the OC used for the back ... 
	IC6CON1 = IC1CON1;
	
	IC7CON2 = IC1CON2; // Same here ...
	IC7CON1 = IC1CON1;
	
	
	
	// OC7 is used to generate the front pulse
	OC7CON1 = 0;
	OC7CON2 = 0;
	OC7CON1bits.OCTSEL = 0x7; // CPU clock
	OC7R = 4; 
	OC7RS = OC7R + 960; // 60us

	OC8CON1 = OC7CON1;
	OC8CON2 = OC7CON2;
	OC8R = OC7RS;
	OC8RS = OC8R + 960;
	
	// Start the OC module ! 
	OC7CON1bits.OCM = 4;
	OC8CON1bits.OCM = 4;

}

void prox_poweroff(void) {

    
	OC7CON1 = 0;
	OC8CON1 = 0;
	IC1CON1bits.ICM = 0;
	IC2CON1bits.ICM = 0;
	IC3CON1bits.ICM = 0;
	IC4CON1bits.ICM = 0;
	IC5CON1bits.ICM = 0;
	IC6CON1bits.ICM = 0;
	IC7CON1bits.ICM = 0;
	
	va_put();
        
        // if calibration is new, and Vbat > 3.3V, then flash
        if(update_calib && vmVariables.vbat[0] > 655) {
            AsebaNative__system_settings_flash(NULL);
        }
}

