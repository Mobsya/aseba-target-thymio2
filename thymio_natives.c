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

#include <types/types.h>

#include <skel-usb.h>
#include "leds.h"
#include "sd.h"
#include "playback.h"
#include "behavior.h"
#include "tone.h"

AsebaNativeFunctionDescription AsebaNativeDescription_set_led = {
	"_leds.set",
	"Set the led",
	{
		{1,"led"},
		{1,"brightness"},
		{0,0}
	}
};

void set_led(AsebaVMState *vm) {
	int led = vm->variables[AsebaNativePopArg(vm)];
	int b = vm->variables[AsebaNativePopArg(vm)];
	
	if(led == SOUND_ON || led < 0 || led > 39) 
		return;
	
	leds_set(led,b);
}

AsebaNativeFunctionDescription AsebaNativeDescription_record = {
	"sound.record",
	"Start recording of rN.wav",
	{
		{1,"N"},
		{0,0},
	}
};

AsebaNativeFunctionDescription AsebaNativeDescription_play = {
	"sound.play",
	"Start playback of pN.wav",
	{
		{1,"N"},
		{0,0},
	}
};


AsebaNativeFunctionDescription AsebaNativeDescription_replay = {
	"sound.replay",
	"Start playback of rN.wav",
	{
		{1,"N"},
		{0,0},
	}
};

AsebaNativeFunctionDescription AsebaNativeDescription_sound_system = {
	"sound.system",
	"Start playback of system sound N",
	{
		{1,"N"},
		{0,0},
	}
};


static void prepare_name(unsigned int n, char * buf) {
	unsigned int div;
	unsigned int z = 0;
	for(div=10000; div > 0; div /= 10) {
		unsigned int disp = n / div;
		n %=div;
		if((disp != 0) || (z) || (div == 1)) {
			z = 1;
			*buf++ = '0' + disp;
		}
	}
	
	*buf++ = '.';
	*buf++ = 'w';
	*buf++ = 'a';
	*buf++ = 'v';
	*buf++ = 0;
}

void sound_playback(AsebaVMState *vm) {
	char name[13] = {'p'};
	int number = vm->variables[AsebaNativePopArg(vm)];
	
	behavior_stop(B_SOUND_BUTTON);
	playback_enable_event();
	
	if(number == -1) 
		play_user_sound(NULL);
	else {
		prepare_name(number, &name[1]);
		play_user_sound(name);
	}
}

void sound_record(AsebaVMState *vm) {
	char name[13] = {'r'};
	int number = vm->variables[AsebaNativePopArg(vm)];
	if(number == -1) {
		sd_stop_record();
		return;
	}
	behavior_stop(B_SOUND_BUTTON);
	
	prepare_name(number, &name[1]);
	sd_start_record(name);
}

void sound_replay(AsebaVMState *vm) {
	char name[13] = {'r'};
	int number = vm->variables[AsebaNativePopArg(vm)];
	
	behavior_stop(B_SOUND_BUTTON);
	playback_enable_event();
	
	if(number == -1) 
		play_user_sound(NULL);
	else {
		prepare_name(number, &name[1]);
		play_user_sound(name);
	}
}

void sound_system(AsebaVMState *vm) {
	char name[13] = {'s'};
	int number = vm->variables[AsebaNativePopArg(vm)];
	
	behavior_stop(B_SOUND_BUTTON);
	playback_enable_event();
	
	if(play_sound(number)) {
		prepare_name(number,&name[1]);
		play_user_sound(name);
	}
}

AsebaNativeFunctionDescription AsebaNativeDescription_set_led_circle = {
	"leds.circle",
	"Set circular ring leds",
	{
		{1,"led 0"},
		{1,"led 1"},
		{1,"led 2"},
		{1,"led 3"},
		{1,"led 4"},
		{1,"led 5"},
		{1,"led 6"},
		{1,"led 7"},
		{0,0},
	}
};

