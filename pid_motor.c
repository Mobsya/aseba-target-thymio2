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

#include "pid_motor.h"
#include "pwm_motor.h"
#include "log.h"

#include <skel-usb.h>


#define KP 5
#define TI 3

#define PWM_DIFF_M 300
#define INTEG_MAX (PWM_MAX*2*TI)

// it's about 400mA
#define IMOT_MAX 600
#define IMOT_HYST 300
// Must be power of two ... 
#define IMOT_FILT_CST 64
static int imot[2];
#define ILIM_LATENCY 3
static int ilim_enabled[2];

static int integ[2];

#define COUNTER_M 20
static int counter[2];

static int prev[2];

static int motor_target[2];

void pid_motor_tick(int *u, int * vbat) {
	int i;
	int error;
	int temp;
	for(i = 0; i < 2; i++) {
		/* First current estimation with previous pwm value. */

		// Actually, "PWM_MAX" in the following equation 
		// should be more around 850 to take in account
		// the time we spend to aquire vbat & vind
		temp = __builtin_divsd(__builtin_mulss(vbat[i], prev[i]), PWM_MAX);
		temp += u[i]; // uint is inverted
		imot[i] = (temp + __builtin_mulss(imot[i],(IMOT_FILT_CST - 1))) / IMOT_FILT_CST;
		vmVariables.imot[i] = imot[i];
		if(abs(imot[i]) > IMOT_MAX)
			ilim_enabled[i]++;
		
		if(abs(imot[i]) < IMOT_MAX - IMOT_HYST)
			ilim_enabled[i] = 0;
		
		
		/* Now, PID computation. */
		error = -motor_target[i] + u[i];
		integ[i] += error;
		if(integ[i] > INTEG_MAX)
			integ[i] = INTEG_MAX;
		if(integ[i] < -INTEG_MAX)
			integ[i] = -INTEG_MAX;
			
		if(motor_target[i] == 0) {
			counter[i]++;
			if(counter[i] >= COUNTER_M) 
				counter[i] = COUNTER_M;
		} else {
			counter[i] = 0;
		}
		
		if(counter[i] == COUNTER_M) {
			temp = 0;
			integ[i] = 0;
			prev[i] = 0;
		} else {
			temp =  KP * error;
			temp += integ[i] / TI;
		}
		vmVariables.integretor[i] = integ[i];
		// Lowpass filter on output
		prev[i] += temp;
		prev[i] /= 2;	
		
		if(prev[i] > PWM_MAX){
			integ[i] += (prev[i]-PWM_MAX)/2;//anti-reset windup
			prev[i] = PWM_MAX;
		}
			
		if(prev[i] < - PWM_MAX){
			integ[i] -= (-PWM_MAX-prev[i])/2;
			prev[i] = -PWM_MAX;	
		}
		
		if(ilim_enabled[i] >= ILIM_LATENCY) {
			log_set_flag(LOG_FLAG_MOTOROVERC);
			ilim_enabled[i] = ILIM_LATENCY;
			prev[i] /= 2;
		}
		
		vmVariables.pwm[i] = prev[i];
		
	}
	pwm_motor_left(prev[0]);
	pwm_motor_right(prev[1]);
}

#define SPEED_BOUND 1000
static int target_apply_calib(int t, int s) {
	if (t > SPEED_BOUND)
		t = SPEED_BOUND;
	if (t < -SPEED_BOUND)
		t = -SPEED_BOUND;

	// The rounding is wrong if you have a negative value ...
	// But it's OK as an error of 1 is completely negligible.
	return __builtin_mulss(t,s) >> 8;
}

void pid_motor_set_target(int * t) {
	motor_target[0] = target_apply_calib(t[0], settings.mot256[0]);
	motor_target[1] = target_apply_calib(t[1], settings.mot256[1]);
}

void pid_motor_init(void) {
	integ[0] = 0;
	integ[1] = 0;
	counter[0] = COUNTER_M;
	counter[1] = COUNTER_M;
	motor_target[0] = 0;
	motor_target[1] = 0;
}

