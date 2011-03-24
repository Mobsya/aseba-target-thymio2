#ifndef _BEHAVIOR_H_
#define _BEHAVIOR_H_



void behavior_init(int timer, int prio);

#define BEHAVIOR_SD_READ 0x1
#define BEHAVIOR_SD_WRITE 0x2
#define BEHAVIOR_START 0x8000
#define BEHAVIOR_STOP 0x0000
void behavior_notify_sd(unsigned int rw);

void behavior_start(int b);
void behavior_stop(int b);
int behavior_enabled(int b);

#define B_ALL 			0xFFFF
#define B_SOUND_BUTTON 	(1 << 0)
#define B_LEDS_BUTTON 	(1 << 1)
#define B_LEDS_PROX		(1 << 2)
#define B_LEDS_SD		(1 << 3)
#define B_LEDS_MIC		(1 << 4)
#define B_LEDS_BATTERY	(1 << 5)
#define B_LEDS_RC5		(1 << 6)
#define B_MOTORS		(1 << 7)
#define B_LEDS_ACC		(1 << 8)
#define B_LEDS_NTC		(1 << 9)
#define B_MODE			(1 << 10)
#define B_TEST			(1 << 11)
#define B_MAX			B_TEST

#define B_ALWAYS		(B_LEDS_BATTERY | B_LEDS_RC5 | B_LEDS_SD)

#endif
