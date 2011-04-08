#include "mode.h"
#include "behavior.h"
#include "leds.h"
#include "button.h"
#include "test.h"
#include "usb_uart.h"
#include "playback.h"
#include <skel-usb.h>


// Macro to do a when-like as in aseba
// We put all the state into a section so we can
// Zero it on reinit.

#define when(cond) if(({static unsigned char __attribute((section("when"))) p; \
						unsigned char c = !!(cond); \
						unsigned char r = c && !p; \
						p = c; \
						r;}))

enum mode {
	MODE_MENU = 0, // It's also the entry point for the user mode
	MODE_FOLLOW,
	MODE_EXPLORER,
	MODE_ACC,
	MODE_DRAW,
	MODE_MUSIC,
	MODE_LINE,
	MODE_RC5,
	MODE_SIDE,
	MODE_MAX = MODE_SIDE,
};

static enum mode current_mode;

static void set_body_rgb(unsigned int r, unsigned int g, unsigned int b) {
	leds_set_top(r,g,b);
	leds_set_br(r,g,b);
	leds_set_bl(r,g,b);
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
		case MODE_MUSIC:
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
// TODO
	set_body_rgb(0,0,0);
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
			
			break;
		case MODE_FOLLOW:
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
			// TODO: Stop playing sound
			// we don't need as the button to switch the mode 
			// has been pressed, so generated a sound.
		//	play_sound(SOUND_DISABLE);
			break;
		case MODE_DRAW:
			behavior_stop(B_LEDS_PROX);
			break;
		case MODE_MUSIC:
			behavior_stop(B_LEDS_PROX);
			break;
		case MODE_LINE:
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

static void init_vm_mode(void) {
	// do a menu mode -> vm mode (exit internal state machine)
	behavior_start(B_LEDS_ACC);
	behavior_start(B_LEDS_NTC);
	behavior_start(B_LEDS_MIC);
	behavior_start(B_LEDS_PROX);
	behavior_start(B_SOUND_BUTTON);
}

static void init_mode(enum mode m) {
	set_mode_color(m);
	
	switch(m) {
		case MODE_MENU:
		/* Nothing to do ... */
			break;
		case MODE_FOLLOW:
			leds_set_circle(0,0,0,32,32,32,0,0);
			behavior_start(B_LEDS_PROX);
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
		case MODE_MUSIC:
			behavior_start(B_LEDS_PROX);
			break;
		case MODE_LINE:
			behavior_start(B_LEDS_PROX);
			break;
		case MODE_RC5:
			behavior_start(B_LEDS_PROX);
			break;
		case MODE_SIDE:
			behavior_start(B_LEDS_PROX);
			break;	
	}		
}

static void tick_follow(void) {
	static char led_pulse;
	static char sound_done = 0;
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
		vmVariables.target[0] = 0;
		vmVariables.target[1] = 0;
	} else {
		vmVariables.target[1] = speed_diff + speed_l;
		vmVariables.target[0] = speed_l - speed_diff;
	}

/* Body led managment */	
	led_pulse = led_pulse + 1;
	if(led_pulse > 0) { 
		set_body_rgb(0,led_pulse,0);
		if(led_pulse > 40)
			led_pulse = -128;
	} else 
		set_body_rgb(0,-led_pulse / 4,0);
		
		
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
	

	
	if(vmVariables.ground_delta[0] < 130 || vmVariables.ground_delta[0] < 130) {
		vmVariables.target[0] = 0;
		vmVariables.target[1] = 0;
		leds_set(LED_R_BOT_L, 32);
		leds_set(LED_R_BOT_R, 32);
	} else {
		leds_set(LED_R_BOT_L, 0);
		leds_set(LED_R_BOT_R, 0);
	}
	
	
}
static void tick_explorer(void) {
	static unsigned char led_state;
	static  int speed = 250;
	
	unsigned char l[8] = {0,0,0,0,0,0,0,0};
	unsigned char fixed;

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
	
	if(vmVariables.ground_delta[0] < 130 || vmVariables.ground_delta[0] < 130) {
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
	static unsigned int acc = 32;
	static int counter;
	acc = acc + acc + acc + abs(vmVariables.acc[0]) + abs(vmVariables.acc[1]) + abs(vmVariables.acc[2]);
	acc >>= 2;
	when(acc < 5) {
		play_sound_loop(SOUND_FREEFALL);
	}
	when(acc > 5) {	
		play_sound_loop(SOUND_DISABLE);
		set_body_rgb(15,0,0);
	}
	
	if(acc < 5) {
		counter++;
		if(counter > 5) {
			if (counter == 10)
				counter = 0;
			
			set_body_rgb(32,0,0);
		} else {
			set_body_rgb(0,0,0);
		}
	}
	
	if(vmVariables.acc_tap) {
		vmVariables.acc_tap = 0;
		play_sound(SOUND_TAP);
	}
}
static void tick_draw(void) {
	
}
static void tick_music(void) {
	
}

static void tick_line(void) {
	static unsigned int black_level = 200;
	static unsigned int white_level = 400;
#define STATE_BLACK 0
#define STATE_WHITE 1
	static unsigned char s[2];

	static char dir;
#define DIR_LEFT (-1)
#define DIR_L_LEFT (-2)
#define DIR_RIGHT (1)
#define DIR_L_RIGHT (2)
#define DIR_LOST	(10)
#define DIR_FRONT (0)

#define SPEED_LINE 300
	
	
// Calibration feature
	if(buttons_state[0] && buttons_state[3]) {
		black_level = (vmVariables.ground_delta[0] + vmVariables.ground_delta[1]) / 2;
		black_level += 150;
	}
	
	if(buttons_state[1] && buttons_state[4]) {
		white_level = (vmVariables.ground_delta[0] + vmVariables.ground_delta[1]) / 2;
		if(white_level < 150)
			white_level = 200;
		white_level -= 150;
	}
	
	if(vmVariables.ground_delta[0] < black_level)
		s[0] = STATE_BLACK;
	if(vmVariables.ground_delta[0] > white_level)
		s[0] = STATE_WHITE;
	
	if(vmVariables.ground_delta[1] < black_level)
		s[1] = STATE_BLACK;
	if(vmVariables.ground_delta[1] > white_level)
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
static void tick_rc5(void) {
	
	static int s_l;
	static int s_t;

	when(buttons_state[1]) {
		s_t = -200;
	}
	
	when(buttons_state[4]) {
		s_t = 200;
	}
	
	when(buttons_state[0]) {
		if(s_t)
			s_t = 0;
		else
			s_l -= 200;
	}
	
	when(buttons_state[3]) {
		if(s_t)
			s_t = 0;
		else
			s_l += 200;
	}

	when(buttons_state[0] && buttons_state[3]) 
		s_l = 0;
	when(buttons_state[4] && buttons_state[1])
		s_t = 0;
		
	if(vmVariables.rc5_command) {
		switch(vmVariables.rc5_command) {
			case 2:
				if(s_t)
					s_t = 0;
				else
					s_l += 200;
				break;
			case 4:
				s_t = -200;
				break;
			case 8:
				if(s_t)
					s_t = 0;
				else
					s_l -= 200;
				break;
			case 6:
				s_t = 200;
				break;
		}
		vmVariables.rc5_command = 0;
	}
	
	
	if(s_l > 600) 
		s_l = 600;
	if(s_l < -600)
		s_l = -600;
	if(s_t > 600)
		s_t = 600;
	if(s_t < -600)
		s_t = -600;
		
	vmVariables.target[0] = s_l + s_t;
	vmVariables.target[1] = s_l - s_t;
	
}
static void tick_side(void) {
	
}	

static int mode_enabled(int temp) {
	if(	temp == MODE_DRAW || 
		temp == MODE_MUSIC ||
		temp == MODE_SIDE )
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

void mode_tick(void) {
	static enum mode _selecting;
	static unsigned char ignore;
	
	ignore++;
	// As the user mode disable the "mode menu thing"... 
	if(ignore > 100) {
		ignore = 101;
		when(buttons_state[2]) {
			exit_mode(current_mode);
			if(_selecting == MODE_MENU) {
				// Special case, if we select the mode menu stuff
				behavior_stop(B_MODE);
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
	if(usb_uart_serial_port_open()) {
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
		case MODE_MUSIC:
			tick_music();
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

void mode_init(void) {
	// Init the defaults behaviors + our behavior.
	
	// TODO: Check that SD card is present, if not disable "Music" mode.
	init_mode(MODE_MENU);
	behavior_start(B_ALWAYS | B_MODE);

}


