#ifndef _PWM_MOTOR_H_
#define _PWM_MOTOR_H_

void pwm_motor_init(void);
void pwm_motor_poweroff(void);

// set the PWM duty
void pwm_motor_left(int duty);
void pwm_motor_right(int duty);

#define PWM_MAX 801

// The following function must be use with extreme care
// You can burn the transistor _really_ easily...


// Off the PWM, let the motor floating
void pwm_motor_lock_off_right(void);
void pwm_motor_lock_off_left(void);

// setup the PWM so we can read Vbat on the analog input
void pwm_motor_lock_vbat_left(void);
void pwm_motor_lock_vbat_right(void);

// Setup the PWM so we can read Vinduit on the analog input
void pwm_motor_lock_vind1_left(void);
void pwm_motor_lock_vind2_left(void);
void pwm_motor_lock_vind1_right(void);
void pwm_motor_lock_vind2_right(void);

// restart the standard PWM 
void pwm_motor_unlock_left(void);
void pwm_motor_unlock_right(void);



#endif
