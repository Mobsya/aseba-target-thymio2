#include <types/types.h>
#include <clock/clock.h>
#include <error/error.h>

#include "ntc.h"
#include "regulator.h"

static ntc_cb cb;

static unsigned int calib;

#define calib_off() do {_LATF3 = 0; }while(0)
#define ntc_off() do{_LATB13 = 0;  _LATF3 = 0;} while(0)
#define discharge_mode() do{_TRISG6 = 0; } while(0)
#define mesure_mode() do{_TRISG6 = 1; } while(0)

#define comparator_output_enable() do {_TRISG7 = 0; CM1CONbits.COE = 1;}while(0)
#define comparator_output_disable() do{_TRISG7 = 1; CM1CONbits.COE = 0;}while(0)


#define R_CALIB 10000


unsigned long __udiv3216(unsigned long, unsigned int);

void _ISR _IC8Interrupt(void) {
	unsigned int temp;
	_IC8IF = 0;
	temp = IC8BUF;
	
	// All the mesurments are one-shots. 
	// Disable the module
	IC8CON1bits.ICM = 0x0;
	comparator_output_disable();
	
	if(cb) {
		temp = __udiv3216(__builtin_muluu(temp,R_CALIB),calib);
		
		// Temp = 250 - (Rm - 5000)/9
		
		temp = (250*9 - ((int) temp) + 5000) / 9;
		
		cb(temp);
		
		ntc_off();
	} else {
		// Calibration phase ...
		calib = temp;		
		calib_off();
	}
	
	discharge_mode();
}

	
static void calibrate(void) {
	mesure_mode();
	
	calib = 0;
	
	comparator_output_enable();
	
	asm volatile("disi #2\n"
				"bset IC8CON1, #0\n" 
				"bset LATF, #3\n");
	
	while(!calib) barrier();
}

void ntc_init(ntc_cb mes_done, int prio) {
	va_get(); // We are going to put 3.3 on IR receiver ... need to power it up.
	
	// Discharge circuit
	_LATG6 = 0;
	_TRISG6 = 0;
	_LATF3 = 0;
	_TRISF3 = 0;
	_LATB13 = 0;
	_TRISB13 = 1;
	clock_delay_us(1000);
	
	// Init voltage reference @ 2.06V (around 62.5% Vcc)
	_CVRR = 0;
	_CVR = 15;
	_CVRSS = 0; // Vdd/Vss range
	_CVREN = 1;
	
	// Init comparator
	CM1CONbits.CREF = 1; // CVref input
	CM1CONbits.CCH = 0x2; // C1IND input
	
	CM1CONbits.CEN = 1;
	
	clock_delay_us(10); // Comparator stabilisation time ...
	
	// Init the IC8.
	IC8CON2 = 0;
	IC8CON1 = 0;
	IC8CON1bits.ICTSEL = 7; // Fcy
	_IC8IF = 0;
	_IC8IE = 1;
	_IC8IP = prio;
	
	cb = 0;
	calibrate();
	
	_TRISB13 = 0;
	
	cb = mes_done;
}

void ntc_shutdown(void) {
	// Force everything to 0
	_LATG6  = 0;
	_LATF3 = 0;
	_LATB13 = 0;
	_TRISG6 = 0;
	_TRISF3 = 0;
	_TRISB13 = 0;
	
	// Shutdown Comparator 
	CM1CONbits.CEN = 0;
	
	// Shutdown voltage reference
	_CVREN = 0;
	
	// Shutdown IC
	IC8CON1 = 0;
	_IC8IE = 0;
	
	va_put();
}


void ntc_mesure(void) {
	mesure_mode();
	
	comparator_output_enable();
	
	asm volatile("disi #3\n"
			"bset IC8CON1, #0\n"
			"bset LATB, #13\n"
			"bset LATF, #3\n");
	
}
