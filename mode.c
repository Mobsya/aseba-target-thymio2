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

#include "mode.h"
#include "behavior.h"
#include "leds.h"
#include "button.h"
#include "test.h"
#include "usb_uart.h"
#include "playback.h"
#include "rf.h"
#include "ir_prox.h"
#include "sd.h"
#include <skel-usb.h>


// Macro to do a when-like as in aseba
// We put all the state into a section so we can
// Zero it on reinit.

#define when(cond) if(({static unsigned char __attribute((section("when"))) p; \
						unsigned char c = !!(cond); \
						unsigned char r = c && !p; \
						p = c; \
						r;}))



static enum mode current_mode;
static unsigned int vm_active;

	
static int rc5_speed_l;
static int rc5_speed_t;

static unsigned int bs_black_level = 400;
static unsigned int bs_white_level = 450;


int body_color_pulse_get(void) {
	static char led_pulse;
	int ret;
	led_pulse = led_pulse + 1;
	if(led_pulse > 0) {
		ret = led_pulse;
		if(led_pulse > 40)
			led_pulse = -128;
	} else
		ret = -led_pulse/4;
	return ret;
}

static char _rainbow_get(unsigned char i) {
	if(i < 32)
		return i;
	if( i < 64)
		return 64-i;
	return 0;
}

static void rainbow_get(unsigned char *rgb) {
	static unsigned char led_i;
	led_i = led_i + 1;
	if(led_i > 96)
		led_i = 0;

	rgb[0] = led_i;
	rgb[1] = led_i + 32;
	if(rgb[1] > 96)
		rgb[1] -=96;

	rgb[2] = led_i + 64;
	if(rgb[2] > 96)
		rgb[2] -=96;

	rgb[0] = _rainbow_get(rgb[0]);
	rgb[1] = _rainbow_get(rgb[1]);
	rgb[2] = _rainbow_get(rgb[2]);
}

static void set_mode_color(enum mode m) {
	switch (m) {
		case MODE_MENU:
			leds_set_top(0,0,0);
			break;
		case MODE_FOLLOW:
			leds_set_top(0,32,0);
			break;
		case MODE_EXPLORER:
			leds_set_top(32,32,0);
			break;
		case MODE_ACC:
			leds_set_top(32,0,0);
			break;
		case MODE_DRAW:
			leds_set_top(32,32,32);	
			break;
		case MODE_SOUND:
			leds_set_top(0,0,32);
			break;
		case MODE_LINE:
			leds_set_top(0,32,32);
			break;
		case MODE_RC5:
			leds_set_top(32,0,32);
			break;
		case MODE_SIDE:
			leds_set_top(32,15,5); // TODO enable a rainbow mode ?
			break;
	}
}

