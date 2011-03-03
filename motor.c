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
#define DURATION_PWM (80 - DURATION_IDLE - 3)

static int state;
static int counter; // Internal counter to the state


// Working value for the _CURRENT_ mesurment
static unsigned int vbat[2];
static unsigned int vind1[2];
static unsigned int vind2[2];
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
void motor_new_analog(unsigned int l, unsigned int r) {
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
						if(vind2[i] < vbat[i] - 1) {
							vind[i] = vind2[i] - vbat[i];
						} else {
							vind[i] = vind1[i];
						}
					} else {
						if(vind1[i] > 1) {
							vind[i] = vind1[i];
						} else {
							vind[i] = vind2[i] - vbat[i];
						}
					}
					
					vmVariables.vbat[i] = vbat[i];
					
					if(vind_filtered[i] > 0 && motor_sens[i] > 0) {
						if(++motor_sens[i] >= SENS_MAX)
							motor_sens[i] = SENS_MAX;
					}	
					if(vind_filtered[i] < 0 && motor_sens[i] < 0) {
						if(--motor_sens[i] <= -SENS_MAX)
							motor_sens[i] = -SENS_MAX;
					}
					if(vind_filtered[i]> 0 && motor_sens[i] <= 0) 
						motor_sens[i]++;	
					if(vind_filtered[i] < 0 && motor_sens[i] >= 0) 
						motor_sens[i]--;
				}
			
			
				// Now filter the vind ... 
				vind_left[filter_p] = vind[0];
				vind_right[filter_p++] = vind[1];
				if(filter_p >= 4)
					filter_p = 0;

// TODO: Optimize		
			/*	int mean = 0;
				for( i = 0; i < 4; i++) 
					mean += vind_left[i];*/
				vind_filtered[0] = vind[0]; //mean / 4;
				/*
				mean = 0;
				for(i = 0; i < 4; i++) 
					mean += vind_right[i];*/
				vind_filtered[1] = vind[1]; //mean / 4;
				
				vmVariables.uind[0] = vind_filtered[0];
				vmVariables.uind[1] = vind_filtered[1];
			}
					
			
			// TODO: Run the PID ! 
			
			pid_motor_tick(vind_filtered);
			
			pwm_motor_unlock_left();
			pwm_motor_unlock_right();
			
			// Trigger prox mesurment
			ir_prox_mesure();

			
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
			if(++counter == DURATION_PWM) {
				state = STATE_IDLE;
				counter = 0;
				pwm_motor_lock_off_left();
				pwm_motor_lock_off_right();
			}
			break;
	}
}

