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


#include <p24fxxxx.h>

#include <timer/timer.h>
#include <clock/clock.h>

#include "analog.h"
#include "regulator.h"
#include "ground_ir.h"
#include "log.h"

// Time for the adc to stabilize: 300ns
// pic clock 16Mhz, so we need 4.8 cycles.
#define adc_enable()  do { _ADON = 1; Nop(); Nop(); Nop(); Nop(); Nop(); } while(0)


// 1 adc clock: 250ns: 4Cycles. So wait 6 (5 is not enough for some pics ... )  
#define WAIT_ONE_ADC_CLOCK() do { Nop(); Nop(); Nop(); Nop(); Nop(); Nop();} while(0)

static int analog_state; // 0-4, depending on which button is scanned ... 
static int timer;

void new_sensors_value(unsigned int * val, int button); // callback

void __attribute((interrupt,no_auto_psv)) _ADC1Interrupt(void) {
	_ADON = 0;
	_AD1IF = 0;
}

static void timer_cb(int timer_id) {
	unsigned int sensors[6];
	int a_state = analog_state;
	
	AD1CON1bits.ASAM = 0;
	
	sensors[0] = ADC1BUF0;
	sensors[1] = ADC1BUF1;
	sensors[2] = ADC1BUF2;
	sensors[3] = ADC1BUF3;
	sensors[4] = ADC1BUF4;
	sensors[5] = ADC1BUF5;
		

	(void) ADC1BUF6;
	(void) ADC1BUF7;
	
	// Ground source
	CTMUCON = 0x8634; // Reset CTMU 
	
	// change sampling sequence
	// Put pin as input
	switch(analog_state) {
	case 0:
		_TRISB0 = 0;
		_TRISB1 = 1;
		analog_state = 1;
		break;
		
	case 1:
		_TRISB1 = 0;
		_TRISB2 = 1;
		analog_state = 2;
		break;
		
	case 2:
		_TRISB2 = 0;
		_TRISB3 = 1;
		analog_state = 3;
		break;
		
	case 3:
		_TRISB3 = 0;
		_TRISB4 = 1;
		analog_state = 4;
		break;
		
	case 4:
		_TRISB4 = 0;
		_TRISB0 = 1;
		analog_state = 0;
		break;
	}
	AD1CSSL = 0x7c60 | (1 << analog_state);
	
	// reset OC
	OC1CON1 = 0;
	OC1CON1bits.OCTSEL = 0x7; // CPU clock
	

	// switch on ADC
	
	// FIXME: Between here and the OC start, we should disable all interrupt to get a better mesurment
	// No interrupts have higher priority than us, so it's fine ...
	adc_enable();
	AD1CON1bits.ASAM = 1;
	WAIT_ONE_ADC_CLOCK();

	// start OC (current pulse)
	_EDGEN = 1;
	WAIT_ONE_ADC_CLOCK();
	OC1CON1bits.OCM = 4;
	
	// Unground it
	WAIT_ONE_ADC_CLOCK();
	_IDISSEN = 0;

	// Let's process the datas
	new_sensors_value(sensors, a_state);
}

void analog_disable(void) {
	timer_disable(timer);
	
	_ADON = 0;
	_AD1IE = 0;
	
	CTMUCONbits.CTMUEN = 0;

	OC1CON1 = 0;
	OC1CON2 = 0;
	

	_TRISB0 = 0;
	_TRISB1 = 0;
	_TRISB2 = 0;
	_TRISB3 = 0;
	_TRISB4 = 0;
	
	ground_ir_shutdown();
	
	va_put();
}	


void analog_init(int t, int prio) {	
// Init the adc statemachine to:
// convert micro, motor, IR and _ONE_ button at 8Khz (doing 1 or 16 conversion at xKhz cost the same ...)
// It use:
//  - ADC
//  - CTMU
//  - OC1 (CTMU timing)
	va_get();

	timer = t;

/*** ADC init */
	AD1CON1bits.ADSIDL = 0; 	// Continue in Idle mode
	AD1CON1bits.FORM = 0; 		// Integer mode
	AD1CON1bits.SSRC = 0b111; 	// auto convert
	AD1CON1bits.ASAM = 0; 		// automatic sample start (will be enabled after)

	AD1CON2bits.VCFG = 0;		// Avdd/Avss references
	AD1CON2bits.CSCNA = 1;		// scan the inputs ...
	AD1CON2bits.SMPI = 6; 		// interrupt every 6 samples (1 button, 2 IR, 1 micro, 2 motor)
	AD1CON2bits.BUFM = 1;		// Double buffer (but second buffer is _NEVER_ used, it give us some time to stop the AD)
	AD1CON2bits.ALTS = 0;		// MUX A
	
	AD1CON3bits.ADRC = 1;		// RC clock Trc = 250ns
	AD1CON3bits.SAMC = 31; 		// 
	
	// 1/(6* Tad *(31 + 12)) == 8.77kHz
		
	AD1PCFGL = 0xE380;
	AD1PCFGH = 0x3; 	// Don't use bandgap referances.
	
	// Add two others channels to slow down the ADC

	AD1CSSL = 0x7c60 | (1<<0); // Scan first button and everything else
	
	AD1CHS = 0;
	
	_AD1IE = 1;
	_AD1IP = prio;
	
/*** CTMU init */
	CTMUCON = 0x0E34;	// Edge1: OC1, Edge2: OC1 ! 
	CTMUICON = 0x0200 | (0x1F << 10); 	// 0.55uAx100, trim = 0
	CTMUCONbits.CTMUEN = 1;
	
/*** OC1 init */
	OC1CON1 = 0;
	OC1CON2 = 0;
	OC1CON1bits.OCTSEL = 0x7; // CPU clock
	OC1R = 1; // FIXME Delay to start the cpu clock
	OC1RS = OC1R + 80; // FIXME recompute the delay .....
	
	// To start the OC: OC1CON1bits.OCM = 4. 
	// We need to reset the module before output next pulse (async) ( by writing OC1CON2 to 0)


/*** Timer init */
	timer_init(timer, 2047, -1); // ~8Khz
	timer_enable_interrupt(timer, timer_cb, prio);

/*** PIN Init, configure as output */
	_LATB0 = 0;
	_LATB1 = 0;
	_LATB2 = 0;
	_LATB3 = 0;
	_LATB4 = 0;
	
	_TRISB0 = 0;
	_TRISB1 = 0;
	_TRISB2 = 0;
	_TRISB3 = 0;
	_TRISB4 = 0;

/*** State machine init */
	analog_state = 2;
	
	adc_enable();
	AD1CON1bits.ASAM = 1;
	Nop(); // Wait a bit to make sure that the AD internal capacitor is connected
	Nop();
	Nop();
	Nop();
	_TRISB0 = 1; // Switch to input
	
	_IDISSEN = 0;
	// Enable the current Pulse ! 
	OC1CON1bits.OCM = 4;
	
	// Start the whole thing ... 
	timer_enable(timer);
}


