#include <types/types.h>

#include "playback.h"
#include "sound.h"
#include "tone.h"
#include "sd.h"


static unsigned int time;
static const struct music * m;

// Last note is 0
struct music {
	unsigned int freq;
	unsigned int duration;
};

static const struct music m_poweron[] = 
	{
		{C5,4},
		{E5,3},
		{G5,4},
		{0,0},
	};
static const struct music m_poweroff[] = 
	{
		{E5,4},
		{G5,3},
		{C5,5},
		{0,0},
	};

static const struct music m_button[] =
	{
		{E5,4},
		{Bb4,3},
		{0,0},
	};


static int playback_buffer(unsigned char * b) {
	if(time-- == 0) {
		if(m) {
			m++;
			if(m->duration) {
				time = m->duration*6;
				tone_setup(m->freq);
			} else {
				time = 0;
				m = 0;
				return 0;
			}
		} else {
			time = 0;
			return 0;
		}
	}
	tone_fill_buffer(b,SOUND_OBUFSZ);
	return 1;
}

static void play_note(unsigned int freq, unsigned int duration) {
	m = 0;
	// Duration in ~ 0.1s.
	tone_setup(freq);
	time = duration*6;
	sound_playback_enable(playback_buffer);
}

static void play_music(const struct music * p) {
	m = p;
	tone_setup(m->freq);
	time = m->duration * 6;
	sound_playback_enable(playback_buffer);
}



void play_sound(int number) {
	char name[7] = {'p','0','.','r','a','w', 0};
	
	switch(number) {
	case 0 ... 9:
		name[1] += number;
		if(!sd_play_file(name)) {
			play_note(A4,10);
		}
		break;
	case 10 ... 19:
		name[1] += number;
		name[0] = 'r';
		sd_play_file(name);
		break;
	case 42:
		sd_play_file("tujhe.raw");
		break;
	case 31415:
		sd_play_file("koin.raw");
		break;
	case SOUND_DISABLE:
		sound_playback_disable();
		break;
	case SOUND_POWERON:
		if(!sd_play_file("poweron.raw")) {
			play_music(m_poweron);
		}
		break;
	case SOUND_POWEROFF:
		if(!sd_play_file("poweroff.raw")) {
			play_music(m_poweroff);
		}
		break;
	case SOUND_BUTTON:
		if(!sd_play_file("button.raw")) {
			play_music(m_button);
		}
		break;
	}
	
}	

void play_sound_block(int number) {
	play_sound(number);
	if(number == SOUND_DISABLE)
		return;
	while(!(time == 0xFFFF || time == 0x0000)) barrier();
}

