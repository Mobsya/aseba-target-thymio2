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

#include "pmp.h"

void setup_io(void) {
	// Setup PWM high side as opendrain, default high
	_ODE3 = 1;
	_LATE3 = 1;
	_TRISE3 = 0;
	
	_ODE5 = 1;
	_LATE5 = 1;
	_TRISE5 = 0;
	
	_ODE0 = 1;
	_LATE0 = 1;
	_TRISE0 = 0;
	
	_ODE1 = 1;
	_LATE1 = 1;
	_TRISE1 = 0;
	
	// Output pin
	_LATF0 = 1;	// USB-500
	_TRISF0 = 0;
	
	// By default, enable 500mA charging.
	// We will switch to 100mA right after making sure to disable the leds
	_LATF1 = 1; // USB-Charge
	_TRISF1 = 0;
	
	_LATF5 = 0;
	_TRISF5 = 0;	// M1PWML1
	
	_LATB15 = 0;
	_TRISB15 = 0;  // Sound out
	
	_LATC13 = 0;	// LED_CS
	_TRISC13 = 0;
	
	_LATD0 = 0;		// LED CLK, make sure a default level is chosen when spi not enabled
	_TRISD0 = 0;
	
	_LATD4 = 0;
	_TRISD4 = 0; // M2PWML1
	
	_LATD5 = 0;
	_TRISD5 = 0;	// M2PWML2
	
	_LATD6 = 0;
	_TRISD6 = 0;	// ir-pulse-groundL
	
	_LATD8 = 0;		// M1PWML2
	_TRISD8 = 0;
	
	_LATD11 = 0;
	_TRISD11 = 0;	// LED_SDO, make sure it's an output when spi not enabled
	
	_LATE2 = 0; // VA, disabled by default
	_TRISE2 = 0;
	
	_LATE4 = 0;		// ir-pulse-groundR
	_TRISE4 = 0;
	
	_LATF4 = 0;	// ir-pulse-back
	_TRISF4 = 0; 
	
	_LATD9 = 0;	// ir-pulse-front
	_TRISD9 = 0;
	
	
	// Put SD card DAT[3] (CS) at low, then input, will be used for card detection...
	_LATB14 = 0;
	_TRISB14 = 0;
	Nop();
	_TRISB14 = 1;
	_LATB14 = 1;
	
}

void setup_pps(void) {
	/* Output only modules */
	
	// PWM OC
	_RP17R = 19;
	_RP25R = 21;
	_RP20R = 22;
	_RP2R = 20;
	
	// Sound OC
	_RP29R = 23;
	
	// Led SPI
	_RP11R = 8;
	_RP12R = 7;
	
	_RP26R = 1; // Comparator output ... 
	
	// OC pulse ir sensors
	_RP10R = 25;
	_RP4R = 24;
	
	
	/* Input only modules */
	
	// IR Sensors
	_IC1R = 7;
	_IC2R = 27;
	_IC3R = 8;
	_IC4R = 19;
	_IC5R = 9;
	_IC6R = 3;
	_IC7R = 37;
	
	// NTC
	_IC8R = 26;
	
	// IR Remote control
	_IC9R = 26;
	
	/* Bidirectional modules */
	
	// SD card SPI
	// FIXME Handle correctly VA switchable LDO !!!! 
	// IO not enabled until SPI is enabled, so it's safe 
	// To do it now (TODO: Check this with a scope ... )
	_RP24R = 11;
	_RP23R = 10;
	_SDI2R = 22;
}

