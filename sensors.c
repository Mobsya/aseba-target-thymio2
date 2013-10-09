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

#include <types/types.h>
#include <skel-usb.h>

#include "analog.h"
#include "sensors.h"
#include "button.h"
#include "sound.h"
#include "motor.h"
#include "ground_ir.h"
#include "leds.h"
#include "ir_prox.h"

// Callback from the ADC @ 8kHz
// Capacitive touch sensors availble at 8/5Khz
static int button[5];
static int count;

static inline void manage_buttons_event(void) {
	static int old_b;
	// Periodic button event.
	SET_EVENT(EVENT_BUTTONS);
	 
	if((old_b & 0b1) != vmVariables.buttons_state[0]) {
		old_b ^= 0b1;
		SET_EVENT(EVENT_B_BACKWARD);
	}
	if((old_b & 0b10) != vmVariables.buttons_state[1] << 1) {
		old_b ^= 0b10;
		SET_EVENT(EVENT_B_LEFT);
	}
	if((old_b & 0b100) != vmVariables.buttons_state[2] << 2) {
		old_b ^= 0b100;
		SET_EVENT(EVENT_B_CENTER);
	}
	if((old_b & 0b1000) != vmVariables.buttons_state[3] << 3) {
		old_b ^= 0b1000;
		SET_EVENT(EVENT_B_FORWARD);
	}
	if((old_b & 0b10000) != vmVariables.buttons_state[4] << 4) {
		old_b ^= 0b10000;
		SET_EVENT(EVENT_B_RIGHT);
	}	
}	

#define PERIOD_100MS	799
static int per_acc;				// Period accumulator, allow the ir_prox to change their period. This accumulate the change and release it oneW tick per 100ms
								// Negative mean that we want to slow down (count higher than 799) positive we want to accelerate (stop counting at 798)

int sensors_prox_drift(void) {
	return per_acc;
}

void new_sensors_value(unsigned int * val, int b) {
	static unsigned int time;		// some sensors needs to have access to a timebase over 100ms (0-799), can be more than 799 if the ir want to stretch the period
	leds_tick_cb();
	
	/* Sound ... */
	sound_new_sample(val[3]);

	/* Motor and IR sensors need to be synchronized
	 * But the Horizontal IR need to be able to change its period slightly.
	 * We keep the period drift here. The offset are kept inside each sub-routine
	 *
	 * The motors needs to have a period of about 100Hz, they trigger every 80 cycle, phase shifted by 40. need 10 cycles
	 * The ground IR need a period of 10Hz, they trigger at time == 50, need 6 cycles
	 * The horizontal IR need a period of 10Hz, with variable phase, trigger at time = 0, need 40 cycle.
	 */

	motor_new_analog(val[5],val[4],time);

	per_acc += ir_prox_tick(time);
	if(per_acc > 100)
		per_acc = 100;
	if(per_acc < -100)
		per_acc = -100;

	ground_ir_new(val[1],val[2],time);

	if(per_acc < 0) {
		if(time == PERIOD_100MS) {
			per_acc++;
			time++;
		} else if(time > PERIOD_100MS) {
			time = 0;
		} else {
			time++;
		}
	} else if(per_acc > 0) {
		if(time == PERIOD_100MS - 1) {
			per_acc--;
			time = 0;
		} else if(time++ >= PERIOD_100MS) { // We have to re-check as per_acc might be set when time == PERIOD_100MS
			time = 0;
		}
	} else {
		if(time++ >= PERIOD_100MS)
			time = 0;
	}

	// Latch the leds ...
	asm volatile ("	bset LATC,#13");
	
/* Capacitive button first stage filtering ... */
	button[b] += val[0];
	
	// last value ... call the processing algos...
	if(count == 31) 
		button_process(button[b],b);
	
	if(b == 4) {
		count++;
		if(count == 32) {
			manage_buttons_event();
			count = 0;
			button[0] = 0;
			button[1] = 0;
			button[2] = 0; 
			button[3] = 0;
			button[4] = 0;	
		}
	}
}	
