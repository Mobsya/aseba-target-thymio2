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

#ifndef _BEHAVIOR_H_
#define _BEHAVIOR_H_



void behavior_init(int prio);
void behavior_trigger(void);

// Access for read, will pulse continiously the led
#define BEHAVIOR_SD_READ 0x1
// Same for write
#define BEHAVIOR_SD_WRITE 0x2
// File-access based, will just blink it once.
#define BEHAVIOR_SD_FILE_ACCESS 0x4
#define BEHAVIOR_START 0x8000
#define BEHAVIOR_STOP 0x0000
void behavior_notify_sd(unsigned int rw);

void behavior_sound_mic(unsigned char level);

void behavior_start(int b);
void behavior_stop(int b);
int behavior_enabled(int b);

#define B_ALL 			0xFFFF
#define B_SOUND_BUTTON 		(1 << 0)
#define B_LEDS_BUTTON 		(1 << 1)
#define B_LEDS_PROX		(1 << 2)
#define B_LEDS_SD		(1 << 3)
#define B_LEDS_MIC		(1 << 4)
#define B_LEDS_BATTERY		(1 << 5)
#define B_LEDS_RC5		(1 << 6)
#define B_MOTORS		(1 << 7)
#define B_LEDS_ACC		(1 << 8)
#define B_LEDS_NTC		(1 << 9)
#define B_MODE			(1 << 10)
#define B_TEST			(1 << 11)
#define B_PAIRING		(1 << 12)
#define B_SETTING               (1 << 13)
#define B_MAX			B_SETTING

#define B_ALWAYS		(B_LEDS_BATTERY | B_LEDS_RC5 | B_LEDS_SD | B_SOUND_BUTTON | B_LEDS_BUTTON)

#endif
