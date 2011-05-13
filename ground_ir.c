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

static int state;

void ground_ir_new(unsigned int r, unsigned int l) {	
	switch(state) {
	// +50 to be phase shifted from the horizontal prox & motor
		case 50:
			vmVariables.ground_ambiant[0] = r;
			vmVariables.ground_ambiant[1] = l;
			PULSE_R = 1;
			break;
		case 53:
			vmVariables.ground_reflected[0] = r;
			vmVariables.ground_delta[0] = r - vmVariables.ground_ambiant[0];
			PULSE_R = 0;
			PULSE_L = 1;
			break;
		case 56: 
			vmVariables.ground_reflected[1] = l;
			vmVariables.ground_delta[1] = l - vmVariables.ground_ambiant[1];
			PULSE_L = 0;
			break;
	}	
	// Period is very important, must match the motor one and horizontal prox
	if(state++ == 799) 
		state = 0;	
}

void ground_ir_shutdown(void) {
	PULSE_L = 0;
	PULSE_R = 0;
}
