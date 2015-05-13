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

#ifndef _LEDS_H_
#define _LEDS_H_

void leds_init(void);

void leds_poweroff(void);

void leds_tick_cb(void);

void leds_set(unsigned char l, unsigned char brightness);

void leds_clear_all(void);

void leds_set_circle(unsigned char l1, unsigned char l2, unsigned char l3, unsigned char l4, unsigned char l5, unsigned char l6, unsigned char l7, unsigned char l8);

void leds_set_top(unsigned char r, unsigned char g, unsigned char b);
void leds_set_br(unsigned char r, unsigned char g, unsigned char b);
void leds_set_bl(unsigned char r, unsigned char g, unsigned char b);
void leds_set_body_rgb(unsigned int r, unsigned int g, unsigned int b);

void leds_set_prox_h(unsigned char l1, unsigned char l2, unsigned char l3, unsigned char l4, unsigned char l5, unsigned char l6, unsigned char l7, unsigned char l8);

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
	SOUND_ON,		/* 15 */
	
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
