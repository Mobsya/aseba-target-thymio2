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

#include "button.h"

#include <types/types.h>
#include <skel-usb.h>

#include <math.h>

char buttons_state[5]; // duplicate with vmVariables.buttons_state, but we don't want the user to mess with the buttons



// last 16 buttons values
static unsigned int old_b[5][16];
// Insertion index
static unsigned char insert[5]; 
// Maximum value of the 16 
static unsigned int max[5]; 
// Minimum value of the 16
static unsigned int min[5];
// sum of the 16
static unsigned long sum[5];
// Mean IIR filter. 
static unsigned  long sum_filtered[5];
// IIR filter for the "noise".
static unsigned long noise[5];

// Some statemachine variables
static unsigned char inhibit[5];

// The binary status of the button
#define DEBOUNCE 3
#define STATE_RELEASED (-1)
static char state[5];

static unsigned char init[5];

// Some offset, constants, etc ...
#define MIN_TRESHOLD 200
#define TRESH_OFFSET 20

static void iir_sum(unsigned int i) {
	// Fixed point 26.6
	unsigned long u  = sum[i]*64;
	
	// TODO CHECK that compiler optimize correctly
	sum_filtered[i] = sum_filtered[i] * 128 + u;
	
	
	sum_filtered[i] = __udiv3216(sum_filtered[i], 129);
}

static void iir_noise(unsigned int i) {
	// Fixed point 26.6
	unsigned long u = __builtin_muluu(max[i] - min[i], 512);
	
	noise[i] = noise[i] * 128 + u;
	
	noise[i] = __udiv3216(noise[i], 129);
}

// update the max, min & mean value
static void compute_stats(unsigned int b, unsigned int i) {

	int j;
	
	// adjust the sum and insert now button value
	sum[i] -= old_b[i][insert[i]];
	sum[i] += b;
	old_b[i][insert[i]] = b;
	
	insert[i] = (insert[i] + 1) & 0xF; // +1 modulo 16
	
		// recompute the max & min ... 
	max[i] = 0;
	min[i] = 0xFFFF;
	for(j = 0; j < 16; j++) {
		if(max[i] < old_b[i][j])
			max[i] = old_b[i][j];
		
		if(min[i] > old_b[i][j])
			min[i] = old_b[i][j];
	}
	
	iir_sum(i);
	iir_noise(i);
}

void button_process(unsigned int b, unsigned int i) {
	unsigned int tresh;
	
	/* first give raw value to the user... */
	vmVariables.buttons[i] = b;
	
	if(!init[i]) {
		init[i] = 1;
		int j; 
		for(j = 0; j < 16; j++)
			old_b[i][j] = b;
		noise[i] = 20*512;
		sum_filtered[i] = 64*16UL * b;
		sum[i] = 16UL * b;
	}
	
	vmVariables.buttons_mean[i] = sum_filtered[i] / (64*16);
	vmVariables.buttons_noise[i] = noise[i] / 256;
		
	tresh = __builtin_divsd(noise[i],256);
	if(tresh < MIN_TRESHOLD)
		tresh = MIN_TRESHOLD;
	
	tresh += TRESH_OFFSET;
	// Fixme, do the check only one every 16 samples ? 
	if(b < __builtin_divud(sum_filtered[i],64*16) - tresh) {
		state[i]++;
	} else {
		if(state[i] == DEBOUNCE)
			state[i] = STATE_RELEASED;
		else
			state[i] = 0;	
	}
	
	if(state[i] > DEBOUNCE) {
		state[i] = DEBOUNCE;
		buttons_state[i] = 1;
		vmVariables.buttons_state[i] = 1;
	} else {
		if(state[i] == STATE_RELEASED) {
			// We got released, inhibit any stat update for some time ...
			inhibit[i] = 16;
		}
		vmVariables.buttons_state[i] = 0;
		buttons_state[i]= 0;
		if(inhibit[i] > 0) 
			inhibit[i]--;
		else 
			if(!state[i])
				compute_stats(b,i);
	}
	
	// Poweroff button managment. non-configurable behavior
	if(i == 2) {
		static unsigned char poweroff_timer;
		if(buttons_state[2]) {
			if(poweroff_timer++ >= 110) {
				poweroff_timer = 200;
				// Trigger softirq for poweroff (I cannot do it here ...)
				_INT3IF = 1;
			}
		} else {
			poweroff_timer = 0;
		}
	}
}

