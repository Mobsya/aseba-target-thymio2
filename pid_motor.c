#include "pid_motor.h"
#include "pwm_motor.h"

#include <skel-usb.h>


#define KP 5
#define TI 3

#define PWM_DIFF_M 300
#define INTEG_MAX (PWM_MAX*2*TI)

static int integ[2];

#define COUNTER_M 40
static int counter[2];

static int prev[2];

void pid_motor_tick(int *u) {
	int i;
	int error;
	int output[2];
	for(i = 0; i < 2; i++) {
		error = -vmVariables.target[i] + u[i];
		integ[i] += error;
		if(integ[i] > INTEG_MAX)
			integ[i] = INTEG_MAX;
		if(integ[i] < -INTEG_MAX)
			integ[i] = -INTEG_MAX;
			
		if(vmVariables.target[i] == 0) {
			counter[i]++;
			if(counter[i] >= COUNTER_M) 
				counter[i] = COUNTER_M;
		} else {
			counter[i] = 0;
		}
		
		if(counter[i] == COUNTER_M) {
			output[i] = 0;
			integ[i] = 0;
			prev[i] = 0;
		} else {
			output[i] =  KP * error;
			output[i] += integ[i] / TI;
		}
		/*
		if(abs(vmVariables.pwm[i]-output[i]) > PWM_DIFF_M) {
			if(output[i] > vmVariables.pwm[i]) 
				vmVariables.pwm[i] += PWM_DIFF_M;
			else
				vmVariables.pwm[i] -= PWM_DIFF_M;
		}*/
		
		prev[i] += output[i];
		prev[i] /= 2;	
	}
	
	pwm_motor_left(prev[0]);
	pwm_motor_right(prev[1]);
}

void pid_motor_init(void) {
	integ[0] = 0;
	integ[1] = 0;
	counter[0] = COUNTER_M;
	counter[1] = COUNTER_M;	
}

