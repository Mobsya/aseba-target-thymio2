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

#include <p24Fxxxx.h>

#include <types/types.h>
#include <error/error.h>

#include <skel-usb.h>

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

	if(settings.sound_shift < 8)	
		leds_set(SOUND_ON, 32);
	else
		leds_set(SOUND_ON, 0);

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
	OC6R = 127 >> settings.sound_shift;
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

void static compute_stats(unsigned char * buf) {
// sampling freq: 7.8Khz.
// Number of sample: 128
// Cutoff freq. approx 60Hz
// Everything below 60Hz won't be seen by thoses stats.
	
	unsigned int max = 0;
	unsigned int min = 255;
	int i;
	for(i = 0; i < SOUND_IBUFSZ; i++) {
		if(*buf > max)
			max = *buf;
		if(*buf < min)
			min = *buf;
		buf++;
	}
	
	vmVariables.sound_level = max - min;
	vmVariables.sound_mean = max + min;

	behavior_sound_mic(vmVariables.sound_level);

	if(vmVariables.sound_tresh && vmVariables.sound_level > vmVariables.sound_tresh)
		SET_EVENT(EVENT_MIC);
}	

void _ISR _INT2Interrupt(void) {
	_INT2IF = 0;

	if(ibufp >= SOUND_IBUFSZ) {
		compute_stats(&input_buf[0]);
		sound_mic_buffer(&input_buf[0]);
	} else {
		compute_stats(&input_buf[SOUND_IBUFSZ]);
		sound_mic_buffer(&input_buf[SOUND_IBUFSZ]);
	}
}

// Do not change this, as it is called from
// an interrupt with only w0 saved.
static unsigned char __attribute((near)) mic_ign;
void __attribute((naked)) sound_ignore_next_sample(void) {
	asm ("mov #2, w0		\n"
		 "mov.b WREG,%[mic]	\n"
		 : [mic] "=T" (mic_ign)::"w0");
}	

void sound_new_sample(unsigned int sample) {
	static unsigned int last_sample;
	// Output new sample ...
	if(callback) { 
// The shift changes the DC value of the signal, but we have a
// High-pass filter before the power amp so it is not important
		OC6R = output_buf[obufp++] >> settings.sound_shift;
		if(obufp == SOUND_OBUFSZ)
			_INT1IF = 1;
			
		if(obufp == SOUND_OBUFSZ * 2) {
			_INT1IF = 1;
			obufp = 0;
		}
	}
	// I don't want that the compiler optimize
	// Between input and output
	barrier();
	
	// Input
	if(mic_ign == 1) {
		input_buf[ibufp++] = last_sample >> 2;
	} else {
		input_buf[ibufp++] = sample >> 2;
		last_sample = sample;
	}
	
	if(mic_ign)
		mic_ign--;
	
	if (ibufp == SOUND_IBUFSZ) 
		_INT2IF = 1;
		
	if (ibufp == SOUND_IBUFSZ * 2) {
		_INT2IF = 1;
		ibufp = 0;
	}
}
