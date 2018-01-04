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

/* Descriptors */

#include "thymio_natives.h"

const AsebaVMDescription vmDescription = {
	"thymio-II", // Name of the microcontroller
	{
		{1, "_id"}, // Do not touch it
		{1, "event.source"}, // Nor this one
		{VM_VARIABLES_ARG_SIZE, "event.args"}, // neither this one
		{2, "_fwversion"}, // Do not event think about changing this one ...
		{1, "_productId"}, // Robot type
		
		{5, "buttons._raw"},
		{1, "button.backward"},
		{1, "button.left"},
		{1, "button.center"},
		{1, "button.forward"},
		{1, "button.right"},
			
		{5, "buttons._mean"},
		{5, "buttons._noise"},
		
		{7, "prox.horizontal"},

		{7, "prox.comm.rx._payloads"},
		{7, "prox.comm.rx._intensities"},
		{1, "prox.comm.rx"},
		{1, "prox.comm.tx"},

		{2, "prox.ground.ambiant"},
		{2, "prox.ground.reflected"},
		{2, "prox.ground.delta"},
		
		{1, "motor.left.target"},
		{1, "motor.right.target"},
		{2, "_vbat"},
		{2, "_imot"},
		{1, "motor.left.speed"},
		{1, "motor.right.speed"},
		{1, "motor.left.pwm"},
		{1, "motor.right.pwm"},
		
		{3, "acc"},
		
		{1, "temperature"},
		
		{1, "rc5.address"},
		{1, "rc5.command"},
		
		{1, "mic.intensity"},
		{1, "mic.threshold"},
		{1, "mic._mean"},
		
		{2, "timer.period"},
		
		{1, "acc._tap"},
				
		{1, "sd.present"},		
		/******
		 ---> PUT YOUR VARIABLES DESCRIPTIONS HERE <---
		first value is the number of element in the array (1 if not an array)
		second value is the name of the variable which will be displayed in aseba studio
		******/

		{0, NULL} // Null terminated
	}
};

static const AsebaLocalEventDescription localEvents[] = {
	/*******
	---> PUT YOUR EVENT DESCRIPTIONS HERE <---
	First value is event "name" (will be used as "onvent name" in asebastudio
	second value is the event description)
	*******/
	{ "button.backward", "Backward button status changed"},
	{ "button.left", "Left button status changed"},
	{ "button.center", "Center button status changed"},
	{ "button.forward", "Forward button status changed"},
	{ "button.right", "Right button status changed"},	
	{ "buttons", "Buttons values updated"},
	{ "prox", "Proximity values updated"},
	{ "prox.comm", "Data received on the proximity communication"},
	{ "tap", "A tap is detected"},
	{ "acc", "Accelerometer values updated"},
	{ "mic", "Fired when microphone intensity is above threshold"},	
	{ "sound.finished", "Fired when the playback of a user initiated sound is finished"},
	{ "temperature", "Temperature value updated"},
	{ "rc5", "RC5 message received"},
	{ "motor", "Motor timer"},
	{ "timer0", "Timer 0"},
	{ "timer1", "Timer 1"},
	{ NULL, NULL }
};


extern AsebaNativeFunctionDescription AsebaNativeDescription_poweroff;
void power_off(AsebaVMState *vm);


static const AsebaNativeFunctionDescription* nativeFunctionsDescription[] = {
	&AsebaNativeDescription__system_reboot,
	&AsebaNativeDescription__system_settings_read,
	&AsebaNativeDescription__system_settings_write,
	&AsebaNativeDescription__system_settings_flash,
	
	ASEBA_NATIVES_STD_DESCRIPTIONS,
	
	THYMIO_NATIVES_DESCRIPTIONS,
	
	&AsebaNativeDescription_poweroff,
	0       // null terminated
};

static AsebaNativeFunctionPointer nativeFunctions[] = {
	AsebaNative__system_reboot,
	AsebaNative__system_settings_read,
	AsebaNative__system_settings_write,
	AsebaNative__system_settings_flash,
	
	ASEBA_NATIVES_STD_FUNCTIONS,
	
	THYMIO_NATIVES_FUNCTIONS,
	
	power_off
};

