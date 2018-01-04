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
#include "ir_prox.h"
#include "rf.h"

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

AsebaNativeFunctionDescription AsebaNativeDescription_duration = {
	"sound.duration",
	"Give duration in 1/10s of rN.wav",
	{
		{1,"N"},
		{1,"duration"},		
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

static char * _prepare_name(unsigned int n, char * buf) {
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
	return buf;
}

static void prepare_name(unsigned int n, char * buf) {
	buf = _prepare_name(n, buf);
	
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

void sound_duration(AsebaVMState *vm) {
	char name[13] = {'r'};
	int number = vm->variables[AsebaNativePopArg(vm)];
	unsigned int durationIndex = AsebaNativePopArg(vm);
	
	prepare_name(number, &name[1]);	
	vm->variables[durationIndex] = sd_read_duration(name);
	
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
		{1,"l0"},
		{1,"l1"},
		{1,"l2"},
		{1,"l3"},
		{1,"l4"},
		{1,"l5"},
		{1,"l6"},
		{1,"l7"},
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
		{1,"l0"},
		{1,"l1"},
		{1,"l2"},
		{1,"l3"},
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
		{1,"l0"},
		{1,"l1"},
		{1,"l2"},
		{1,"l3"},
		{1,"l4"},
		{1,"l5"},
		{1,"l6"},
		{1,"l7"},
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
	
	leds_set_prox_h(l1,l2,l3,l4,l5,l6,l7,l8);
	
}

AsebaNativeFunctionDescription AsebaNativeDescription_set_vprox_leds = {
	"leds.prox.v",
	"Set vertical proximity leds",
	{
		{1,"l0"},
		{1,"l1"},
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
	
	behavior_stop(B_SOUND_BUTTON);
	
	playback_enable_event();

	play_frequency(freq, time);
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

AsebaNativeFunctionDescription AsebaNativeDescription_prox_network = {
	"prox.comm.enable",
	"Enable or disable the proximity communication",
	{
		{1, "state"},
		{0,0},
	}
};
		
void prox_network(AsebaVMState * vm) {
	int enable = vm->variables[AsebaNativePopArg(vm)];
	if(enable)
		prox_enable_network();
	else
		prox_disable_network();
}

AsebaNativeFunctionDescription AsebaNativeDescription_sd_open = {
	"sd.open",
	"Open a file on the SD card",
	{
		{1, "number"},
		{1, "status"},
		{0,0},
	}
};

void thymio_native_sd_open(AsebaVMState * vm) {
	int no = vm->variables[AsebaNativePopArg(vm)];
	unsigned int status = AsebaNativePopArg(vm);
	char name[13] = {'u'};
	char * p;


	if(no == -1) {
		sd_user_open(NULL);
		vm->variables[status] = 0;
	} else {
		p = _prepare_name(no, &name[1]);
		*p++ = '.';
		*p++ = 'd';
		*p++ = 'a';
		*p++ = 't';
		*p++ = 0;
		vm->variables[status] = sd_user_open(name);
	}
}

AsebaNativeFunctionDescription AsebaNativeDescription_sd_write = {
	"sd.write",
	"Write data to the opened file",
	{
		{-1, "data"},
		{1, "written"},
		{0,0},
	}
};


void thymio_native_sd_write(AsebaVMState * vm) {
	// variable pos
	unsigned char * data = (unsigned char *) (vm->variables + AsebaNativePopArg(vm));
	uint16_t status = AsebaNativePopArg(vm);

	// variable size
	uint16_t length = AsebaNativePopArg(vm) * 2;

	vm->variables[status] = sd_user_write(data,length) / 2;
}

AsebaNativeFunctionDescription AsebaNativeDescription_sd_read = {
	"sd.read",
	"Read data from the opened file",
	{
		{-1, "data"},
		{1, "read"},
		{0,0},
	}
};

void thymio_native_sd_read(AsebaVMState * vm) {
	// variable pos
	unsigned char * data = (unsigned char *) (vm->variables + AsebaNativePopArg(vm));
	uint16_t status = AsebaNativePopArg(vm);

	// variable size
	uint16_t length = AsebaNativePopArg(vm) * 2;

	vm->variables[status] = sd_user_read(data,length) / 2;
}

AsebaNativeFunctionDescription AsebaNativeDescription_sd_seek = {
	"sd.seek",
	"Seek the opened file",
	{
		{1, "position"},
		{1, "status"},
		{0,0},
	}
};

void thymio_native_sd_seek(AsebaVMState * vm) {
	unsigned long seek =  vm->variables[AsebaNativePopArg(vm)];
	unsigned int status = AsebaNativePopArg(vm);

	vm->variables[status] = sd_user_seek(seek * 2);
}

AsebaNativeFunctionDescription AsebaNativeDescription_rf_nodeid = {
	"_rf.nodeid",
	"Set Wireless Node ID",
	{
		{1, "nodeID"},
		{0,0},
	}
};

void set_rf_nodeid(AsebaVMState * vm) {
	unsigned int nodeid = vm->variables[AsebaNativePopArg(vm)];
	rf_set_node_id(nodeid);
	rf_flash_setting();
}