void set_led_circle(AsebaVMState *vm) {
	int l1 = vm->variables[AsebaNativePopArg(vm)];
	int l2 = vm->variables[AsebaNativePopArg(vm)];
	int l3 = vm->variables[AsebaNativePopArg(vm)];
	int l4 = vm->variables[AsebaNativePopArg(vm)];
	int l5 = vm->variables[AsebaNativePopArg(vm)];
	int l6 = vm->variables[AsebaNativePopArg(vm)];
	int l7 = vm->variables[AsebaNativePopArg(vm)];
	int l8 = vm->variables[AsebaNativePopArg(vm)];
	
	behavior_stop(B_LEDS_ACC);

	leds_set_circle(l1,l2,l3,l4,l5,l6,l7,l8);
}

AsebaNativeFunctionDescription AsebaNativeDescription_set_led_rgb_top = {
	"leds.top",
	"Set RGB top led",
	{
		{1,"red"},
		{1,"green"},
		{1,"blue"},
		{0,0},
	}
};

void set_rgb_top(AsebaVMState *vm) {
	int r = vm->variables[AsebaNativePopArg(vm)];
	int g = vm->variables[AsebaNativePopArg(vm)];
	int b = vm->variables[AsebaNativePopArg(vm)];
	
	leds_set_top(r,g,b);
}

AsebaNativeFunctionDescription AsebaNativeDescription_set_led_rgb_br = {
	"leds.bottom.right",
	"Set RGB botom right led",
	{
		{1,"red"},
		{1,"green"},
		{1,"blue"},
		{0,0},
	}
};

void set_rgb_br(AsebaVMState *vm) {
	int r = vm->variables[AsebaNativePopArg(vm)];
	int g = vm->variables[AsebaNativePopArg(vm)];
	int b = vm->variables[AsebaNativePopArg(vm)];
	
	leds_set_br(r,g,b);
}
AsebaNativeFunctionDescription AsebaNativeDescription_set_led_rgb_bl = {
	"leds.bottom.left",
	"Set RGB botom left led",
	{
		{1,"red"},
		{1,"green"},
		{1,"blue"},
		{0,0},
	}
};

void set_rgb_bl(AsebaVMState *vm) {
	int r = vm->variables[AsebaNativePopArg(vm)];
	int g = vm->variables[AsebaNativePopArg(vm)];
	int b = vm->variables[AsebaNativePopArg(vm)];
		
	leds_set_bl(r,g,b);
}

AsebaNativeFunctionDescription AsebaNativeDescription_set_led_buttons = {
	"leds.buttons",
	"Set buttons leds",
	{
		{1,"led 0"},
		{1,"led 1"},
		{1,"led 2"},
		{1,"led 3"},
		{0,0},
	}
};

void set_buttons_leds(AsebaVMState *vm) {
	int l1 = vm->variables[AsebaNativePopArg(vm)];
	int l2 = vm->variables[AsebaNativePopArg(vm)];
	int l3 = vm->variables[AsebaNativePopArg(vm)];
	int l4 = vm->variables[AsebaNativePopArg(vm)];
		
	behavior_stop(B_LEDS_BUTTON);	
	
	leds_set(LED_BUTTON_0, l1);
	leds_set(LED_BUTTON_1, l2);
	leds_set(LED_BUTTON_2, l3);
	leds_set(LED_BUTTON_3, l4);
}

AsebaNativeFunctionDescription AsebaNativeDescription_set_hprox_leds = {
	"leds.prox.h",
	"Set horizontal proximity leds",
	{
		{1,"led 0"},
		{1,"led 1"},
		{1,"led 2"},
		{1,"led 3"},
		{1,"led 4"},
		{1,"led 5"},
		{1,"led 6"},
		{1,"led 7"},
		{0,0},
	}
};

