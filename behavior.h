#ifndef _BEHAVIOR_H_
#define _BEHAVIOR_H_



void behavior_init(int timer, int prio);

void behavior_start(int b);
void behavior_stop(int b);


#define B_ALL 			0xFFFF
#define B_SOUND_BUTTON 	(1 << 0)
#define B_LEDS_BUTTON 	(1 << 1)
#define B_LEDS_PROX		(1 << 2)
#define B_LEDS_BODY		(1 << 3)
#define B_LEDS_SD		(1 << 4)
#define B_LEDS_MIC		(1 << 5)
#define B_LEDS_BATTERY	(1 << 6)
#define B_LEDS_RC5		(1 << 7)
#define B_MOTORS		(1 << 8)
#define B_STATE_MACHINE	(1 << 9)

#endif