static void exit_mode(enum mode m) {
	leds_set_body_rgb(0,0,0);
	leds_set_circle(0,0,0,0,0,0,0,0);
	leds_set(LED_FRONT_IR_0, 0);
	leds_set(LED_FRONT_IR_1, 0);
	leds_set(LED_FRONT_IR_2A, 0);
	leds_set(LED_FRONT_IR_2B, 0);
	leds_set(LED_FRONT_IR_3, 0);
	leds_set(LED_FRONT_IR_4, 0);
	leds_set(LED_GROUND_IR_0, 0);
	leds_set(LED_GROUND_IR_1, 0);
	leds_set(LED_IR_BACK_L, 0);
	leds_set(LED_IR_BACK_R, 0);
	
	
	switch(m) {
		case MODE_MENU:
			behavior_stop(B_SETTING);
			break;
		case MODE_FOLLOW:
			prox_disable_network();
			behavior_stop(B_LEDS_PROX);
			vmVariables.target[0] = 0;
			vmVariables.target[1] = 0;
			break;
		case MODE_EXPLORER:
			behavior_stop(B_LEDS_PROX);
			vmVariables.target[0] = 0;
			vmVariables.target[1] = 0;
			break;
		case MODE_ACC:
			behavior_stop(B_LEDS_ACC);
			behavior_stop(B_LEDS_PROX);
			vmVariables.target[0] = 0;
			vmVariables.target[1] = 0;
			// TODO: Stop playing sound
			// we don't need as the button to switch the mode 
			// has been pressed, so generated a sound.
		//	play_sound(SOUND_DISABLE);
			break;
		case MODE_DRAW:
			behavior_stop(B_LEDS_PROX);
			break;
		case MODE_SOUND:
			behavior_stop(B_LEDS_PROX);
			behavior_stop(B_LEDS_MIC);
			leds_set(LED_SOUND, 0);
			vmVariables.sound_tresh = 0;
			CLEAR_EVENT(EVENT_MIC);
			vmVariables.target[0] = 0;
			vmVariables.target[1] = 0;
			break;
		case MODE_LINE:
			if (!sd_user_open("_BLWHLVL.DAT")) {
				unsigned int black = 0;
				unsigned int white = 0;

				sd_user_read((unsigned char *) &black, sizeof(bs_black_level));
				sd_user_read((unsigned char *) &white, sizeof(bs_white_level));

				if (black != bs_black_level || white != bs_white_level) {
					sd_user_seek(0);
					sd_user_write((unsigned char *) &bs_black_level, sizeof(bs_black_level));
					sd_user_write((unsigned char *) &bs_white_level, sizeof(bs_white_level));
				}
				sd_user_open(NULL);
			}

			vmVariables.target[0] = 0;
			vmVariables.target[1] = 0;
			behavior_stop(B_LEDS_PROX);
			break;
		case MODE_RC5:
			vmVariables.target[0] = 0;
			vmVariables.target[1] = 0;
			behavior_stop(B_LEDS_PROX);
			break;
		case MODE_SIDE:
			behavior_stop(B_LEDS_PROX);
			break;	
	}	
}

void init_vm_mode(void) {
	// do a menu mode -> vm mode (exit internal state machine)
	behavior_start(B_LEDS_ACC);
	behavior_start(B_LEDS_NTC);
	behavior_start(B_LEDS_MIC);
	behavior_start(B_LEDS_PROX);
	behavior_start(B_SOUND_BUTTON);
	behavior_start(B_LEDS_MIC);
}

static void init_mode(enum mode m) {
	set_mode_color(m);
	
	switch(m) {
		case MODE_MENU:
			behavior_start(B_SETTING);
			break;
		case MODE_FOLLOW:
			behavior_start(B_LEDS_PROX);
			prox_enable_network();
			break;
		case MODE_EXPLORER:
			behavior_start(B_LEDS_PROX);
			break;
		case MODE_ACC:
			behavior_start(B_LEDS_ACC);
			behavior_start(B_LEDS_PROX);
			leds_set_top(15,0,0);
			break;
		case MODE_DRAW:
			behavior_start(B_LEDS_PROX);
			break;
		case MODE_SOUND:
			behavior_start(B_LEDS_PROX);
			behavior_start(B_LEDS_MIC);
			vmVariables.sound_tresh = 246;
			break;
		case MODE_LINE:
			if (!sd_user_open("_BLWHLVL.DAT")) {
				sd_user_read((unsigned char *) &bs_black_level, sizeof(bs_black_level));
				sd_user_read((unsigned char *) &bs_white_level, sizeof(bs_white_level));
				sd_user_open(NULL);
			}
			behavior_start(B_LEDS_PROX);
			break;
		case MODE_RC5:
			rc5_speed_l = 0;
			rc5_speed_t = 0;
			behavior_start(B_LEDS_PROX);
			break;
		case MODE_SIDE:
			behavior_start(B_LEDS_PROX);
			break;	
	}		
}

