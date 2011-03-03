#include <p24fxxxx.h>

#include "pwm_motor.h"


#define M1_H1 _LATE3
#define M1_L1 _LATF5
#define M1_H2 _LATE5
#define M1_L2 _LATD8
#define M2_H1 _LATE0
#define M2_L1 _LATD4
#define M2_H2 _LATE1
#define M2_L2 _LATD5

#define PWM_PERIOD (16000000 / 20000)

static int duty_left;
static int duty_right;
static int locked_left;
static int locked_right;


static __attribute((noinline)) void delay_2us(void) {
	// 2 cycles to call, 2 cycles to return wait 28 cycles
	Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(), Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(), Nop(); Nop(); Nop(); 
}

void pwm_motor_init(void) {
	// Init the PWM to 20kHz
	OC2CON1bits.OCTSEL = 0x7;
	OC2CON2bits.SYNCSEL = 0x1f;
	OC2R = 0;
	OC2RS = PWM_PERIOD;
	OC2CON1bits.OCM = 6;
	
	OC3CON1bits.OCTSEL = 0x7;
	OC3CON2bits.SYNCSEL = 0x1f;
	OC3R = 0;
	OC3RS = PWM_PERIOD;
	OC3CON1bits.OCM = 6;
	
	OC4CON1bits.OCTSEL = 0x7;
	OC4CON2bits.SYNCSEL = 0x1f;
	OC4R = 0;
	OC4RS = PWM_PERIOD;
	OC4CON1bits.OCM = 6;
	
	OC5CON1bits.OCTSEL = 0x7;
	OC5CON2bits.SYNCSEL = 0x1f;
	OC5R = 0;
	OC5RS = PWM_PERIOD;
	OC5CON1bits.OCM = 6;
	
	duty_left = 0;
	duty_right = 0;
	
	locked_left = 0;
	locked_right = 0;
}


void pwm_motor_poweroff(void) {
	locked_left = 1;
	locked_right = 1;
	// Shutdown high-side
	M1_H1 = 1;
	M1_H2 = 1;
	M2_H1 = 1;
	M2_H2 = 1;
	// Make sure low side is off too
	M1_L1 = 0;
	M1_L2 = 0;
	M2_L1 = 0;
	M2_L2 = 0;
	
	// Shutdown OC
	OC2CON1 = 0;
	OC3CON1 = 0;
	OC4CON1 = 0;
	OC5CON1 = 0;
}

static void _fast_pwm_left(int duty) {
	if(duty > 0) {
		M1_H2 = 0;
		OC2R = duty;
	} else if(duty < 0) {
		M1_H1 = 0;
		OC3R = -duty;
	} else {
		M1_H1 = 1;
		M1_H2 = 1;
		OC2R = 0;
		OC3R = 0;
	}
}

static void _full_pwm_left(int duty) {
	M1_H1 = 1;
	M1_H2 = 1;
	OC2CON1bits.OCM = 0; // immediatly shutdown PWM
	OC3CON1bits.OCM = 0;
	
	delay_2us(); // Exactly 32 cycles
	
	if(duty > 0) {
		OC2R = duty;
		OC3R = 0;
		M1_H2 = 0;
	} else if (duty < 0) {
		OC3R = -duty;
		OC2R = 0;
		M1_H1 = 0;
	} else {
		OC3R = 0;
		OC2R = 0;
	}
	OC2CON1bits.OCM = 6; // restart PWM
	OC3CON1bits.OCM = 6;
}

static void _full_pwm_right(int duty) {
	// Ok, switching off H-bridge ... 
	M2_H1 = 1;
	M2_H2 = 1;
	OC4CON1bits.OCM = 0; // immediatly shutdown PWM
	OC5CON1bits.OCM = 0;
	
	delay_2us(); // Exactly 32 cycles
	
	if(duty > 0) {
		OC4R = duty;
		OC5R = 0;
		M2_H2 = 0;
	} else if(duty < 0){
		OC5R = -duty;
		OC4R = 0;
		M2_H1 = 0;
	} else {
		OC4R = 0;
		OC5R = 0;
	}
	OC4CON1bits.OCM = 6; // restart PWM
	OC5CON1bits.OCM = 6;
}

static void _fast_pwm_right(int duty) {
	if(duty > 0) {
		M2_H2 = 0;
		OC4R = duty;
	} else if(duty < 0) {
		M2_H1 = 0;
		OC5R = -duty;
	} else {
		M2_H1 = 1;
		M2_H2 = 1;
		OC4R = 0;
		OC5R = 0;
	}
}

void pwm_motor_left(int duty) {

	if(!locked_left) {
		if(!((duty ^ duty_left) & 0x8000) || duty_left == 0 || duty == 0) {
			// Same sign, or switching on/off the pwm
			// we can immediatly set the pwm
			_fast_pwm_left(duty);
		} else {
			_full_pwm_left(duty);
		}
	}
	
	duty_left = duty;
}

void pwm_motor_right(int duty) {

	if(!locked_right) {
		if(!((duty ^ duty_right) & 0x8000) || duty_right == 0 || duty == 0) {
			// Same sign, or switching on/off the pwm
			// we can immediatly set the pwm
			_fast_pwm_right(duty);
		} else {
			_full_pwm_right(duty);
		}
	}
	
	duty_right = duty;
}

void pwm_motor_lock_off_left(void) {
	locked_left = 1;
	OC2CON1bits.OCM = 0; // Switch off PWM
	OC3CON1bits.OCM = 0; // ''
	M1_H2 = 1;
	M1_H1 = 1;
}

void pwm_motor_lock_vbat_left(void) {
	locked_left = 1;
	M1_H2 = 1;
	M1_L2 = 0;
	M1_H1 = 0; // Turn on high side switch
}

void pwm_motor_lock_vind1_left(void) {
	locked_left = 1;
	M1_H2 = 1; // Make sure high-side switch if off
	delay_2us();
	M1_L2 = 1; // switch on low-side 2 switch
}

void pwm_motor_lock_vind2_left(void) {
	locked_left = 1;
	M1_L2 = 0;
	M1_H2 = 0; // switch on high-side 2 switch
}

void pwm_motor_unlock_left(void) {
	if(!locked_left)
		return;
	locked_left = 0;
	// No need to switch off high-side, will be done in _full_pwm_*
	_full_pwm_left(duty_left);
}

void pwm_motor_unlock_right(void) {
	if(!locked_right)
		return;
	locked_right = 0;
	_full_pwm_right(duty_right);
}


void pwm_motor_lock_off_right(void) {
	locked_right = 1;
	OC4CON1bits.OCM = 0; // Switch off PWM
	OC5CON1bits.OCM = 0;
	M2_H2 = 1;
	M2_H1 = 1; 
}

void pwm_motor_lock_vbat_right(void) {
	locked_right = 1;
	M2_H2 = 1;
	M2_L2 = 0;
	M2_H1 = 0; // Turn on high side switch
}


void pwm_motor_lock_vind1_right(void) {
	locked_right = 1;
	M2_H2 = 1; // Make sure high-side switch if off
	delay_2us();
	M2_L2 = 1; // switch on low-side 2 switch
}

void pwm_motor_lock_vind2_right(void) {
	locked_right = 1;
	M2_L2 = 0;
	M2_H2 = 0; // switch on high-side 2 switch
}
