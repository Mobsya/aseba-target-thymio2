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

#include <p24Fxxxx.h>
#include <clock/clock.h>
#include <timer/timer.h>

#include <string.h>

#include "regulator.h"
#include "leds.h"

#define LED_CS _LATC13

#define LED_BANK 5
#define MAX_BRIGHTNESS 32

#define LEDS_WAIT 40

static unsigned char leds_off[LED_BANK] = {0x3,0x80,0xf,0xf,0xf};
unsigned char led[MAX_BRIGHTNESS * LED_BANK]; // Not static because used in assembly file
unsigned char * __attribute((near)) led_index = led; // Not static because used in assembly file

// count must be between 0 and 32
void _leds_set(unsigned char * p, unsigned int count, unsigned char mask);
void _leds_clr(unsigned char * p, unsigned int count, unsigned char mask);

void leds_clear_all(void) {
	// Enable to poweroff all leds, but keep the SOUND_ON as it is.	
	unsigned char _l[LED_BANK];
	int i;
	memcpy(_l,leds_off,LED_BANK);
	
	_l[1] |= led[SOUND_ON / 8] & (1 << (SOUND_ON % 8));
	
	for(i = 0; i < MAX_BRIGHTNESS; i++) 
		memcpy(led + i * LED_BANK, _l, LED_BANK);
}

static void _set(unsigned char bank, unsigned char offset, unsigned char pin, unsigned char count) {	
	unsigned char c = count;
	
	if(offset + c > MAX_BRIGHTNESS)
		c = MAX_BRIGHTNESS - offset; 
	
	_leds_set(&led[bank + offset * LED_BANK], c, 1 << pin);
	
	if(c != count) {
		c = count - c;
		_leds_set(&led[bank], c, 1 << pin);
	}
}

static void _clr(unsigned char bank, unsigned char offset, unsigned char pin, unsigned char count) {
	unsigned char c = count;
	
	if(offset + c > MAX_BRIGHTNESS)
		c = MAX_BRIGHTNESS - offset; 
	
	_leds_clr(&led[bank + offset * LED_BANK], c, ~(1 << pin));
	
	if(c != count) {
		c = count - c;
		_leds_clr(&led[bank], c, ~(1 << pin));
	}
}

static void _set_led(unsigned char bank, unsigned char pin, unsigned char inverted, unsigned char brightness) {
	unsigned char start = pin;
	
	if(inverted) {
		_clr(bank, start, pin, brightness);
		start += brightness;
		if(start >= MAX_BRIGHTNESS)
			start -= MAX_BRIGHTNESS;
		_set(bank, start, pin, MAX_BRIGHTNESS - brightness);
	} else {
		_set(bank, start, pin, brightness);
		start += brightness;
		if(start >= MAX_BRIGHTNESS)
			start -= MAX_BRIGHTNESS;
		_clr(bank, start, pin, MAX_BRIGHTNESS - brightness);
	}
}	


// brightness == 0|32: about 150cycles
// brightness != 0|32: about 260cycles
void leds_set(unsigned char l, unsigned char brightness) {
	unsigned char bank;
	unsigned char pin;
	unsigned char inverted;
	
	if(l >= LED_BANK * 8)
		return;
		
	bank = l >> 3;
	pin = l & 0x7;
	inverted = leds_off[bank] & (1 << pin);
		
	if(brightness >= MAX_BRIGHTNESS)
		brightness = MAX_BRIGHTNESS;
		
	// handle special cases (b == 0 or 32)
	if(!brightness) {
		if(inverted)
			_leds_set(&led[bank], MAX_BRIGHTNESS, 1 << pin);
		else
			_leds_clr(&led[bank], MAX_BRIGHTNESS, ~(1 << pin));
	} else if(brightness == MAX_BRIGHTNESS) {
		if(inverted)
			_leds_clr(&led[bank], MAX_BRIGHTNESS, ~(1 << pin));
		else
			_leds_set(&led[bank], MAX_BRIGHTNESS, 1 << pin);
	} else {
		_set_led(bank, pin, inverted, brightness);
	}
}	