static void tick_follow(void) {
	
	static char sound_done;
	static char does_see_friend;
	static unsigned char led_state;
	static char led_delta = 1;
	static int speed = 300;


#define DETECT 500 

	int i;
	int speed_diff;
	int speed_l = 0;
	int max, mi, t;
	
	max = vmVariables.prox[0];
	mi = 0;
	for(i = 1; i < 5; i++) {
		if(vmVariables.prox[i]> max) {
			max = vmVariables.prox[i];
			mi = i;
		}
	}
	
	t = 2 - mi;
	speed_diff = t * (speed / 2);
	if (max > 3500) 
		speed_l = (3500 - max) / 2;
	
	if (max > 4000)
		speed_l = -speed;
	
	if (max < 3000) {
		t = 300 - (max - 1000) / 7;
		speed_l = t;
	}
	
	if (max < 2000) 
		speed_l = speed;
	
	if(speed_l > speed)
		speed_l = speed;
	if(speed_l < -speed)
		speed_l = -speed;
	
	if(max < DETECT) {
		if(does_see_friend) {
			vmVariables.target[0] = speed;
			vmVariables.target[1] = speed;
		} else {
			vmVariables.target[0] = 0;
			vmVariables.target[1] = 0;
		}
	} else {
		vmVariables.target[1] = speed_diff + speed_l;
		vmVariables.target[0] = speed_l - speed_diff;
	}

	if(does_see_friend > 0 && sound_done) {
		unsigned char rgb[3];

		rainbow_get(rgb);

		leds_set_top(rgb[0],rgb[1],rgb[2]);
		leds_set_bl(rgb[2],rgb[0],rgb[1]);
		leds_set_br(rgb[1],rgb[2],rgb[0]);
	} else
		leds_set_body_rgb(0,body_color_pulse_get(),0);

	if(does_see_friend) {
		led_state += led_delta;
		if(led_state >= 31)
			led_delta = -1;
		else if(led_state == 0)
			led_delta = 1;

		leds_set_circle(0, led_state >> 4, led_state >> 3, led_state, 32, led_state, led_state >> 3, led_state >> 4);	
	} else
		leds_set_circle(0,0,0,32,32,32,0,0);

	/* button managment */
	when(buttons_state[3])  {
		speed = speed + 50;
		if(speed > 500)
			speed = 500;
	}
	when(buttons_state[0]) {
		speed = speed - 50;
		if(speed < -300)
			speed = -300;
	}
	
	when(max > DETECT) 
		play_sound(SOUND_F_DETECT);
	
	if(speed_diff == 0 && speed_l == 0 && sound_done == 0 && max > DETECT) {
		sound_done = 1;
		play_sound(SOUND_F_OK);
	}
	if(speed_diff != 0 || max < DETECT)
		sound_done = 0;
	

	
	if(vmVariables.ground_delta[0] < 130 || vmVariables.ground_delta[1] < 130) {
		vmVariables.target[0] = 0;
		vmVariables.target[1] = 0;
		leds_set(LED_R_BOT_L, 32);
		leds_set(LED_R_BOT_R, 32);
	} else {
		leds_set(LED_R_BOT_L, 0);
		leds_set(LED_R_BOT_R, 0);
	}
	
	if(does_see_friend)
		does_see_friend--;

	if(IS_EVENT(EVENT_DATA)) {
		CLEAR_EVENT(EVENT_DATA);
		does_see_friend = 0;
		mi = 0;
		max = vmVariables.intensity[0];
		vmVariables.intensity[0] = 0;
		for(i = 1; i < 7; i++) {
			if(vmVariables.intensity[i] > max) {
				mi = i;
				max = vmVariables.intensity[i];
			}
			vmVariables.intensity[i] = 0;
		}
		if(max > 3000) {
			vmVariables.ir_tx_data = mi;
			if(vmVariables.rx_data > 0 && vmVariables.rx_data < 4) {
				when(mi == 2) {
					play_sound(SOUND_F_OK);
				}
				if(mi == 2)
					does_see_friend = 6;
			}
		}
	}

}
static void tick_explorer(void) {
	static unsigned char led_state;
	static  int speed = 150;
	
	unsigned char l[8] = {0,0,0,0,0,0,0,0};
	unsigned char fixed;
	
	int p = body_color_pulse_get();
	leds_set_top(p,p,0);

	/* circle led managment */
	led_state += 2;
	fixed = led_state / 32;
	l[fixed] = 32;
	l[(fixed - 1) & 0x7] = 32 - (led_state & 0x1F);
	l[(fixed + 1) & 0x7] = led_state & 0x1F;
	leds_set_circle(l[0],l[1],l[2],l[3],l[4],l[5],l[6],l[7]);
	
	/* Button managment */
	when(buttons_state[3])  {
		speed = speed + 50;
		if(speed > 500)
			speed = 500;
	}
	when(buttons_state[0]) {
		speed = speed - 50;
		if(speed < -300)
			speed = -300;
	}
	
	if(speed >= 0) {
		long temp1 = 0;
		long temp2 = 0;
		temp1 += vmVariables.prox[0];
		temp1 += vmVariables.prox[1] * 2;
		temp1 += vmVariables.prox[2] * 3;
		temp1 += vmVariables.prox[3] * 2;
		temp1 += vmVariables.prox[4];
		temp2 += vmVariables.prox[0] * -4;
		temp2 += vmVariables.prox[1] * -3;
		temp2 += vmVariables.prox[3] * 3;
		temp2 += vmVariables.prox[4] * 4;
		vmVariables.target[0] = speed - __builtin_divsd((temp1 + temp2) * speed, 2000);
		vmVariables.target[1] = speed - __builtin_divsd((temp1 - temp2) * speed, 2000);
		
		if(vmVariables.target[0] < -600) 
			vmVariables.target[0] = -600;
		if(vmVariables.target[1] < -600)
			vmVariables.target[1]= -600;
		if(vmVariables.target[0] > 600)
			vmVariables.target[0] = 600;
		if(vmVariables.target[1] > 600)
			vmVariables.target[1] = 600;
			
	} else {
		long temp = __builtin_mulss(vmVariables.prox[6], speed);
		vmVariables.target[0] = speed + __builtin_divsd(temp, -300);
		
		temp = __builtin_mulss(vmVariables.prox[5], speed);
		vmVariables.target[1] = speed  + __builtin_divsd(temp, -300);
	} 
	
	if(vmVariables.ground_delta[0] < 130 || vmVariables.ground_delta[1] < 130) {
		vmVariables.target[0] = 0;
		vmVariables.target[1] = 0;
		leds_set(LED_R_BOT_L, 32);
		leds_set(LED_R_BOT_R, 32);
	} else {
		leds_set(LED_R_BOT_L, 0);
		leds_set(LED_R_BOT_R, 0);
	}
}