void set_hprox_leds(AsebaVMState *vm) {
	int l1 = vm->variables[AsebaNativePopArg(vm)];
	int l2 = vm->variables[AsebaNativePopArg(vm)];
	int l3 = vm->variables[AsebaNativePopArg(vm)];
	int l4 = vm->variables[AsebaNativePopArg(vm)];
	int l5 = vm->variables[AsebaNativePopArg(vm)];
	int l6 = vm->variables[AsebaNativePopArg(vm)];
	int l7 = vm->variables[AsebaNativePopArg(vm)];
	int l8 = vm->variables[AsebaNativePopArg(vm)];

	behavior_stop(B_LEDS_PROX);
	
	leds_set(LED_FRONT_IR_0, l1);
	leds_set(LED_FRONT_IR_1, l2);
	leds_set(LED_FRONT_IR_2A,l3);
	leds_set(LED_FRONT_IR_2B,l4);
	leds_set(LED_FRONT_IR_3, l5);
	leds_set(LED_FRONT_IR_4, l6);
	leds_set(LED_IR_BACK_L,  l7);
	leds_set(LED_IR_BACK_R,  l8);
	
}

AsebaNativeFunctionDescription AsebaNativeDescription_set_vprox_leds = {
	"leds.prox.v",
	"Set vertical proximity leds",
	{
		{1,"led 0"},
		{1,"led 1"},
		{0,0},
	}
};

void set_vprox_leds(AsebaVMState *vm) {
	int l1 = vm->variables[AsebaNativePopArg(vm)];
	int l2 = vm->variables[AsebaNativePopArg(vm)];
	
	behavior_stop(B_LEDS_PROX);
	
	leds_set(LED_GROUND_IR_0,l1);
	leds_set(LED_GROUND_IR_1,l2);
}


AsebaNativeFunctionDescription AsebaNativeDescription_set_rc_leds = {
	"leds.rc",
	"Set rc led",
	{
		{1,"led"},
		{0,0},
	}
};

void set_rc_leds(AsebaVMState *vm) {
	int l1 = vm->variables[AsebaNativePopArg(vm)];
	
	behavior_stop(B_LEDS_RC5);
	
	leds_set(LED_RC,l1);
}


AsebaNativeFunctionDescription AsebaNativeDescription_set_sound_leds = {
	"leds.sound",
	"Set sound led",
	{
		{1,"led"},
		{0,0},
	}
};


void set_sound_leds(AsebaVMState *vm) {
	int l1 = vm->variables[AsebaNativePopArg(vm)];
	
	behavior_stop(B_LEDS_MIC);
	
	leds_set(LED_SOUND,l1);
}	


AsebaNativeFunctionDescription AsebaNativeDescription_set_ntc_leds = {
	"leds.temperature",
	"Set ntc led",
	{
		{1,"red"},
		{1,"blue"},
		{0,0},
	}
};


void set_ntc_leds(AsebaVMState *vm) {
	int l1 = vm->variables[AsebaNativePopArg(vm)];
	int l2 = vm->variables[AsebaNativePopArg(vm)];
	
	behavior_stop(B_LEDS_NTC);
	
	leds_set(LED_TEMP_RED,l1);
	leds_set(LED_TEMP_BLUE,l2);
}	




AsebaNativeFunctionDescription AsebaNativeDescription_play_freq = {
	"sound.freq",
	"Play frequency",
	{
		{1,"Hz"},
		{1,"ds"},
		{0,0},
	}
};

void play_freq(AsebaVMState * vm) {
	int freq = vm->variables[AsebaNativePopArg(vm)];
	int time = vm->variables[AsebaNativePopArg(vm)];
	int flags;
	
	// In order to not race wrt behavior producing sound
	// I need to rise my IPL to behavior IPL
	RAISE_IPL(flags, 1);
	
	play_frequency(freq, time);
	
	IRQ_ENABLE(flags);
}

AsebaNativeFunctionDescription AsebaNativeDescription_set_wave = {
	"sound.wave",
	"Set the primary wave of the tone generator",
	{
		{WAVEFORM_SIZE, "wave"},
		{0,0},
	}
};

void set_wave(AsebaVMState * vm) {
	int *wave = (int *) vm->variables + AsebaNativePopArg(vm);
	
	int i;
	for(i = 0; i < WAVEFORM_SIZE; i++) {
		if(wave[i] > 127 || wave[i] < -128) {
			AsebaVMEmitNodeSpecificError(vm, "Error: samples must be in [-128,127] range");
			return;
		}
	}
	
	tone_set_waveform(wave);
}
