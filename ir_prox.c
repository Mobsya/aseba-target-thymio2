#include <types/types.h>

#include "ir_prox.h"
#include "regulator.h"

#include <skel-usb.h>

// I don't want gcc to optimise too much, force noinline
static int __attribute((noinline)) ic_bufne(int ic) {
	IC1CON1BITS * ic_ptr =(IC1CON1BITS *) &IC1CON1bits + 4*ic; // Memory map: con1, con2, buf, tmr
	return ic_ptr->ICBNE;
}

static unsigned int __attribute((noinline)) ic_buf(int ic) {
	return *((&IC1BUF) + ic*4);
}

void ir_prox_mesure(void) {
	int temp[2];
	int i;	
	for(i = 0; i < 7; i++) {
		if(ic_bufne(i)) {
			temp[0] = ic_buf(i);
			if(ic_bufne(i)) {
				temp[1] = ic_buf(i);
				vmVariables.prox[i] = temp[1] - temp[0];
			} else 
				vmVariables.prox[i] = 0;
		} else {
			vmVariables.prox[i] = 0;
		}
		// Make sure we have an empty buffer ...
		while(ic_bufne(i))
			ic_buf(i);
	}
	
	SET_EVENT(EVENT_PROX);
	
	// Retrigger everything ...
	OC7CON1 = 0;
	OC8CON1 = 0;
	OC7CON1bits.OCTSEL = 0x7; // CPU clock
	
	OC7CON1bits.OCM = 4;
	
	OC8CON1 = OC7CON1;
}


void prox_init(void) {
	va_get();
	
	
	// IC configuration (example, to be used by all IC)
	
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
	// We just do the pin toggeling in soft ... 
	
	OC7CON1 = 0;
	OC7CON2 = 0;
	OC7CON1bits.OCTSEL = 0x7; // CPU clock
	OC7R = 4; // FIXME Delay to start the cpu clock
	OC7RS = OC7R + 960; // 60us FIXME recompute the delay .....

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
}

