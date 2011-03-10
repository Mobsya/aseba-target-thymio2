#include <p24fxxxx.h>
#include <clock/clock.h>
#include <timer/timer.h>

#include "regulator.h"
#include "leds.h"

#define LED_CS _LATC13

#define LED_BANK 5
#define MAX_BRIGHTNESS 32

static unsigned char leds_off[LED_BANK] = {0x3,0x80,0xf,0xf,0xf};
static unsigned char led[MAX_BRIGHTNESS * LED_BANK];
static unsigned char index;

// Todo: check the constrains & qualifiers
#define atomic_andb(x,y) do { __asm__ volatile ("and.b %[yy], [%[xx]], [%[xx]]": : [xx] "r" (x), [yy] "r"(y): "cc","memory"); } while(0)
/** Atomic or operation to prevent race conditions inside interrupts: *x = (*x) | y */
#define atomic_orb(x,y) do { __asm__ volatile ("ior.b %[yy], [%[xx]], [%[xx]]" : : [xx] "r" (x), [yy] "r"(y): "cc","memory"); } while(0)

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
	
	// FIXME: Disable timer here ? 
	
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
	clock_delay_us(1000); // FIXME: Check this delay to be the minimal delay ! 
	
	SPI1STAT = 0;
	SPI1CON1bits.DISSCK = 0;
	SPI1CON1bits.DISSDO = 0;
	SPI1CON1bits.MODE16 = 0;
	SPI1CON1bits.SMP = 0;
	SPI1CON1bits.CKE = 0;
	SPI1CON1bits.SSEN = 0;
	SPI1CON1bits.CKP = 1;
	SPI1CON1bits.MSTEN = 1;
	SPI1CON1bits.SPRE = 0x6; // SPI clock, 8Mhz
	SPI1CON1bits.PPRE = 0x3;
	
	SPI1CON2 = 1; // bufferized mode
	
	// enable SPI module
	SPI1STATbits.SPIEN = 1;
	
	// Write the init leds sequence:
	SPI1BUF = leds_off[0];
	SPI1BUF = leds_off[1];
	SPI1BUF = leds_off[2];
	SPI1BUF = leds_off[3];
	SPI1BUF = leds_off[4];
	
	clock_delay_us(5);
	
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
	clock_delay_us(5);
	SPI1BUF = leds_off[0];
	SPI1BUF = leds_off[1];
	SPI1BUF = leds_off[2];
	SPI1BUF = leds_off[3];
	SPI1BUF = leds_off[4];
	clock_delay_us(5);
	LED_CS = 1;
	clock_delay_us(1);
	LED_CS = 0; 
	SPI1STATbits.SPIEN = 0;
	
	va_put();
}
