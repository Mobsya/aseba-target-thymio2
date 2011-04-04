#include <skel-usb.h>
#include "leds.h"
#include "sd.h"
#include "playback.h"


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
	"_sound.record",
	"Start sound recording",
	{
		{1,"[0-9]"},
		{0,0},
	}
};

AsebaNativeFunctionDescription AsebaNativeDescription_play = {
	"_sound.play",
	"Start sound playback",
	{
		{1,"[0-9]"},
		{0,0},
	}
};

void sound_playback(AsebaVMState *vm) {
	int number = vm->variables[AsebaNativePopArg(vm)];
	play_sound(number);
}

void sound_record(AsebaVMState *vm) {
	int number = vm->variables[AsebaNativePopArg(vm)];
	char name[7] = {'r','0','.','r','a','w', 0};
	if(number < 0) {
		sd_stop_record();
		return;
	}
		
	if(number > 9)
		number = 9;
	name[1] += number;
	
	sd_start_record(name);
}

AsebaNativeFunctionDescription AsebaNativeDescription_set_led_circle = {
	"leds.circle",
	"Set circular ring leds",
	{
		{1,"led 1"},
		{1,"led 2"},
		{1,"led 3"},
		{1,"led 4"},
		{1,"led 5"},
		{1,"led 6"},
		{1,"led 7"},
		{1,"led 8"},
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

AsebaNativeFunctionDescription AsebaNativeDescription_play_freq = {
	"_sound.freq",
	"Play frequency",
	{
		{1,"dHz"},
		{1,"dS"},
		{0,0},
	}
};

void play_freq(AsebaVMState * vm) {
	int freq = vm->variables[AsebaNativePopArg(vm)];
	int time = vm->variables[AsebaNativePopArg(vm)];
	
	play_frequency_block(freq, time);
}

