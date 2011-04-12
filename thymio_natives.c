#include <skel-usb.h>
#include "leds.h"
#include "sd.h"
#include "playback.h"
#include "behavior.h"


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
	"Start sound recording",
	{
		{1,"[0-9]"},
		{0,0},
	}
};

AsebaNativeFunctionDescription AsebaNativeDescription_play = {
	"sound.play",
	"Start sound playback",
	{
		{1,"[0-9]"},
		{0,0},
	}
};

void sound_playback(AsebaVMState *vm) {
	int number = vm->variables[AsebaNativePopArg(vm)];
	
	behavior_stop(B_SOUND_BUTTON);
	
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
	
	behavior_stop(B_SOUND_BUTTON);
	
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
		{1,"led 1"},
		{1,"led 2"},
		{1,"led 3"},
		{1,"led 4"},
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
		{1,"led 1"},
		{1,"led 2"},
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
	"leds.ntc",
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

