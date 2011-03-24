#include <clock/clock.h>
#include "leds.h"
#include "behavior.h"
#include "usb_uart.h"
#include "sound.h"
#include <skel-usb.h>

// Note: written after a 10 hours work day...

#define WAIT_SHORT 350

static void wait_ms(unsigned int ms) {
	unsigned int i;
	for(i = 0; i < ms; i++)
		clock_delay_us(1000);
}

static void test_led_ir(void) {
	leds_set(LED_FRONT_IR_0,32);
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_FRONT_IR_0,0);
	leds_set(LED_FRONT_IR_1,32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_FRONT_IR_1, 0);
	leds_set(LED_FRONT_IR_2A, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_FRONT_IR_2A, 0);
	leds_set(LED_FRONT_IR_2B, 32);

	wait_ms(WAIT_SHORT);
	
	leds_set(LED_FRONT_IR_2B, 0);
	leds_set(LED_FRONT_IR_3, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_FRONT_IR_3, 0);
	leds_set(LED_FRONT_IR_4, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_FRONT_IR_4, 0);
}

static void test_led_circle(void) {
	leds_set(LED_CIRCLE_4, 32);
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_CIRCLE_4, 0);
	leds_set(LED_CIRCLE_5, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_CIRCLE_5, 0);
	leds_set(LED_CIRCLE_6, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_CIRCLE_6, 0);
	leds_set(LED_CIRCLE_7, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_CIRCLE_7 ,0);
	leds_set(LED_CIRCLE_0, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_CIRCLE_0, 0);
	leds_set(LED_CIRCLE_1, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_CIRCLE_1, 0);
	leds_set(LED_CIRCLE_2, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_CIRCLE_2, 0);
	leds_set(LED_CIRCLE_3, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_CIRCLE_3, 0);
}

static void test_led_buttons(void) {
	leds_set(LED_BUTTON_3, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_BUTTON_3, 0);
	leds_set(LED_BUTTON_0, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_BUTTON_0, 0);
	leds_set(LED_BUTTON_1, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_BUTTON_1, 0);
	leds_set(LED_BUTTON_2, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_BUTTON_2, 0);
}

static void test_led_rgb(void) {
	leds_set(LED_R_BOT_L, 32);
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_R_BOT_L, 0);
	leds_set(LED_G_BOT_L, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_G_BOT_L, 0);
	leds_set(LED_B_BOT_L, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_B_BOT_L, 0);
	leds_set(LED_R_BOT_R, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_R_BOT_R, 0);
	leds_set(LED_G_BOT_R, 32);
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_G_BOT_R, 0);
	leds_set(LED_B_BOT_R, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_B_BOT_R, 0);
	leds_set(LED_R_TOP, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_R_TOP, 0);
	leds_set(LED_G_TOP, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_G_TOP, 0);
	leds_set(LED_B_TOP, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_B_TOP, 0);
}

static void test_leds(void) {
	leds_set(LED_BATTERY_0, 0);
	
	test_led_ir();
	
	leds_set(LED_RC, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_RC, 0);
	leds_set(LED_SOUND, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_SOUND,0);
	leds_set(LED_BATTERY_0, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_BATTERY_0, 0);
	leds_set(LED_BATTERY_1, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_BATTERY_1,0);
	leds_set(LED_BATTERY_2, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_BATTERY_2,0);
	
	test_led_circle();
	
	test_led_buttons();
	
	leds_set(LED_TEMP_RED, 32);
	wait_ms(WAIT_SHORT);
	leds_set(LED_TEMP_RED, 0);
	leds_set(LED_TEMP_BLUE, 32);
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_TEMP_BLUE,0);
	leds_set(LED_IR_BACK_L, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_IR_BACK_L, 0);
	leds_set(LED_SD_CARD, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_SD_CARD, 0);
	leds_set(LED_IR_BACK_R, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_IR_BACK_R, 0);
	leds_set(LED_GROUND_IR_0, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_GROUND_IR_0, 0);
	leds_set(LED_GROUND_IR_1, 32);
	
	wait_ms(WAIT_SHORT);
	
	leds_set(LED_GROUND_IR_1, 0);
	
	
	test_led_rgb();
}


void test_mode_start(void) {
	// DO all the things we can do syncronously
	test_leds();
	
	// sound is tested as the start ...
	
	
	leds_set(LED_R_TOP, 32);
	
	// Switch on all beacon leds
	leds_set(LED_FRONT_IR_0,32);
	leds_set(LED_FRONT_IR_1,32);
	leds_set(LED_FRONT_IR_2A, 32);
	leds_set(LED_FRONT_IR_3, 32);
	leds_set(LED_FRONT_IR_4, 32);
	leds_set(LED_IR_BACK_L, 32);
	leds_set(LED_IR_BACK_R, 32);
	
	leds_set(LED_SOUND, 32);
	leds_set(LED_TEMP_BLUE, 32);
	leds_set(LED_CIRCLE_0, 32),
	leds_set(LED_CIRCLE_1, 32);
	leds_set(LED_CIRCLE_2, 32);
	leds_set(LED_CIRCLE_3, 32);
	leds_set(LED_CIRCLE_4, 32);
	leds_set(LED_CIRCLE_5, 32);
	leds_set(LED_CIRCLE_6, 32);
	leds_set(LED_CIRCLE_7, 32);
	
	leds_set(LED_RC,32);
	leds_set(LED_GROUND_IR_0,32);
	leds_set(LED_GROUND_IR_1,32);
	leds_set(LED_R_BOT_L,32);
	
	
	behavior_start(B_TEST);
	sound_set_mic_stat(1);
}