static void tick_acc(void) {
#define ACC_OBSTACLE 1000
#define ACC_FREE_FALL 14
	static unsigned int acc = 32;
	static int counter;
	int play = 0;
	acc = acc + acc + acc + abs(vmVariables.acc[0]) + abs(vmVariables.acc[1]) + abs(vmVariables.acc[2]);
	acc >>= 2;
	if(acc < ACC_FREE_FALL) {
		play = 1;
	}
	when(acc > ACC_FREE_FALL) {	
		leds_set_top(15,0,0);
	}
	
	if(acc < ACC_FREE_FALL) {
		counter++;
		if(counter > 5) {
			if (counter == 10)
				counter = 0;
			
			leds_set_top(32,0,0);
		} else {
			leds_set_top(0,0,0);
		}
	} else {
		leds_set_body_rgb(body_color_pulse_get(),0,0);
	}	
	
	if(vmVariables.acc_tap) {
		vmVariables.acc_tap = 0;
		play_sound(SOUND_TAP);
	}
	
	// Moving part.
	if(vmVariables.prox[1] > ACC_OBSTACLE && vmVariables.prox[2] > ACC_OBSTACLE && vmVariables.prox[3] > ACC_OBSTACLE &&
			(vmVariables.prox[5] > ACC_OBSTACLE || vmVariables.prox[6] > ACC_OBSTACLE) && 
			(vmVariables.ground_delta[0] > 130 && vmVariables.ground_delta[1] > 130)) {
		vmVariables.target[0] = 0;
		vmVariables.target[1] = 0;
		play = 1;
	} else if(vmVariables.prox[0] > ACC_OBSTACLE || vmVariables.prox[1] > ACC_OBSTACLE
				|| vmVariables.prox[2] > ACC_OBSTACLE || vmVariables.prox[3] > ACC_OBSTACLE
				|| vmVariables.prox[4] > ACC_OBSTACLE) {
		int temp = vmVariables.prox[0]/5 + vmVariables.prox[1]/4 + vmVariables.prox[2]/4;
		temp += vmVariables.prox[3]/4 + vmVariables.prox[4]/5;
		
		int temp2 = vmVariables.prox[0]/6 + vmVariables.prox[1]/5;
		temp2 -= vmVariables.prox[3]/5 + vmVariables.prox[4]/6;
		
		vmVariables.target[0] = -(temp + temp2);
		vmVariables.target[1] = temp2 - temp;
	} else if(vmVariables.prox[5] > ACC_OBSTACLE || vmVariables.prox[6] > ACC_OBSTACLE) {
		vmVariables.target[0] = vmVariables.prox[5]/4;
		vmVariables.target[1] = vmVariables.prox[6]/4;
	} else {
		vmVariables.target[0] = 0;
		vmVariables.target[1] = 0;
	}
	
	if(vmVariables.ground_delta[0] < 130 || vmVariables.ground_delta[1] < 130) {
		vmVariables.target[0] = 0;
		vmVariables.target[1] = 0;
		leds_set_br(32,0,0);
		leds_set_bl(32,0,0);
	} else {
		leds_set_br(0,0,0);
		leds_set_bl(0,0,0);
	}
	
	
	if(vmVariables.target[0] < -600) 
		vmVariables.target[0] = -600;
	if(vmVariables.target[1] < -600)
		vmVariables.target[1]= -600;
	if(vmVariables.target[0] > 600)
		vmVariables.target[0] = 600;
	if(vmVariables.target[1] > 600)
		vmVariables.target[1] = 600;
		
	when(play) {
		play_sound_loop(SOUND_FREEFALL);
	}
	when(!play) {
		play_sound(SOUND_DISABLE);
	}
}
static void tick_draw(void) {
	
}


