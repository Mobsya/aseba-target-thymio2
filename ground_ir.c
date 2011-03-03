#include <types/types.h>
#include <skel-usb.h>

#include "ground_ir.h"

#define PULSE_L _LATD6
#define PULSE_R _LATE4

static int state;

void ground_ir_new(unsigned int r, unsigned int l) {	
	switch(state) {
		case 0:
			vmVariables.ground_ambiant[0] = r;
			vmVariables.ground_ambiant[1] = l;
			PULSE_R = 1;
			break;
		case 1:
			vmVariables.ground_reflected[0] = r;
			PULSE_R = 0;
			PULSE_L = 1;
			break;
		case 2: 
			vmVariables.ground_reflected[1] = l;
			PULSE_L = 0;
			break;
	}	
	
	if(state++ == 800) 
		state = 0;	
}

void ground_ir_shutdown(void) {
	PULSE_L = 0;
	PULSE_R = 0;
}
