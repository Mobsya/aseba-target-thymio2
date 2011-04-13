#include <types/types.h>
#include <skel-usb.h>

#include "analog.h"
//#include "sensors.h"
#include "button.h"
#include "sound.h"
#include "motor.h"
#include "ground_ir.h"
#include "leds.h"

// Callback from the ADC @ 8kHz
// Capacitive touch sensors availble at 8/5Khz
static int button[5];
static int count;

static int timer;

static char leds_timer;

void new_sensors_value(unsigned int * val, int b) {
	/* Sound ... */
	sound_new_sample(val[3]);
	
	/* Motors */
	motor_new_analog(val[5],val[4]);
	
	/* IR sensors ... */
	ground_ir_new(val[1], val[2]);
	

	leds_tick_cb();

	/* ~1Khz timer */
	if(timer++ == 7) {
		timer = 0;
		cb_1khz();
	}
	
/* Capacitive button first stage filtering ... */
	button[b] += val[0];
	
	// last value ... call the processing algos...
	if(count == 31) 
		button_process(button[b],b);
	
	if(b == 4) {
		count++;
		if(count == 32) {
			SET_EVENT(EVENT_BUTTONS);
			count = 0;
			button[0] = 0;
			button[1] = 0;
			button[2] = 0; 
			button[3] = 0;
			button[4] = 0;
		}
	}
}	