#define SOUND_TURN_SPEED 80
#define SOUND_FRONT_SPEED 80
static void tick_sound(void) {
	static char time;
	static char clap;
	static char disco;
#define SOUND_STOP 0
#define SOUND_RUN 1
#define SOUND_TURNRIGHT 2
	static char direction;
	static char claptime;
	
	if(IS_EVENT(EVENT_MIC)) {
		CLEAR_EVENT(EVENT_MIC);
		if(clap == 0) {
			time = 0;
			clap = 1;
			leds_set_circle(32,0,0,0,0,0,0,0);
		} else if(clap == 1 && 2 < time && time < 30) {
			clap = 2;
			claptime = time + 1;
			leds_set_circle(32,32,0,0,0,0,0,32);
		} else if(clap == 2 && claptime < time && time < 40) {
			clap = 3;
			leds_set_circle(32,32,32,0,0,0,32,32);
			play_sound(SOUND_F_OK);
			disco = 0;
		}
	}
	
	if(time < 100)
		time++;
		
	if(time > 50) {
		clap = 0;
		leds_set_circle(0,0,0,0,0,0,0,0);
	}	
	
	if(time == 5) {
		if(direction == SOUND_RUN && clap == 1) {
			direction = SOUND_TURNRIGHT;
			vmVariables.target[0] = SOUND_TURN_SPEED;
			vmVariables.target[1] = -SOUND_TURN_SPEED;
		} else if(direction == SOUND_TURNRIGHT && clap == 1) {
			direction = SOUND_RUN;
			vmVariables.target[0] = SOUND_FRONT_SPEED;
			vmVariables.target[1] = SOUND_FRONT_SPEED;
		}
	}
	if(time == 30) {
		if(direction == SOUND_STOP && clap == 2) {
			direction = SOUND_RUN;
			vmVariables.target[0] = SOUND_FRONT_SPEED;
			vmVariables.target[1] = SOUND_FRONT_SPEED;
		} else if(clap == 2) {
			direction = SOUND_STOP;
			vmVariables.target[0] = 0;
			vmVariables.target[1] = 0;
		}
	}
	
	if(time == 40) {
		if(clap == 3) {
			direction = SOUND_RUN;
			vmVariables.target[0] = SOUND_TURN_SPEED;
			vmVariables.target[1] = 0;
			disco = 1;
		}
	}	
	
	if(vmVariables.ground_delta[0] < 130 || vmVariables.ground_delta[1] < 130) {
		vmVariables.target[0] = 0;
		vmVariables.target[1] = 0;
		leds_set_br(32,0,0);
		leds_set_bl(32,0,0);
	} else {
		leds_set_br(0,0,0);
		leds_set_bl(0,0,0);
	}
	
	/* Body led managment */
	if(disco) {
		unsigned char rgb[3];

		rainbow_get(rgb);

		leds_set_top(rgb[0],rgb[1],rgb[2]);
		leds_set_bl(rgb[2],rgb[0],rgb[1]);
		leds_set_br(rgb[1],rgb[2],rgb[0]);
		disco++;
		if(disco == 127) {
			disco = 0;
			leds_set_bl(0,0,0);
			leds_set_br(0,0,0);
		}
	} else
		leds_set_top(0,0,body_color_pulse_get());
		
	
}

