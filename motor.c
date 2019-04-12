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

#include "motor.h"
#include "pwm_motor.h"
#include "pid_motor.h"
#include "ir_prox.h"

#include <types/types.h>
#include <skel-usb.h>

#define STATE_IDLE 0
#define STATE_VBAT 1
#define STATE_VIND1 2
#define STATE_VIND2 3
#define STATE_PWM 4

#define DURATION_IDLE 2

static int state;
static int counter; // Internal counter to the state


// Working value for the _CURRENT_ mesurment
static int vbat[2];
static int vind1[2];
static int vind2[2];
static int vind[2];

// Filter value we mean the vind over 4
static int vind_left[4];
static int vind_right[4];
static int filter_p; // Same for left and right
static int vind_filtered[2];

static int motor_sens[2];

#define SENS_MAX 10

// We are called at 8kHz
// We should manage the Vind and Vbat acquisition @ 100Hz
//		=> We do it when time == 40, 120, 200, 280, 360, 440, 520, 600, 680, 760
//	in order to be phase shifted with the horizontal and vertical prox.
void motor_new_analog(unsigned int l, unsigned int r, unsigned int time) {
	switch (state) {
		case STATE_IDLE:
			if (++counter == DURATION_IDLE) {
				counter = 0;
				
				if(motor_sens[0] >= 0) {
					pwm_motor_lock_vind1_left();
				} else {
					pwm_motor_lock_vind2_left();
				}
				if(motor_sens[1] >= 0) {
					pwm_motor_lock_vind1_right();
				} else {
					pwm_motor_lock_vind2_right();
				}
				state = STATE_VIND1;
			}
			break;
		case STATE_VBAT:
			vbat[0] = l;
			vbat[1] = r;
			
			SET_EVENT(EVENT_MOTOR);
			
			{
				int i;
				for( i  = 0; i < 2; i++) {
					if(motor_sens[i] >= 0) {
						if(vind2[i] < vbat[i] - 2) {//do not easy take change of sense
							vind[i] = vind2[i] - vbat[i];
						} else {
							vind[i] = vind1[i];
						}
					} else {
						if(vind1[i] > 2) {
							vind[i] = vind1[i];
						} else {
							vind[i] = vind2[i] - vbat[i];
						}
					}
					
					vmVariables.vbat[i] = vbat[i];
					
					if(vind_filtered[i] >= 0 && motor_sens[i] > 0) {
						if(++motor_sens[i] >= SENS_MAX)
							motor_sens[i] = SENS_MAX;
					}	
					if(vind_filtered[i] < 0 && motor_sens[i] < 0) {
						if(--motor_sens[i] <= -SENS_MAX)
							motor_sens[i] = -SENS_MAX;
					}
					if(vind_filtered[i] >= 0 && motor_sens[i] <= 0) 
						motor_sens[i]++;	
					if(vind_filtered[i] < 0 && motor_sens[i] >= 0) 
						motor_sens[i]--;
						
					
				}

				// Now filter the vind ... 
				vind_filtered[0] -= vind_left[filter_p];
				vind_filtered[1] -= vind_right[filter_p];
				vind_filtered[0] += vind[0];
				vind_filtered[1] += vind[1];
				vind_left[filter_p] = vind[0];
				vind_right[filter_p++] = vind[1];
				if(filter_p >= 4)
					filter_p = 0;
			}
			
			
			pid_motor_tick(vind,vbat);
			
			pwm_motor_unlock_left();
			pwm_motor_unlock_right();
			
			state = STATE_PWM;
			
			break;
		case STATE_VIND1:
			state = STATE_VIND2;
			if(motor_sens[0] >= 0) {
				pwm_motor_lock_vind2_left();
				vind1[0] = l;
			} else {
				pwm_motor_lock_vind1_left();
				vind2[0] = l;
			}
			
			if(motor_sens[1] >= 0) {
				pwm_motor_lock_vind2_right();
				vind1[1] = r;
			} else {
				pwm_motor_lock_vind1_right();
				vind2[1] = r;
			}
			break;
			
		case STATE_VIND2:
			state = STATE_VBAT;
			if(motor_sens[0] >= 0) {
				vind2[0] = l;
			} else {
				vind1[0] = l;
			}
			
			if(motor_sens[1] >= 0) {
				vind2[1] = r;
			} else {
				vind1[1] = r;
			}	
			
			pwm_motor_lock_vbat_left();
			pwm_motor_lock_vbat_right();
	
			break;
			
		case STATE_PWM:	
			if(time == 40 || time ==  120 || time == 200 || time == 280
				|| time == 360 || time == 440 || time == 520 || time == 600
				|| time == 680 || time == 760) {
				state = STATE_IDLE;
				pwm_motor_lock_off_left();
				pwm_motor_lock_off_right();
			}
			break;
	}
}

void motor_get_vind(int * u) {
	u[0] =__builtin_divsd(__builtin_mulss(vind_filtered[0],64), settings.mot256[0]);
	u[1] =__builtin_divsd(__builtin_mulss(vind_filtered[1],64), settings.mot256[1]);
}

