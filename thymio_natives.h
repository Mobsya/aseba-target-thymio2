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

#ifndef _THYMIO_NATIVES_H_
#define _THYMIO_NATIVES_H_

extern AsebaNativeFunctionDescription AsebaNativeDescription_set_led;
void set_led(AsebaVMState *vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription_record;
void sound_record(AsebaVMState *vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription_play;
void sound_playback(AsebaVMState *vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription_replay;
void sound_replay(AsebaVMState *vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription_sound_system;
void sound_system(AsebaVMState *vm);

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

extern AsebaNativeFunctionDescription AsebaNativeDescription_set_hprox_leds;
void set_hprox_leds(AsebaVMState *vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription_set_vprox_leds;
void set_vprox_leds(AsebaVMState *vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription_set_rc_leds;
void set_rc_leds(AsebaVMState *vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription_set_sound_leds;
void set_sound_leds(AsebaVMState *vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription_set_ntc_leds;
void set_ntc_leds(AsebaVMState *vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription_set_wave;
void set_wave(AsebaVMState *vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription_prox_network;
void prox_network(AsebaVMState *vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription_sd_open;
void thymio_native_sd_open(AsebaVMState * vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription_sd_write;
void thymio_native_sd_write(AsebaVMState * vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription_sd_read;
void thymio_native_sd_read(AsebaVMState * vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription_sd_seek;
void thymio_native_sd_seek(AsebaVMState * vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription_rf_nodeid;
void set_rf_nodeid(AsebaVMState * vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription_duration;
void sound_duration(AsebaVMState *vm);

#define THYMIO_NATIVES_DESCRIPTIONS \
	&AsebaNativeDescription_set_led, \
	&AsebaNativeDescription_record, \
	&AsebaNativeDescription_play, \
	&AsebaNativeDescription_replay, \
	&AsebaNativeDescription_sound_system, \
	&AsebaNativeDescription_set_led_circle, \
	&AsebaNativeDescription_set_led_rgb_top, \
	&AsebaNativeDescription_set_led_rgb_bl, \
	&AsebaNativeDescription_set_led_rgb_br, \
	&AsebaNativeDescription_play_freq, \
	&AsebaNativeDescription_set_led_buttons, \
	&AsebaNativeDescription_set_hprox_leds, \
	&AsebaNativeDescription_set_vprox_leds, \
	&AsebaNativeDescription_set_rc_leds, \
	&AsebaNativeDescription_set_sound_leds, \
	&AsebaNativeDescription_set_ntc_leds, \
	&AsebaNativeDescription_set_wave, \
        &AsebaNativeDescription_prox_network, \
        &AsebaNativeDescription_sd_open, \
        &AsebaNativeDescription_sd_write, \
        &AsebaNativeDescription_sd_read, \
        &AsebaNativeDescription_sd_seek, \
	&AsebaNativeDescription_rf_nodeid, \
	&AsebaNativeDescription_duration
	
#define THYMIO_NATIVES_FUNCTIONS \
	set_led, \
	sound_record, \
	sound_playback, \
	sound_replay, \
	sound_system, \
	set_led_circle, \
	set_rgb_top, \
	set_rgb_bl, \
	set_rgb_br, \
	play_freq, \
	set_buttons_leds, \
	set_hprox_leds, \
	set_vprox_leds, \
	set_rc_leds, \
	set_sound_leds, \
	set_ntc_leds, \
	set_wave, \
        prox_network, \
        thymio_native_sd_open, \
        thymio_native_sd_write, \
        thymio_native_sd_read, \
        thymio_native_sd_seek, \
	set_rf_nodeid, \
	sound_duration


#endif