static void tick_line(void) {

#define STATE_BLACK 0
#define STATE_WHITE 1
	static unsigned char s[2];

	static char dir;
	unsigned char stop_moving = 0;
#define DIR_LEFT (-1)
#define DIR_L_LEFT (-2)
#define DIR_RIGHT (1)
#define DIR_L_RIGHT (2)
#define DIR_LOST	(10)
#define DIR_FRONT (0)

#define SPEED_LINE 300

	int p = body_color_pulse_get();

	leds_set_top(0, p, p);
	
// Calibration feature
	if(buttons_state[0] && buttons_state[3]) {
		bs_black_level = (vmVariables.ground_delta[0] + vmVariables.ground_delta[1]) / 2;
		bs_black_level += 150;
		stop_moving = 1;
	}
	
	if(buttons_state[1] && buttons_state[4]) {
		bs_white_level = (vmVariables.ground_delta[0] + vmVariables.ground_delta[1]) / 2;
		if(bs_white_level < 150)
			bs_white_level = 200;
		bs_white_level -= 150;
		stop_moving = 1;
	}

	// if the user is trying to calibrate, then don't try to move
	if(stop_moving) {
		leds_set_circle(0,0,0,0,0,0,0,0);
		vmVariables.target[0] = 0;
		vmVariables.target[1] = 0;
		return;
	}
	
	if(vmVariables.ground_delta[0] < bs_black_level)
		s[0] = STATE_BLACK;
	if(vmVariables.ground_delta[0] > bs_white_level)
		s[0] = STATE_WHITE;
	
	if(vmVariables.ground_delta[1] < bs_black_level)
		s[1] = STATE_BLACK;
	if(vmVariables.ground_delta[1] > bs_white_level)
		s[1] = STATE_WHITE;
	
	
	if(s[0] == STATE_BLACK && s[1] == STATE_BLACK)
		// Black line right under us
		dir = DIR_FRONT;
	else if (s[0] == STATE_WHITE && s[1] == STATE_BLACK)
		dir = DIR_RIGHT;
	else if (s[1] == STATE_WHITE && s[0] == STATE_BLACK)
		dir = DIR_LEFT;
	else {
		// Lost
		if (dir > 0) 
			dir = DIR_L_RIGHT;
		else if (dir < 0)
			dir = DIR_L_LEFT;
		else
			dir = DIR_LOST;
	}
	
	if(dir == DIR_FRONT) {
		vmVariables.target[0] = SPEED_LINE;
		vmVariables.target[1] = SPEED_LINE;
		leds_set_circle(32,0,0,0,32,0,0,0);
	} else if(dir == DIR_RIGHT) {
		vmVariables.target[0] = SPEED_LINE;
		vmVariables.target[1] = 0;
		leds_set_circle(0,32,0,32,0,0,0,0);
	} else if (dir == DIR_LEFT) {
		vmVariables.target[0] = 0;
		vmVariables.target[1] = SPEED_LINE;
		leds_set_circle(0,0,0,0,0,32,0,32);
	} else if (dir == DIR_L_LEFT) {
		vmVariables.target[0] = -SPEED_LINE;
		vmVariables.target[1] = SPEED_LINE;
		leds_set_circle(0,0,0,0,0,0,32,0);
	} else if (dir == DIR_L_RIGHT) {
		vmVariables.target[0] = SPEED_LINE;
		vmVariables.target[1] = -SPEED_LINE;
		leds_set_circle(0,0,32,0,0,0,0,0);
	} else if( dir == DIR_LOST) {
		vmVariables.target[0] = SPEED_LINE;
		vmVariables.target[1] = -SPEED_LINE;
//		leds_set_circle(32,32,32,32,32,32,32,32);
	}

}


