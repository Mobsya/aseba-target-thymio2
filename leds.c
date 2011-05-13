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
static unsigned char led[MAX_BRIGHTNESS * LED_BANK];
static unsigned char index;

// Todo: check the constrains & qualifiers
#define atomic_andb(x,y) do { __asm__ volatile ("and.b %[yy], [%[xx]], [%[xx]]": : [xx] "r" (x), [yy] "r"(y): "cc","memory"); } while(0)
/** Atomic or operation to prevent race conditions inside interrupts: *x = (*x) | y */
#define atomic_orb(x,y) do { __asm__ volatile ("ior.b %[yy], [%[xx]], [%[xx]]" : : [xx] "r" (x), [yy] "r"(y): "cc","memory"); } while(0)

void leds_clear_all(void) {
	// Enable to poweroff all leds, but keep the SOUND_ON as it is.	
	unsigned char _l[LED_BANK];
	int i;
	memcpy(_l,leds_off,LED_BANK);
	
	_l[1] |= led[SOUND_ON / 8] & (1 << (SOUND_ON % 8));
	
	for(i = 0; i < MAX_BRIGHTNESS; i++) 
		memcpy(led + i * LED_BANK, _l, LED_BANK);
}

void leds_set(unsigned char l, unsigned char brightness) {
	unsigned char bank;
	unsigned char pin;
	unsigned char i;
	unsigned char *p;
	unsigned char inverted;
	
	// ~400 cycles to set a led... 
	
	if(l >= LED_BANK * 8) 
		return;
		
	bank = l >> 3;
	pin = 1 << (l & 0x7);
	inverted = leds_off[bank] & pin;
	
	p = led + ((((l & 0x7) * LED_BANK) + bank));
	
	if(p >= led + sizeof(led))
		p -= sizeof(led);
	
	if(brightness >= MAX_BRIGHTNESS)
		brightness = MAX_BRIGHTNESS;
	
	if(inverted) {
		pin = ~pin;
		for(i = 0; i < brightness; i++) {
			atomic_andb(p, pin);
			p = (p + LED_BANK);
			if(p >= led + sizeof(led))
				p -= sizeof(led);
		}
		pin = ~pin;
		for(;i < MAX_BRIGHTNESS; i++) {
			atomic_orb(p, pin);
			p = (p + LED_BANK);
			if(p >= led + sizeof(led))
				p -= sizeof(led);
		}
	} else {
		for(i = 0; i < brightness; i++) {
			atomic_orb(p,pin);
			p = (p + LED_BANK);
			if(p >= led + sizeof(led))
				p -= sizeof(led);
		}
		pin = ~pin;
		for(; i < MAX_BRIGHTNESS; i++) {
			atomic_andb(p,pin);
			p = (p + LED_BANK);
			if(p >= led + sizeof(led))
				p -= sizeof(led);
		}
	}
}

void leds_tick_cb(void) {
	LED_CS = 1;
	SPI1BUF = led[index++];
	SPI1BUF = led[index++];
	SPI1BUF = led[index++];
	SPI1BUF = led[index++];
	SPI1BUF = led[index++];
	if(index == sizeof(led)) 
		index = 0;
	LED_CS = 0;
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
	SPI1CON1bits.CKE = 0;
	SPI1CON1bits.SSEN = 0;
	SPI1CON1bits.CKP = 0;
	SPI1CON1bits.MSTEN = 1;
	SPI1CON1bits.SPRE = 4; // SPI clock, 1Mhz
	SPI1CON1bits.PPRE = 0x2;
	
	
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
