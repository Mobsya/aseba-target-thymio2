#ifndef _LEDS_H_
#define _LEDS_H_

void leds_init(void);

void leds_poweroff(void);

void leds_tick_cb(void);

void leds_set(unsigned char l, unsigned char brightness);

enum leds {
	LED_IR_BACK_L,		/* 0 */
	LED_IR_BACK_R,
	LED_R_TOP,
	LED_G_TOP,
	LED_B_TOP,
	LED_BATTERY_0,
	LED_BATTERY_1,
	LED_BATTERY_2,		/* 7 */

	LED_R_BOT_L, 		/* 8 */
	LED_G_BOT_L	,
	LED_B_BOT_L	,
	LED_R_BOT_R	,
	LED_G_BOT_R	,
	LED_B_BOT_R	,
	LED_SD_CARD	,
	SOUND_ON,			/* 15 */
	
	LED_FRONT_IR_0,		/* 16 */
	LED_FRONT_IR_1,
	LED_FRONT_IR_3,
	LED_FRONT_IR_4,
	LED_FRONT_IR_2A	,
	LED_FRONT_IR_2B,
	LED_GROUND_IR_0,
	LED_GROUND_IR_1,	/* 23 */
	
	LED_CIRCLE_0,		/* 24 */
	LED_CIRCLE_1,
	LED_CIRCLE_2,
	LED_CIRCLE_3,
	LED_CIRCLE_4,
	LED_RC,
	LED_SOUND,
	LED_CIRCLE_7,		/* 31 */
	
	LED_BUTTON_0,		/* 32 */
	LED_BUTTON_2,
	LED_BUTTON_3,
	LED_BUTTON_1,
	LED_CIRCLE_5,
	LED_CIRCLE_6,
	LED_TEMP_RED,
	LED_TEMP_BLUE,		/* 39 */
};	


#endif
