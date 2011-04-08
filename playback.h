#ifndef _PLAYBACK_H_
#define _PLAYBACK_H_

#define SOUND_DISABLE	(-1) /* Special case: immediatly stop playback */

#define SOUND_POWERON 	(-2)
#define SOUND_POWEROFF 	(-3)
#define SOUND_BUTTON	(-4)
#define SOUND_FREEFALL 	(-5)
#define SOUND_TAP		(-6)
#define SOUND_BUTTON_M	(-7)
#define SOUND_F_DETECT	(-8)
#define SOUND_F_OK		(-9)


// Negative: system sound
// Positive: user sound
void play_sound(int number);

void play_sound_loop(int number);

void play_sound_block(int number);

void play_frequency_block(int freq, int time);

#endif
