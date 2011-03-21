#include "mode.h"
#include "behavior.h"
#include "leds.h"
#include <skel-usb.h>

enum mode {
	MODE_MENU = 0,
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
	leds_set(LED_R_BOT_L, r);
	leds_set(LED_R_BOT_R, r);

	leds_set(LED_G_TOP, g);
	leds_set(LED_G_BOT_L, g);
	leds_set(LED_G_BOT_R, g);
	
	leds_set(LED_B_TOP, b);
	leds_set(LED_B_BOT_L, b);
	leds_set(LED_B_BOT_R, b);
}

static void set_mode_color(void) {
	switch (current_mode) {
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
			set_body_rgb(0,0,0); // TODO enable a rainbow mode ?
			break;
	}
}

static void switch_to_mode(enum mode new) {
	// TODO: put everything back to idle
// Motors
	vmVariables.target[0] = 0;
	vmVariables.target[1] = 0;
// Leds ...
	leds_clear_all();
// Sound ? I let it as is, so we have the end of the "button" music
	
	
	
}


void mode_tick(void) {
	static enum mode _selecting;
	// IF we are called here it mean we are not in the "User mode"
	// As the user mode disable the "mode menu thing"... 
	when(buttons_state[2]) {
		switch_to_mode(_selecting);
	}
	
	// TODO: Tick to the differents mode controller ... 
}

void mode_init(void) {
	// Init the defaults behaviors + our behavior.
	
	// TODO: Check that SD card is present, if not disable "Music" mode.
	
	behavior_start(B_ALWAYS | B_MODE);
	switch_to_mode(MODE_MENU);
}


