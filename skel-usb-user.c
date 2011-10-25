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
		{1, "buttons.backward"},
		{1, "buttons.right"},
		{1, "buttons.center"},
		{1, "buttons.forward"},
		{1, "buttons.left"},
			
		{5, "buttons._mean"},
		{5, "buttons._noise"},
		
		{7, "prox.horizontal"},
		{2, "prox.ground.ambiant"},
		{2, "prox.ground.reflected"},
		{2, "prox.ground.delta"},
		
		{2, "motor.target"},
		{2, "_vbat"},
		{2, "motor.speed"},
		{2, "motor.pwm"},
		
		{3, "acc"},
		
		{1, "temperature"},
		
		{1, "rc5.address"},
		{1, "rc5.command"},
		
		{1, "acc._tap"},
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
	{ "button", "Button event"},
	{ "motor", "Motor timer"},
	{ "acc", "Accelerometer event"},
	{ "temperature", "Temperature event"},
	{ "rc5", "RC5 message event"},
	{ "prox", "Proximity sensors"},
	{ "tap", "A tap is detected"},
	{ "button.left", "Left button event"},
	{ "button.right", "Right button event"},
	{ "button.forward", "Forward button event"},
	{ "button.backward", "Backward button event"},
	{ "button.center", "Center button event"},
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

