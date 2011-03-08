#include "tone.h"
#include "sound.h"

// Tone generator optimized for the thymio 7812Hz sampling freq.
// Base frequency: 55Hz, sampled at 7812Hz, 142 samples
unsigned char wave[142];

static unsigned int ptr;

static unsigned int skip_100;
static unsigned int counter;
static unsigned int skip;


unsigned long __udiv3216(unsigned long, unsigned int);

void tone_init(void) {
	int i;
	
	for(i = 0; i < 71; i++) 
		wave[i] = (255*i)/71;
		
	for(i = 71; i < 142; i++) 
		wave[i] = 255 - (255*(i-71))/71;
	
}

void tone_setup(unsigned int dHz) {
	// 
	if(dHz < 550) 
		dHz = 550;
	if(dHz > 78120 / 2)
		dHz = 78120 / 2;
	
	// Compute the skip value 
	skip_100 = __udiv3216(__builtin_muluu(dHz, 1420) + 7812/2,7812);
	skip = skip_100 / 100;
	skip_100 -= skip*100;
	
	counter = 0;
	
	// Don't reset ptr as we will have a smoother transistion ...
}

void tone_fill_buffer(unsigned char *b, unsigned int c) {
	unsigned int i;
	
	for(i = 0; i < c; i++) {
		*b++ = wave[ptr];
		ptr += skip;
	/*	if(++counter == 100) {
			counter = 0;
			ptr += skip_100;
		}*/
		if(ptr > 141)
			ptr -= 142;
	}
}