#define BUTTON_TRESHOLD 500

void __attribute__((noreturn)) analog_enter_poweroff_mode(void) {
	unsigned int button_max = BUTTON_TRESHOLD;
	unsigned int temp;
	int check_usb = 0;
	int pressed = 0;
	int i;
	int was_connected;
	
	was_connected = U1OTGSTATbits.SESVD;
	
	if(was_connected) {
		// Keep USB powered (to keep the pull-up/down).
		PMD4 = 0xFFFE;
	} else {
		PMD4 = 0xFFFF;
	}
	// Switch off everything else
	PMD1 = 0xFFFF;
	PMD2 = 0xFFFF;
	PMD3 = 0xFFFF;
	PMD5 = 0xFFFF;
	PMD6 = 0xFFFF;
	
	
	// We don't want _any_ interrupt (if somebody forgot to disable one ...)
	SET_IPL(7);
	
	_DOZE = 0x6; // CPU clock / 64 == 250kHz when _DOZEN = 1
/*	
	// Switch to FRCPLL clock (faster to start)
	// First switch to FRC without PLL
	_RCDIV = 0; // 8Mhz RC clock
	_CPDIV = 1; // 16Mhz from USB PLL
	
	__builtin_write_OSCCONH(0); // FRC
	__builtin_write_OSCCONL(1); // Switch
		
	while(_OSWEN); // wait until it's done ... 
	
	// Then switch to FRC + PLL
	
	__builtin_write_OSCCONH(1); // FRC+PLL
	__builtin_write_OSCCONL(1); // Switch 
	while(_OSWEN);
	*/
	
// Enable watchdog to wake us in 200ms (delay configured in configuration bits)
	while(1) {
		RCONbits.SWDTEN = 1;
		asm volatile("pwrsav #0"); // Poweroff
		// We got a wakeup from the watchdog ! 
		RCONbits.SWDTEN = 0;
		
		log_poweroff_tick();
		
		if(check_usb) {
			// Enable USB module
			_USB1MD = 0;
			U1PWRCbits.USBPWR = 1;
		}
		
		// Enable ADC & CTMU
		_ADC1MD = 0;
		_CTMUMD = 0;
		
		// Configure AD for 1 channel (button 3) 
		// No scanning.
		// CTMU "Full manual" mode
		
		CTMUICON = 0x0200 | (0x1F << 10);
		CTMUCON = 0x8090;
		
		AD1CON3bits.ADRC = 1;		// RC clock Trc = 250ns
		AD1PCFGL = 0xE380;
		AD1PCFGH = 0x3; 	// Don't use bandgap referances.	
		_ADON = 1;
		AD1CHS = 2; 
	
		temp = 0;
		for(i = 0; i < 32; i++) {
			_SAMP = 1; // Start sampling;
			_TRISB2 = 1; // Input ... 
			CTMUCONbits.EDG1STAT = 1;
			_DOZEN = 1;
			Nop();
			Nop();
			CTMUCONbits.EDG1STAT = 0;
			_SAMP = 0;
			_TRISB2 = 0;
			_DOZEN = 0;
			temp += ADC1BUF0;
		}
		
		if(temp < button_max - BUTTON_TRESHOLD) 
			pressed++;
		else
			pressed = 0;
			
		if(pressed == 3) {
			log_prepare_reset();
			asm volatile("reset");
		}
		
		if (temp < BUTTON_TRESHOLD)
			temp = BUTTON_TRESHOLD;
			
		if(button_max < temp)
			button_max = temp;
		
		_ADC1MD = 1;
		_CTMUMD = 1;
		
		if(check_usb) {
			// F*ck microchip ... 
			_DOZEN = 1;
			while(!(U1OTGSTATbits.SESEND || U1OTGSTATbits.SESVD)); 
			_DOZEN = 0;
			if(U1OTGSTATbits.SESVD) {
				// 5V present, wakeup if was not present before ! 
				if(!was_connected) {
					log_prepare_reset();
					asm volatile("reset");
				}
			} else {
				// if usb unplug, reinit the button treshold as it will trigger spurious button activity
				if(was_connected) 
					button_max = BUTTON_TRESHOLD;
				was_connected = 0;
			}
			// Switch off usb module
			// Only if 5V is not present
			if(!was_connected)
				_USB1MD = 1; 
		}
		check_usb = !check_usb;
	}
}
