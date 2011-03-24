#include "mode.h"
#include "behavior.h"
#include "leds.h"
#include "button.h"
#include "test.h"
#include "usb_uart.h"
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
	leds_set(LED_R_TOP, r);
//	leds_set(LED_R_BOT_L, r);
//	leds_set(LED_R_BOT_R, r);

	leds_set(LED_G_TOP, g);
//	leds_set(LED_G_BOT_L, g);
//	leds_set(LED_G_BOT_R, g);
	
	leds_set(LED_B_TOP, b);
//	leds_set(LED_B_BOT_L, b);
//	leds_set(LED_B_BOT_R, b);
}

static void set_mode_color(enum mode m) {
	switch (m) {
		case MODE_MENU:
			set_body_rgb(0,0,0);
			break;
		case MODE_FOLLOW:
			set_body_rgb(0,32,0);
			break;
		case MODE_EXPLORER:
			set_body_rgb(32,32,0);
			break;
		case MODE_ACC:
			set_body_rgb(32,0,0);
			break;
		case MODE_DRAW:
			set_body_rgb(32,0,32);
			break;
		case MODE_MUSIC:
			set_body_rgb(0,0,32);
			break;
		case MODE_LINE:
			set_body_rgb(0,32,32);
			break;
		case MODE_RC5:
			set_body_rgb(32,32,32);
			break;
		case MODE_SIDE:
			set_body_rgb(1,1,1); // TODO enable a rainbow mode ?
			break;
	}
}

static void exit_mode(enum mode m) {
// TODO
	set_body_rgb(0,0,0);
	
	switch(m) {
		case MODE_MENU:
			
			break;
		case MODE_FOLLOW:

			break;
		case MODE_EXPLORER:

			break;
		case MODE_ACC:
		
			break;
		case MODE_DRAW:
		
			break;
		case MODE_MUSIC:
	
			break;
		case MODE_LINE:
	
			break;
		case MODE_RC5:

			break;
		case MODE_SIDE:
			break;	
	}	
}

static void init_vm_mode(void) {
	// do a menu mode -> vm mode (exit internal state machine)
	behavior_start(B_LEDS_ACC);
	behavior_start(B_LEDS_NTC);
	behavior_start(B_LEDS_MIC);
	behavior_start(B_SOUND_BUTTON);
}

static void init_mode(enum mode m) {
// TODO
	switch(m) {
		case MODE_MENU:
		/* Nothing to do ... */
			break;
		case MODE_FOLLOW:

			break;
		case MODE_EXPLORER:

			break;
		case MODE_ACC:
		
			break;
		case MODE_DRAW:
		
			break;
		case MODE_MUSIC:
	
			break;
		case MODE_LINE:
	
			break;
		case MODE_RC5:

			break;
		case MODE_SIDE:
			break;	
	}		
}

static void tick_follow(void) {
	
}
static void tick_explorer(void) {
}

static void tick_acc(void) {
}
static void tick_draw() {
}
static void tick_music(void) {
}

static void tick_line(void) {
}
static void tick_rc5(void) {
}
static void tick_side(void) {
}		

void mode_tick(void) {
	static enum mode _selecting;
	// As the user mode disable the "mode menu thing"... 
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
				_selecting--;
			}
			when(buttons_state[1]) {
				_selecting--;
			}
			when(buttons_state[3]) {
				_selecting++;
			}
			when(buttons_state[4]) {
				_selecting++;
			}
			
			while(_selecting > MODE_MAX)
				_selecting -= MODE_MAX+1;
			while(_selecting < 0)
				_selecting += MODE_MAX+1;
				
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
	
	behavior_start(B_ALWAYS | B_MODE | B_LEDS_PROX | B_LEDS_BUTTON);
	init_mode(MODE_MENU);
}


