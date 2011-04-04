#include <types/types.h>

#include "playback.h"
#include "sound.h"
#include "tone.h"
#include "sd.h"


static int time;
static const struct music * m;
static const struct music * m_start;
static unsigned char l;

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
static const struct music m_freefall[] = 
	{
		{A4, 1},
		{A6, 1},
		{0,0},
	};
	
static const struct music m_tap[] =
	{
		{A4, 1},
		{Bb5, 1},
		{A5, 1},
		{0,0},
	};


static int playback_buffer(unsigned char * b) {
	
	if(time >= 0)
		if(time-- == 0) {
			if(m) {
				m++;
				if(m->duration) {
					time = m->duration*6;
					tone_setup(m->freq);
				} else if(!l){
					time = 0;
					m = 0;
					return 0;
				} else {
					m = m_start;
					time = m->duration*6;
					tone_setup(m->freq);
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
	if(duration == 0)
		time = -1;
	else
		time = duration*6;
	sound_playback_enable(playback_buffer);
}

static void play_music(const struct music * p, int loop) {
	m_start = p;
	m = p;
	l = loop;
	tone_setup(m->freq);
	time = m->duration * 6;
	sound_playback_enable(playback_buffer);
}



static void _play_sound(int number, int loop) {
	char name[7] = {'p','0','.','r','a','w', 0};
	
	switch(number) {
	case 0 ... 9:
		name[1] += number;
		if(!sd_play_file(name,loop)) {
			if(loop) 
				play_note(A4,0);
			else
				play_note(A4, 10);	
		}
		break;
	case 10 ... 19:
		name[1] += number;
		name[0] = 'r';
		sd_play_file(name,loop);
		break;
	case 42:
		sd_play_file("tujhe.raw",loop);
		break;
	case 31415:
		sd_play_file("koin.raw",loop);
		break;
	case SOUND_DISABLE:
		sound_playback_disable();
		break;
	case SOUND_POWERON:
		if(!sd_play_file("poweron.raw",loop)) {
			play_music(m_poweron,loop);
		}
		break;
	case SOUND_POWEROFF:
		if(!sd_play_file("poweroff.raw",loop)) {
			play_music(m_poweroff,loop);
		}
		break;
	case SOUND_BUTTON:
		if(!sd_play_file("button.raw",loop)) {
			play_music(m_button,loop);
		}
		break;
	case SOUND_FREEFALL:
		if(!sd_play_file("freefall.raw",loop)) {
			play_music(m_freefall, loop);
		}
		break;
	case SOUND_TAP:
		if(!sd_play_file("tap.raw",loop)) {
			play_music(m_tap,loop);
		}
		break;
	}
	
}	

void play_sound(int number) {
	_play_sound(number, 0);
}

void play_sound_loop(int number) {
	_play_sound(number, 1);	
}

void play_sound_block(int number) {
	_play_sound(number,0);
	if(number == SOUND_DISABLE)
		return;
	while(!(time == 0xFFFF || time == 0x0000)) barrier();
}

void play_frequency_block(int freq, int time) {
	struct music temp[] = {{
		freq, time
	},
		{0,0}
	};
	play_music(temp,0);
	
	while(m != NULL) barrier();
}

