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

#include "tone.h"
#include "sound.h"

unsigned char wave[WAVEFORM_SIZE];

static unsigned int ptr;

static unsigned int skip_d;
static unsigned int counter;
static unsigned int skip;


unsigned long __udiv3216(unsigned long, unsigned int);

void tone_init(void) {
	int i;

	for(i = 0; i < WAVEFORM_SIZE/2; i++) 
		wave[i] = (255*i)/(WAVEFORM_SIZE/2);
		
	for(i = WAVEFORM_SIZE/2; i < WAVEFORM_SIZE; i++) 
		wave[i] = 255 - (255*(i-WAVEFORM_SIZE/2))/(WAVEFORM_SIZE/2);
}

void tone_setup(unsigned int dHz) {
	if(dHz < 550) 
		dHz = 550;
	if(dHz > 78120 / 2)
		dHz = 78120 / 2;
	
	// Compute the skip value 
	skip_d = __builtin_divud(__builtin_muluu(dHz, 1420) + 7812/2,7812);
	skip = skip_d / 100;
	skip_d -= skip*100;
	
	if(skip_d)
		skip_d =  100/skip_d;
	
	counter = 0;
	
	// Don't reset ptr as we will have a smoother transistion ...
}

void tone_fill_buffer(unsigned char *b, unsigned int c) {
	unsigned int i;
	
	for(i = 0; i < c; i++) {
		*b++ = wave[ptr];
		ptr += skip;
		if(skip_d && (++counter == skip_d)) {
			counter = 0;
			ptr++;
		}
	
		if(ptr > WAVEFORM_SIZE - 1)
			ptr -= WAVEFORM_SIZE;
	}
}


void tone_set_waveform(int * buf) {
	int i;
	for(i = 0; i < WAVEFORM_SIZE; i++)
		wave[i] = buf[i] + 128;
}	