#define RC5_SPEED_STEP 150
#define RC5_SPEED_SAT (RC5_SPEED_STEP*4)

static void tick_rc5(void) {
	int p = body_color_pulse_get();

	leds_set_top(p,0,p);

	when(buttons_state[1]) {
		rc5_speed_t = -RC5_SPEED_STEP;
	}
	
	when(buttons_state[4]) {
		rc5_speed_t = RC5_SPEED_STEP;
	}
	
	when(buttons_state[0]) {
		if(rc5_speed_t)
			rc5_speed_t = 0;
		else
			rc5_speed_l -= RC5_SPEED_STEP;
	}
	
	when(buttons_state[3]) {
		if(rc5_speed_t)
			rc5_speed_t = 0;
		else
			rc5_speed_l += RC5_SPEED_STEP;
	}

	when(buttons_state[0] && buttons_state[3]) 
		rc5_speed_l = 0;
	when(buttons_state[4] && buttons_state[1])
		rc5_speed_t = 0;
		
	if(vmVariables.rc5_command) {
		switch(vmVariables.rc5_command) {
			case 2:
			case 80:
			case 32:
				if(rc5_speed_t)
					rc5_speed_t = 0;
				else
					rc5_speed_l += RC5_SPEED_STEP;
				break;
			case 4:
			case 85:
			case 17:
			case 77:
				rc5_speed_t = -RC5_SPEED_STEP;
				break;
			case 8:
			case 81:
			case 33:
				if(rc5_speed_t)
					rc5_speed_t = 0;
				else
					rc5_speed_l -= RC5_SPEED_STEP;
				break;
			case 6:
			case 86:
			case 16:
			case 78:
				rc5_speed_t = RC5_SPEED_STEP;
				break;
			case 5:
			case 87:
			case 13:
				rc5_speed_t = 0;
				rc5_speed_l = 0;
				break;
		}
		vmVariables.rc5_command = 0;
	}
	
	
	if(rc5_speed_l > RC5_SPEED_SAT)
		rc5_speed_l = RC5_SPEED_SAT;
	if(rc5_speed_l < -RC5_SPEED_SAT)
		rc5_speed_l = -RC5_SPEED_SAT;
	if(rc5_speed_t > RC5_SPEED_SAT)
		rc5_speed_t = RC5_SPEED_SAT;
	if(rc5_speed_t < -RC5_SPEED_SAT)
		rc5_speed_t = -RC5_SPEED_SAT;
		
	vmVariables.target[0] = rc5_speed_l + rc5_speed_t;
	vmVariables.target[1] = rc5_speed_l - rc5_speed_t;
	
}
static void tick_side(void) { 
	
}	

