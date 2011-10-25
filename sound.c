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

#include <p24fxxxx.h>

#include <types/types.h>
#include <error/error.h>

#include "sound.h"
#include "regulator.h"
#include "leds.h"
//#include "sound_recording.h"
#include "playback.h"
#include "behavior.h"

// Reduce to 8 bits sound ... 
static unsigned char input_buf[SOUND_IBUFSZ*2];
static unsigned char output_buf[SOUND_OBUFSZ*2];
static unsigned int obufp;
static unsigned int ibufp;
static sound_cb callback;
static int sound_mic_stat;

unsigned char sound_mic_max;
unsigned char sound_mic_min;

void sound_init(void) {
	int i;
	// Init the output compare
	va_get(); // used by the mic
	
	obufp = 0;
	ibufp = 0;
	
	// init output buffer to middle level
	for(i = 0; i < SOUND_OBUFSZ*2; i ++)
		output_buf[i] = 128;
	
	// Enable the amplifier the amplifier
	leds_set(SOUND_ON, 32);
	
	// OC6 is used ...
	// Init OC6 to have maximum PWM speed 8 bit resolution
	OC6CON1bits.OCTSEL = 0x7; // Fcy clock source
	OC6CON2bits.SYNCSEL = 0x1f; // period register is OC6RS
	OC6R = 128;
	OC6RS = 255;
	OC6CON1bits.OCM = 6; // edge PWM mode
	callback = NULL;
	
	
	// We use INT1 to trigger sound buffer filling (read)
	_INT1IP = 2;
	_INT1IE = 1;
	
	// We use INT2 to trigger sound buffer writing
	_INT2IP = 2;
	_INT2IE = 1;
	
}

void sound_playback_enable(sound_cb cb) {
	callback = NULL;
	barrier();
	
	leds_set(SOUND_ON, 32);
	// Prefill the buffers
	cb(&output_buf[0]);
	cb(&output_buf[SOUND_OBUFSZ]);
	obufp = 0;
	
	barrier();
	
	callback = cb;
}

void sound_playback_hold(void){
	// If we interrupt an SD read we need to
	// tell the behavior that SD read is over.
	behavior_notify_sd(BEHAVIOR_STOP | BEHAVIOR_SD_READ);
	
	callback = NULL;
	barrier();
};

void sound_playback_disable(void) {
	sound_playback_hold();
	
	leds_set(SOUND_ON, 0);
	OC6R = 127;
}

void sound_poweroff(void) {
	va_put();
	
	// Powerdown the amplifier
	leds_set(SOUND_ON, 0);
	
	// Disable OC6
	OC6CON1 = 0;
}	

void sound_set_mic_stat(int enabled) {
	sound_mic_stat = enabled;
}

void _ISR _INT1Interrupt(void) {
	int ret;
	_INT1IF = 0;
	
	if(obufp >= SOUND_OBUFSZ) 
		ret = callback ? callback(&output_buf[0]) : 0;
	else
		ret = callback ? callback(&output_buf[SOUND_OBUFSZ]) : 0;
	
	if(!ret)
		sound_playback_disable();
}

void _ISR _INT2Interrupt(void) {
	_INT2IF = 0;
	
	if(sound_mic_stat) {
		unsigned char *ptr;
		if(ibufp >= SOUND_IBUFSZ)
			ptr = input_buf;
		else
			ptr = &input_buf[SOUND_IBUFSZ];
		
		
		unsigned char max = 0;
		unsigned char min = 255;
		int i;
		for(i = 0; i < SOUND_IBUFSZ; i++) {
			if(*ptr > max)
				max = *ptr;
			if(*ptr < min)
				min = *ptr;
			ptr++;
		}
		sound_mic_max = max;
		sound_mic_min = min;
	}
	
	if(ibufp >= SOUND_IBUFSZ)
		sound_mic_buffer(&input_buf[0]);
	else
		sound_mic_buffer(&input_buf[SOUND_IBUFSZ]);
}
	

void sound_new_sample(unsigned int sample) {
	// Output new sample ...
	if(callback) { 
		OC6R = output_buf[obufp++];
		if(obufp == SOUND_OBUFSZ)
			_INT1IF = 1;
			
		if(obufp == SOUND_OBUFSZ * 2) {
			_INT1IF = 1;
			obufp = 0;
		}
	}
	
	// Input
	input_buf[ibufp++] = sample >> 2;
	if (ibufp == SOUND_IBUFSZ) 
		_INT2IF = 1;
		
	if (ibufp == SOUND_IBUFSZ * 2) {
		_INT2IF = 1;
		ibufp = 0;
	}
}
