#include <timer/timer.h>

#include "behavior.h"
#include "leds.h"
#include "button.h"
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


static int timer;
static unsigned int behavior;

#define ENABLED(b) ({behavior & b;})
#define ENABLE(b) do {behavior |= b;} while(0)
#define DISABLE(b) do {behavior &= ~b;} while(0)

static unsigned char led_sd;


static void behavior_sd(void) {
	static char sd_led_counter;
	
	if(led_sd) {
		sd_led_counter += 2;
	
		if(sd_led_counter >= 32) 
			sd_led_counter = -32;
		leds_set(LED_SD_CARD, abs(sd_led_counter));
	} 
	
	when(led_sd == 0) {
		leds_set(LED_SD_CARD, 0);
	}
}

static void behavior_battery(void) {
	static char counter;
	
	// First check if 5V is present, if it's the case then 
	// Do a funny sequence on the battery led
	// Else display the battery level
	// Blink if low.
	
	if(U1OTGSTATbits.SESVD) {
		static char state;
		// On 5V
		int i = counter ? counter : 1;
		counter += i >20 ? 15 : i;
		
		if(counter > 100) {
			state ++;
			counter = 1;
			if(state == 3) {
				state = 0;
			}
			switch(state) {
				case 0:
					leds_set(LED_BATTERY_2, 0);
					break;
				case 1:
					leds_set(LED_BATTERY_0, 0);
					break;
				case 2:
					leds_set(LED_BATTERY_1, 0);	
			}
		}
		leds_set(LED_BATTERY_0 + state, counter);
	} else {
		unsigned int bat = vmVariables.vbat[0] + vmVariables.vbat[1];
		unsigned long temp = __builtin_mulss(bat,1000);
		bat = __builtin_divud(temp,3978);
		
		// On battery
		// >3.8 three bar
		// 3.5-3.8 two bar
		// 3.4-3.5 one bar
		// < 3.4 blink one bar
		
		when(bat >= 380) {
			leds_set(LED_BATTERY_0, 32);
			leds_set(LED_BATTERY_1, 32);
			leds_set(LED_BATTERY_2, 32);
		}
		when(bat > 350 && bat < 380) {
			leds_set(LED_BATTERY_0, 32);
			leds_set(LED_BATTERY_1, 32);
			leds_set(LED_BATTERY_2, 0);
		}
		when(bat > 340 && bat <= 350) {
			leds_set(LED_BATTERY_0, 32);
			leds_set(LED_BATTERY_1, 0);
			leds_set(LED_BATTERY_2, 0);
		}
		if(bat <= 340) {
			counter++;
			if(counter == 3) 
				leds_set(LED_BATTERY_0, 32);

			if(counter > 5) {
				leds_set(LED_BATTERY_0, 0);
				counter = 0;
			}
		}
	}
}

static void behavior_leds_buttons(void) {
	static unsigned char button_counter[5];
	int i;
	for(i = 0; i < 5; i++) {
		if(buttons_state[i]) {
			button_counter[i] +=  3;
			if(button_counter[i] > 32)
				button_counter[i] = 32;
		} else {
			button_counter[i] = 0;
		}
	}
	if(button_counter[2]) {
		for(i = 32; i < 36; i++)
			leds_set(i, button_counter[2]);
	} else {
		if(button_counter[0]) 
			leds_set(33,button_counter[0]);

		if(button_counter[1]) 
			leds_set(34,button_counter[1]);

		if(button_counter[3]) 
			leds_set(32, button_counter[3]);
			
		if(button_counter[4]) 
			leds_set(35, button_counter[4]);
	}
	
	when(button_counter[0] == 0 && button_counter[2] == 0) {
		leds_set(33,0);
	}
	when(button_counter[1] == 0 && button_counter[2] == 0) {
		leds_set(34, 0);
	}
	when(button_counter[3] == 0 && button_counter[2] == 0) {
		leds_set(32, 0);
	}
	when(button_counter[4] == 0 && button_counter[2] == 0) {
		leds_set(35, 0);
	}
}

static void behavior_sound_buttons(void) {
	when(buttons_state[0]) 
		play_sound(SOUND_BUTTON);
		
	when(buttons_state[1])
		play_sound(SOUND_BUTTON);
	
	when(buttons_state[2])
		play_sound(SOUND_BUTTON);
	
	when(buttons_state[3])
		play_sound(SOUND_BUTTON);
	
	when(buttons_state[4])
		play_sound(SOUND_BUTTON);	
}


static void timer_cb(int timer_id) {
	if(ENABLED(B_LEDS_SD)) 
		behavior_sd();
	
	if(ENABLED(B_LEDS_BATTERY)) 
		behavior_battery();
		
	if(ENABLED(B_LEDS_BUTTON))
		behavior_leds_buttons();
	
	if(ENABLED(B_SOUND_BUTTON))
		behavior_sound_buttons();
			
}

void behavior_init(int t, int prio) {
	timer = t;
	timer_init(timer, 100, 3);
	timer_enable_interrupt(timer,timer_cb, prio);
}

void behavior_start(int b) {
	ENABLE(b);

	if(behavior)
		timer_enable(timer);
}

void behavior_stop(int b) {
	
	DISABLE(b);
	
	if(!behavior) {
		// TODO: Zero the when section
		timer_disable(timer);
	}
}

void behavior_notify_sd(unsigned int rw) {
	if(rw & BEHAVIOR_START) {
	 	led_sd |= rw & 0x3;
	} else 
		led_sd &= ~rw;
}
