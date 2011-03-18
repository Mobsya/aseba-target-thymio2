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
