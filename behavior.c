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


#include <timer/timer.h>

#include "behavior.h"
#include "leds.h"
#include "button.h"
#include "playback.h"
#include "rc5.h"
#include "mode.h"
#include "test.h"
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
	static char was_charging = 0;
	
	// First check if 5V is present, if it's the case then 
	// Do a funny sequence on the battery led
	// Else display the battery level
	// Blink if low.
	
	if(U1OTGSTATbits.SESVD) {
		static char state;
		
		if(!was_charging) {
			// switch off everything.
			leds_set(LED_BATTERY_0, 0);
			leds_set(LED_BATTERY_1, 0);
			leds_set(LED_BATTERY_2, 0);
			was_charging = 1;
		}
		
		// On 5V
		int i = counter ? counter : 1;
		counter += i > 10 ? 7 : i/2 + 1;
		
		if(counter > 100) {
			state ++;
			counter = 1;
			if(state == 3) {
				state = 0;
			}
			switch(state) {
				case 0:
					leds_set(LED_BATTERY_2, 0);
					leds_set(LED_BATTERY_1, 0);
					break;
			}
		}
		leds_set(LED_BATTERY_0 + state, counter);
	} else {
		unsigned int bat = vmVariables.vbat[0] + vmVariables.vbat[1];
		unsigned long temp = __builtin_mulss(bat,1000);
		bat = __builtin_divud(temp,3978);
		
#define BAT_HIGH 390
#define BAT_MIDDLE 360
#define BAT_LOW 340
	
		if(was_charging) {
			was_charging = 0;
			if(bat >= BAT_HIGH) {
				leds_set(LED_BATTERY_0, 32);
				leds_set(LED_BATTERY_1, 32);
				leds_set(LED_BATTERY_2, 32);
			} else if (bat > BAT_MIDDLE) {
				leds_set(LED_BATTERY_0, 32);
				leds_set(LED_BATTERY_1, 32);
				leds_set(LED_BATTERY_2, 0);
			} else if (bat > BAT_LOW) {
				leds_set(LED_BATTERY_0, 32);
				leds_set(LED_BATTERY_1, 0);
				leds_set(LED_BATTERY_2, 0);
			}
		}
		
		// On battery
		// >3.9 three leds
		// 3.6-3.9 two leds
		// 3.4-3.55 one led
		// < 3.4 blink one led
		
		
		
		when(bat >= BAT_HIGH) {
			leds_set(LED_BATTERY_0, 32);
			leds_set(LED_BATTERY_1, 32);
			leds_set(LED_BATTERY_2, 32);
		}
		when(bat > BAT_MIDDLE && bat < (BAT_HIGH - 5)) {
			leds_set(LED_BATTERY_0, 32);
			leds_set(LED_BATTERY_1, 32);
			leds_set(LED_BATTERY_2, 0);
		}
		when(bat > BAT_LOW && bat <= (BAT_MIDDLE - 5)) {
			leds_set(LED_BATTERY_0, 32);
			leds_set(LED_BATTERY_1, 0);
			leds_set(LED_BATTERY_2, 0);
		}
		when(bat <= BAT_LOW) {
			leds_set(LED_BATTERY_1, 0);
			leds_set(LED_BATTERY_2, 0);
		}
		if(bat <= BAT_LOW) {
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
		play_sound(SOUND_BUTTON_M);
	
	when(buttons_state[3])
		play_sound(SOUND_BUTTON);
	
	when(buttons_state[4])
		play_sound(SOUND_BUTTON);	
}


static void behavior_leds_prox(void) {
	// 5 front, two back, two ground
	static int max[9] = {4000,4000,4000,4000,4000,4000,4000,900,900};
	static int min[9] = {900,900,900,900,900,900,900,0,0};
	static unsigned char led[9] = {16,17,20,18,19,0,1,22,23};
	
// Do some adaptative stuff, so first we have a "standard" model of the sensor
// Then everytime we are called, we readapt the max&min value of the sensors
// So we can get the real range and display it on the leds.

	int i;
	
	// Todo: fixme: use vmVariables ? user can corrupt ... 
	for(i = 0; i < 7; i++) {
		if(max[i] < vmVariables.prox[i])
			max[i] = vmVariables.prox[i];
		if(vmVariables.prox[i] != 0 && min[i] > vmVariables.prox[i])
			min[i] = vmVariables.prox[i];
	}
	for(i = 0; i < 2; i++) {
		if(max[i+7] < vmVariables.ground_delta[i])
			max[i+7] = vmVariables.ground_delta[i];
		// min is fixed to 0 ... this is _physical_ 	
	}

	// Do a linear transformation from min-max to led 0-31!
	for(i = 0; i < 7; i++) {
		// Because of the min&max calculation above, we cannot have a
		// Division by 0 here.
		int s = vmVariables.prox[i] - min[i];
		int d = max[i] - min[i];
		
		if(s < 0) 
			s = 0;
			
		int b = __builtin_divsd(__builtin_mulss(s, 32),d);
		leds_set(led[i], b);
		// Special case sensor 2 has two leds
		if(i == 2) 
			leds_set(led[i] + 1, b);
	}
	
	for(i = 0; i < 2; i++) {
		int s = vmVariables.ground_delta[i] > 0 ? vmVariables.ground_delta[i] : 0;
		int b = __builtin_divsd(__builtin_mulss(s,32),max[i + 7]);
		leds_set(led[i+7], b);
	}
}

static void behavior_leds_rc5(void) {
	// Switch on for a short time when we have a valid rc5 code ..
	when(rc5_valid_flag == 0) {
		leds_set(LED_RC, 0);
	}
	
	if(rc5_valid_flag) {
		rc5_valid_flag = 0;
		leds_set(LED_RC, 32);
	}
}

sint16 aseba_atan2(sint16 y, sint16 x); // We use a function which should be private to aseba native ...

static void behavior_leds_acc(void) {
	static int previous_led;

	int intensity;
	int led = -1;
	
	// FIXME: Use vmVariables ?!
	if(vmVariables.acc[2] < 21) {
		int ha = aseba_atan2(vmVariables.acc[0], vmVariables.acc[1])/2;
		if(ha >= -2000 && ha < 2000)
			led = 28;
		else if (ha < -2000 && ha >= -6000) 
			led = 27;
		else if (ha < -6000 && ha >= -10000)
			led = 26;
		else if (ha < -10000 && ha >= -14000)
			led = 25;
		else if (ha  < -14000 || ha >= 14000)
			led = 24;
		else if (ha < 6000 && ha >= 2000) 
			led = 36;
		else if (ha < 10000 && ha >= 6000)
			led = 37;
		else if (ha < 14000 && ha >= 10000) 
			led = 31;
		
		intensity = 40 - abs(vmVariables.acc[2])*2;
		if(intensity < 0)
			intensity = 0;
		
		if(led >= 0) {
			if(previous_led >= 0)
				leds_set(previous_led, 0);
			leds_set(led, intensity);
		}	
		previous_led = led;
	} else {
		if(previous_led >= 0)
			leds_set(previous_led, 0);
			
		previous_led = -1;
	}
}

static void behavior_leds_ntc(void ) {
	// Fixme: Use vmVariables ?!
	static unsigned char counter = 0;

#define TEMP_HOT 280
#define TEMP_COLD 150

	if(++counter == 10) {
		counter = 0;
		// transistion from TEMP_COLD to TEMP_HOT
		if(vmVariables.ntc < TEMP_COLD) {
			leds_set(LED_TEMP_RED, 0);
			leds_set(LED_TEMP_BLUE, 32);
		} else if(vmVariables.ntc > TEMP_HOT) {
			leds_set(LED_TEMP_RED, 32);
			leds_set(LED_TEMP_BLUE, 0);
		} else {
			int t = vmVariables.ntc - TEMP_COLD;
			t = (t * 32) / (TEMP_HOT-TEMP_COLD);
			leds_set(LED_TEMP_RED, t);
			leds_set(LED_TEMP_BLUE, 32 - t);
		}
	}
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

	if(ENABLED(B_LEDS_PROX)) 
		behavior_leds_prox();

	if(ENABLED(B_LEDS_RC5))
		behavior_leds_rc5();
		
	if(ENABLED(B_LEDS_ACC))
		behavior_leds_acc();
		
	if(ENABLED(B_LEDS_NTC))
		behavior_leds_ntc();
		
	if(ENABLED(B_MODE))
		mode_tick();
		
	if(ENABLED(B_TEST))
		test_mode_tick();
}

void behavior_init(int t, int prio) {
	timer = t;
	timer_init(timer, 20, 3);
	timer_enable_interrupt(timer,timer_cb, prio);
}

void behavior_start(int b) {
	if(!behavior)
		timer_enable(timer);

	ENABLE(b);
}

int behavior_enabled(int b) {
	return ENABLED(b);
}

void behavior_stop(int b) {
	
	DISABLE(b);
	
	if(!behavior) {
		// TODO: Zero the when section ?
		timer_disable(timer);
	}
}

static unsigned char sound_led = 0;
void behavior_sound_mic(unsigned char level) {
	unsigned char pl = sound_led;
	unsigned char nl = level >> 3;

	if(!ENABLED(B_LEDS_MIC))
		return;

	if(vmVariables.sound_tresh) {
		if(level > vmVariables.sound_tresh && nl > sound_led)
			sound_led = nl;
		else {
			if(sound_led)
				sound_led-- ;
		}
			
	} else
		sound_led = 0;
	
	if(pl != sound_led) 
		leds_set(LED_SOUND, sound_led);
}

// Some helper for the SD card, special case because 
// of no real & proper way to do a refcounting in the SD code
void behavior_notify_sd(unsigned int rw) {
	if(rw & BEHAVIOR_START) {
	 	led_sd |= rw & 0x3;
	} else 
		led_sd &= ~rw;
}
