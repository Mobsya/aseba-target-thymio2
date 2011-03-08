#include <p24fxxxx.h>

#include <types/types.h>
#include <error/error.h>

#include "sound.h"
#include "regulator.h"
#include "leds.h"
//#include "sound_recording.h"
#include "playback.h"

// Reduce to 8 bits sound ... 
static unsigned char input_buf[SOUND_IBUFSZ*2];
static unsigned char output_buf[SOUND_OBUFSZ*2];
static unsigned int obufp;
static unsigned int ibufp;
static sound_cb callback;

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
	_INT1IP = 1;
	_INT1IE = 1;
	
	// We use INT2 to trigger sound buffer writing
	_INT2IP = 1;
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

void sound_playback_disable(void) {
	callback = NULL;
	barrier();
	OC6R = 127;
}

void sound_poweroff(void) {
	va_put();
	
	// Powerdown the amplifier
	leds_set(SOUND_ON, 0);
	
	// Disable OC6
	OC6CON1 = 0;
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