void leds_init(void) {
	int i;
	LED_CS = 0;
	
	va_get();
	clock_delay_us(1000); // Wait until VA raise
	
	SPI1STAT = 0;
	SPI1CON1bits.DISSCK = 0;
	SPI1CON1bits.DISSDO = 0;
	SPI1CON1bits.MODE16 = 0;
	SPI1CON1bits.SMP = 0;
	SPI1CON1bits.CKE = 1; //change data on falling edge
	SPI1CON1bits.SSEN = 0;
	SPI1CON1bits.CKP = 0; //active state high 
	SPI1CON1bits.MSTEN = 1;
	SPI1CON1bits.SPRE = 0b110; // Secondary 2:1. Spi clock 8Mhz
	SPI1CON1bits.PPRE = 0x3; // Primary 1:1
	
	
	SPI1CON2 = 1; // bufferized mode
	
	// enable SPI module
	SPI1STATbits.SPIEN = 1;
	
	// Write the init leds sequence:
	SPI1BUF = leds_off[0];
	SPI1BUF = leds_off[1];
	SPI1BUF = leds_off[2];
	SPI1BUF = leds_off[3];
	SPI1BUF = leds_off[4];
	
	clock_delay_us(LEDS_WAIT);
	
	LED_CS = 1;
	
	// Fill the array with default off value
	for(i = 0; i < MAX_BRIGHTNESS; i++) {
		led[i*5] 	 = leds_off[0];
		led[i*5 + 1] = leds_off[1];
		led[i*5 + 2] = leds_off[2];
		led[i*5 + 3] = leds_off[3];
		led[i*5 + 4] = leds_off[4];
	}
}

void leds_poweroff(void) {
	LED_CS = 0;
	clock_delay_us(LEDS_WAIT);
	SPI1BUF = leds_off[0];
	SPI1BUF = leds_off[1];
	SPI1BUF = leds_off[2];
	SPI1BUF = leds_off[3];
	SPI1BUF = leds_off[4];
	clock_delay_us(LEDS_WAIT);
	LED_CS = 1;
	clock_delay_us(1);
	LED_CS = 0; 
	SPI1STATbits.SPIEN = 0;
	
	va_put();
}


void leds_set_circle(unsigned char l1, unsigned char l2, unsigned char l3, unsigned char l4, unsigned char l5, unsigned char l6, unsigned char l7, unsigned char l8) {
	leds_set(LED_CIRCLE_0, l1);
	leds_set(LED_CIRCLE_1, l2);
	leds_set(LED_CIRCLE_2, l3);
	leds_set(LED_CIRCLE_3, l4);
	leds_set(LED_CIRCLE_4, l5);
	leds_set(LED_CIRCLE_5, l6);
	leds_set(LED_CIRCLE_6, l7);
	leds_set(LED_CIRCLE_7, l8);
}

void leds_set_top(unsigned char r, unsigned char g, unsigned char b) {
	leds_set(LED_R_TOP, r);
	leds_set(LED_G_TOP, g);
	leds_set(LED_B_TOP, b);	
}

void leds_set_br(unsigned char r, unsigned char g, unsigned char b) {
	leds_set(LED_R_BOT_R, r);
	leds_set(LED_G_BOT_R, g);
	leds_set(LED_B_BOT_R, b);
}

void leds_set_bl(unsigned char r, unsigned char g, unsigned char b) {
	leds_set(LED_R_BOT_L, r);
	leds_set(LED_G_BOT_L, g);
	leds_set(LED_B_BOT_L, b);
}

void leds_set_prox_h(unsigned char l1, unsigned char l2, unsigned char l3, unsigned char l4, unsigned char l5, unsigned char l6, unsigned char l7, unsigned char l8)
{
	leds_set(LED_FRONT_IR_0, l1);
	leds_set(LED_FRONT_IR_1, l2);
	leds_set(LED_FRONT_IR_2A,l3);
	leds_set(LED_FRONT_IR_2B,l4);
	leds_set(LED_FRONT_IR_3, l5);
	leds_set(LED_FRONT_IR_4, l6);
	leds_set(LED_IR_BACK_L,  l7);
	leds_set(LED_IR_BACK_R,  l8);
}


void leds_set_body_rgb(unsigned int r, unsigned int g, unsigned int b) {
	leds_set_top(r,g,b);
	leds_set_br(r,g,b);
	leds_set_bl(r,g,b);
}
