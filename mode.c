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
			leds_set_top(32,0,32);
			break;
		case MODE_MUSIC:
			leds_set_top(0,0,32);
			break;
		case MODE_LINE:
			leds_set_top(0,32,32);
			break;
		case MODE_RC5:
			leds_set_top(32,32,32);
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
			play_sound(SOUND_DISABLE);
			break;
		case MODE_DRAW:
			behavior_stop(B_LEDS_PROX);
			break;
		case MODE_MUSIC:
			behavior_stop(B_LEDS_PROX);
			break;
		case MODE_LINE:
			behavior_stop(B_LEDS_PROX);
			break;
		case MODE_RC5:
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
	static int speed = 300;
#define DETECT 500 

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
	
	if(vmVariables.prox[0] > DETECT || 
		vmVariables.prox[1] > DETECT ||
		vmVariables.prox[2] > DETECT ||
		vmVariables.prox[3] > DETECT ||
		vmVariables.prox[4] > DETECT) {
		long temp1;
		long temp2;
		
		temp1 = vmVariables.prox[0];
		temp1 += vmVariables.prox[1]; 
		temp1 += (vmVariables.prox[2] - 2500) * 4;
		temp1 += vmVariables.prox[3];
		temp1 += vmVariables.prox[4];
		
		temp2 = -3 * vmVariables.prox[0];
		temp2 += -2 * vmVariables.prox[1];
		temp2 += 2 * vmVariables.prox[3];
		temp2 += 3 * vmVariables.prox[4];
		
		vmVariables.target[0] = speed - __builtin_divsd((temp1 - temp2) * speed, 2000);
		vmVariables.target[1] = speed - __builtin_divsd((temp1 + temp2) * speed, 2000);
	} else {
		vmVariables.target[0] = 0;
		vmVariables.target[1] = 0;
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
	acc += abs(vmVariables.acc[0]) + abs(vmVariables.acc[1]) + abs(vmVariables.acc[2]);
	acc >>= 1;
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
	
}
static void tick_rc5(void) {
	
}
static void tick_side(void) {
	
}		

static enum mode next_mode(enum mode m, int i) {
	int temp = m;
	temp += i;
	
	// TODO: Add here a check if the mode is disabled
	
	while(temp > MODE_MAX)
		temp -= MODE_MAX+1; 
	while(temp < 0)
		temp += MODE_MAX+1;
	
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
	behavior_start(B_ALWAYS | B_MODE | B_LEDS_BUTTON);

}