static unsigned long checked;

#define C_NTC 	(1 << 0)
#define C_VIN 	(1 << 1)
#define C_IR0	(1 << 2)
#define C_IR1	(1 << 3)
#define C_IR2 	(1 << 4)
#define C_IR3 	(1 << 5)
#define C_IR4 	(1 << 6)
#define C_IR5 	(1 << 7)
#define C_IR6 	(1 << 8)
#define C_IRBL	(1 << 9)
#define C_IRBR 	(1 << 10)
#define C_RC5	(1 << 11)
#define C_MIC	(1 << 12)
#define C_BUT0	(1 << 13)
#define C_BUT1	(1 << 14)
#define C_BUT2	(1L << 15)
#define C_BUT3  (1L << 16)
#define C_BUT4  (1L << 17)
#define C_USB	(1L << 18)
#define C_ACC 	(1L << 19)
#define C_ALL	0x0FFFFFL

void test_mode_tick(void) {
	static unsigned int counter = 0;
	if(vmVariables.ntc > 200 && vmVariables.ntc < 400) {
		checked |= C_NTC;
		leds_set(LED_TEMP_BLUE,0);
	}
	
	unsigned int bat = vmVariables.vbat[0] + vmVariables.vbat[1];
	unsigned long temp = __builtin_mulss(bat,1000);
	bat = __builtin_divud(temp,3978);
	
	if(bat > 395 && bat < 430)
		checked |= C_VIN;
	
	if(vmVariables.prox[0] > 900 && vmVariables.prox[0] < 4000) {
		leds_set(LED_FRONT_IR_0,0);
		checked |= C_IR0;
	}
	if(vmVariables.prox[1] > 900 && vmVariables.prox[1] < 4000) {
		leds_set(LED_FRONT_IR_1,0);
		checked |= C_IR1;
	}
	if(vmVariables.prox[2] > 900 && vmVariables.prox[2] < 4000) {
		leds_set(LED_FRONT_IR_2A,0);
		checked |= C_IR2;
	}
	if(vmVariables.prox[3] > 900 && vmVariables.prox[3] < 4000) {
		leds_set(LED_FRONT_IR_3,0);
		checked |= C_IR3;
	}
	if(vmVariables.prox[4] > 900 && vmVariables.prox[4] < 4000) {
		leds_set(LED_FRONT_IR_4,0);
		checked |= C_IR4;
	}
	if(vmVariables.prox[5] > 900 && vmVariables.prox[5] < 4000) {
		leds_set(LED_IR_BACK_L,0);
		checked |= C_IR5;
	}
	if(vmVariables.prox[6] > 900 && vmVariables.prox[6] < 4000) {
		leds_set(LED_IR_BACK_R,0);
		checked |= C_IR6;		
	}
	if(vmVariables.ground_delta[0] > 100 && vmVariables.ground_delta[0]  < 900) {
		leds_set(LED_GROUND_IR_0,0);
		checked |= C_IRBL;
	}
	if(vmVariables.ground_delta[1] > 100 && vmVariables.ground_delta[1]  < 900) {
		leds_set(LED_GROUND_IR_1,0);
		checked |= C_IRBR;
	}
	if(vmVariables.rc5_command != 0 && vmVariables.rc5_address != 0) {
		leds_set(LED_RC,0);
		checked |= C_RC5;
	}
		
	unsigned int mic_2m = sound_mic_max + sound_mic_min;
	
	vmVariables.fwversion[0] = sound_mic_max;
	vmVariables.fwversion[1] = sound_mic_min;
	if(((sound_mic_max - sound_mic_min) > 40) && (mic_2m > 200) && (mic_2m < 300)) {
		checked |= C_MIC;
		leds_set(LED_SOUND,0);
	}
	
	// FIXME What do I do with button 0 & 1 !!
	checked |= C_BUT0;
	leds_set(LED_CIRCLE_4,0);
	checked |= C_BUT1;
	leds_set(LED_CIRCLE_6,0);
	
	if(vmVariables.buttons_state[2]) {
		leds_set(LED_CIRCLE_1,0);
		leds_set(LED_CIRCLE_3,0);
		leds_set(LED_CIRCLE_5,0);
		leds_set(LED_CIRCLE_7,0);
		checked |= C_BUT2;
	}
	if(vmVariables.buttons_state[3]) {
		leds_set(LED_CIRCLE_0,0);
		checked |= C_BUT3;
	}
	if(vmVariables.buttons_state[4]) {
		leds_set(LED_CIRCLE_2,0);
		checked |= C_BUT4;
	}

	if(usb_uart_configured())
		checked |= C_USB;
		
	if(abs(vmVariables.acc[0]) < 2 && abs(vmVariables.acc[1]) < 2 && vmVariables.acc[2] > 20) {
		leds_set(LED_R_BOT_L,0);
		checked |= C_ACC;
	}

		
	if(checked == C_ALL) {
		leds_set(LED_R_TOP, 0);
		leds_set(LED_G_TOP, 32);
	}
	
	counter++;
	
	if(counter < 200)
		vmVariables.target[0] = 200;
	if(counter < 400)
		vmVariables.target[0] = -200;
		
	if(counter < 600) {
		vmVariables.target[0] = 0;
		vmVariables.target[1] = 200;
	}
	if(counter < 800) 
		vmVariables.target[1] = -200;
		
	if(counter < 1200) {
		vmVariables.target[1]= 0;
	}
	if(counter == 1200) 
		counter = 0;
}

