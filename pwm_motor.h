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
