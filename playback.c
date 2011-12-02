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
#include <stdlib.h>

#include <types/types.h>

#include <skel-usb.h>

#include "playback.h"
#include "sound.h"
#include "tone.h"
#include "sd.h"


static int time;
static const struct music * m;
static const struct music * m_start;
static unsigned char l;
static unsigned char generate_events;

// Last note is 0
struct music {
	unsigned int freq;
	unsigned int duration;
};

static const struct music m_poweron[] = 
	{
	{3400,12},
	{4400,6},
	{5100,6},
	{6400,6},
	{7800,6},
	{0,0}
	};
static const struct music m_poweroff[] = 
	{
	{7800,12},
	{6400,6},
	{5100,6},
	{4400,6},
	{3400,6},
	{0,0}
	};

static const struct music m_button[] =
	{
		{1100,6},
		{2200,6},
		{0,0},
	};
static const struct music m_button_m[] = 
	{
		{3300,6},
		{4400,12},
		{6600,6},
		{0,0},
	};
	
static const struct music m_freefall[] = 
	{
		{20280/2,1},
		{17750/2,1},
		{14650/2,1},
		{12790/2,1},
		{25690/2,1},
		{23140/2,1},
		{20620/2,1},
		{19920/2,1},
		{17790/2,1},
		{12790/2,1},
		{26110/2,1},
		{16780/2,1},
		{24300/2,1},
		{23640/2,1},
		{22940/2,1},
		{29690/2,1},
		{0,0},
	};
	
static const struct music m_tap[] =
	{
		{6600, 4},
		{2750, 4},
		{0,0},
	};

static const struct music m_f_detect [] =
	{
		{4400,6},
		{5500,6},
		{0,0}
	};

static const struct music m_f_ok[] = 
	{
		{4400,6},
		{5500,6},
		{6600,6},
		{0,0}
	};


void playback_enable_event(void) {
	generate_events = 1;
}

void playback_notify_eop(void) {
	if(generate_events) 
		SET_EVENT(EVENT_SOUND_FINISHED);
}

static int playback_buffer(unsigned char * b) {
	
	if(time >= 0)
		if(time-- == 0) {
			if(m) {
				m++;
				if(m->duration) {
					time = m->duration;
					tone_setup(m->freq);
				} else if(!l){
					time = 0;
					m = 0;
					playback_notify_eop();
					return 0;
				} else {
					m = m_start;
					time = m->duration;
					tone_setup(m->freq);
				}
			} else {
				time = 0;
				playback_notify_eop();
				return 0;
			}
		}
	tone_fill_buffer(b,SOUND_OBUFSZ);
	return 1;
}

void play_note(unsigned int freq, unsigned int duration) {
	sound_playback_hold();
	
	m = 0;
	// Duration in ~ 0.1s.
	tone_setup(freq);
	if(duration == 0)
		time = -1;
	else
		time = duration;
	sound_playback_enable(playback_buffer);
}

static void play_music(const struct music * p, int loop) {
	sound_playback_hold();
	
	m_start = p;
	m = p;
	l = loop;
	tone_setup(m->freq);
	time = m->duration ;
	sound_playback_enable(playback_buffer);
}

void play_user_sound(char * name) {
	if(!name) {
		sound_playback_disable();
		return;
	}

	if(!sd_play_file(name, 0))
		play_note(A4, 10);
}

static int _play_sound(int number, int loop) {
	int ret = 0;	
	
	switch(number) {
	case SOUND_DISABLE:
		sound_playback_disable();
		break;
	case SOUND_POWERON:
		if(!sd_play_file("s0.wav",loop)) {
			play_music(m_poweron,loop);
		}
		break;
	case SOUND_POWEROFF:
		// Poweroff is a special case, we don't want to have it
		// user configurable as the user could mess with the poweroff
	//	if(!sd_play_file("s1.raw",loop)) {
			play_music(m_poweroff,loop);
	//	}
		break;
	case SOUND_BUTTON:
		if(!sd_play_file("s2.wav",loop)) {
			play_music(m_button,loop);
		}
		break;
	case SOUND_BUTTON_M:
		if(!sd_play_file("s3.wav",loop)) {
			play_music(m_button_m, loop);
		}
		break;	
	case SOUND_FREEFALL:
		if(!sd_play_file("s4.wav",loop)) {
			play_music(m_freefall, loop);
		}
		break;
	case SOUND_TAP:
		if(!sd_play_file("s5.wav",loop)) {
			play_music(m_tap,loop);
		}
		break;
	case SOUND_F_DETECT:
		if(!sd_play_file("s6.wav",loop)) {
			play_music(m_f_detect, loop);
		}
		break;
	case SOUND_F_OK:
		if(!sd_play_file("s7.wav",loop)) {
			play_music(m_f_ok, loop);
		}
		break;
	default:
		ret = -1;
	}
	return ret;
	
}	

int play_sound(int number) {
	return _play_sound(number, 0);
}

void play_sound_loop(int number) {
	_play_sound(number, 1);	
}

void play_sound_block(int number) {
	_play_sound(number,0);
	if(number == SOUND_DISABLE)
		return;
	while(m) barrier();
}

void play_frequency(int freq, int time) {
	if(time < 0) {
		sound_playback_disable();
	} else {
		if(time > 0 && time < 4)
			time = 4;
		play_note(((unsigned int) abs(freq)) * 10, time);
	}
}