static int mode_enabled(int temp) {
	if(	temp == MODE_DRAW ||
		temp == MODE_SIDE)
			return 0;
			
	if(temp == MODE_MENU && !vm_active) // Here mode menu == VM mode
		return 0;
		
	return 1;
}

static enum mode next_mode(enum mode m, int i) {
	int temp = m;
	
	do {
		temp += i;
	
		while(temp > MODE_MAX)
			temp -= MODE_MAX+1; 
		while(temp < 0)
			temp += MODE_MAX+1;
			
	} while(!mode_enabled(temp));
	
	return temp;
}

static enum mode _selecting;
void mode_tick(void) {
	
	static unsigned char ignore;
	
	ignore++;
	// As the user mode disable the "mode menu thing"... 
	if(ignore > 100) {
		ignore = 101;
		when(buttons_state[2]) {
			exit_mode(current_mode);
			if(_selecting == MODE_MENU) {
				// Special case, if we select the mode menu stuff
				behavior_stop(B_MODE | B_SETTING);
				init_vm_mode();
				return;
			}
			if(_selecting == current_mode) {
				init_mode(MODE_MENU);
				current_mode = MODE_MENU;
			} else {
				init_mode(_selecting);
				current_mode = _selecting;
			}
		}
	}	
	
	 // Exit case: Serial port open !
	if(usb_uart_serial_port_open() || (rf_get_status() & RF_DATA_RX)) {
		exit_mode(current_mode);
		behavior_stop(B_MODE);
		init_vm_mode();
		return;
	}
		

	switch(current_mode) {
		case MODE_MENU:
			when(buttons_state[0]) {
				_selecting = next_mode(_selecting, -1);
			}
			when(buttons_state[1]) {
				_selecting = next_mode(_selecting, -1);
			}
			when(buttons_state[3]) {
				_selecting = next_mode(_selecting, 1);
			}
			when(buttons_state[4]) {
				_selecting = next_mode(_selecting, 1);
			}
				
			set_mode_color(_selecting);
			break;
		case MODE_FOLLOW:
			tick_follow();
			break;
		case MODE_EXPLORER:
			tick_explorer();
			break;
		case MODE_ACC:
			tick_acc();
			break;
		case MODE_DRAW:
			tick_draw();
			break;
		case MODE_SOUND:
			tick_sound();
			break;
		case MODE_LINE:
			tick_line();
			break;
		case MODE_RC5:
			tick_rc5();
			break;
		case MODE_SIDE:
			tick_side();
			break;	
	}	
}

void mode_init(int vm_enabled) {
	// Init the defaults behaviors + our behavior.
	
	vm_active = vm_enabled;
	
	init_mode(MODE_MENU);
	
	if(vm_active) 
		_selecting = 0;
	else {
		_selecting = MODE_FOLLOW; // Bypass the "VM exit mode"
		set_mode_color(_selecting);
	}
	
	behavior_start(B_ALWAYS | B_MODE);

}

int mode_get(void) {
	if(behavior_enabled(B_MODE))
		return current_mode;
	else
		return -1;
}

