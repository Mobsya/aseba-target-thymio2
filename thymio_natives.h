#ifndef _THYMIO_NATIVES_H_
#define _THYMIO_NATIVES_H_

extern AsebaNativeFunctionDescription AsebaNativeDescription_set_led;
void set_led(AsebaVMState *vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription_record;
void sound_record(AsebaVMState *vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription_play;
void sound_playback(AsebaVMState *vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription_set_led_circle;
void set_led_circle(AsebaVMState *vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription_set_led_rgb_top;
void set_rgb_top(AsebaVMState *vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription_set_led_rgb_bl;
void set_rgb_bl(AsebaVMState *vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription_set_led_rgb_br;
void set_rgb_br(AsebaVMState *vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription_play_freq;
void play_freq(AsebaVMState * vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription_set_led_buttons;
void set_buttons_leds(AsebaVMState *vm);

#define THYMIO_NATIVES_DESCRIPTIONS \
	&AsebaNativeDescription_set_led, \
	&AsebaNativeDescription_record, \
	&AsebaNativeDescription_play, \
	&AsebaNativeDescription_set_led_circle, \
	&AsebaNativeDescription_set_led_rgb_top, \
	&AsebaNativeDescription_set_led_rgb_bl, \
	&AsebaNativeDescription_set_led_rgb_br, \
	&AsebaNativeDescription_play_freq, \
	&AsebaNativeDescription_set_led_buttons
	
#define THYMIO_NATIVES_FUNCTIONS \
	set_led, \
	sound_record, \
	sound_playback, \
	set_led_circle, \
	set_rgb_top, \
	set_rgb_bl, \
	set_rgb_br, \
	play_freq, \
	set_buttons_leds


#endif

