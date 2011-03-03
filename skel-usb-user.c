/* Descriptors */

const AsebaVMDescription vmDescription = {
	"thymio-II", // Name of the microcontroller
	{
		{1, "_id"}, // Do not touch it
		{1, "event.source"}, // Nor this one
		{VM_VARIABLES_ARG_SIZE, "event.args"}, // neither this one
		{2, "_fwversion"}, // Do not event think about changing this one ...
		{1, "_productId"}, // Robot type
		
		{5, "buttons._raw"},
		{5, "buttons.binary"},

		{7, "prox.horizontal"},
		{2, "prox.ground.ambiant"},
		{2, "prox.ground.reflected"},
		
		{2, "motor.target"},
		{2, "vbat"},
		{2, "motor.speed"},
		{2, "motor.pwm"},
		
		{3, "acc"},
		
		{1, "temperature"},
		
		{1, "rc5.address"},
		{1, "rc5.command"},
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
	{ NULL, NULL }
};

extern AsebaNativeFunctionDescription AsebaNativeDescription_set_led;
void set_led(AsebaVMState *vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription_poweroff;
void power_off(AsebaVMState *vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription_record;
void sound_record(AsebaVMState *vm);


extern AsebaNativeFunctionDescription AsebaNativeDescription_play;
void sound_playback(AsebaVMState *vm);

static const AsebaNativeFunctionDescription* nativeFunctionsDescription[] = {
	&AsebaNativeDescription__system_reboot,
	&AsebaNativeDescription__system_settings_read,
	&AsebaNativeDescription__system_settings_write,
	&AsebaNativeDescription__system_settings_flash,
	
	ASEBA_NATIVES_STD_DESCRIPTIONS,
	
	&AsebaNativeDescription_set_led,
	&AsebaNativeDescription_poweroff,
	&AsebaNativeDescription_record,
	&AsebaNativeDescription_play,
	
	0       // null terminated
};

static AsebaNativeFunctionPointer nativeFunctions[] = {
	AsebaNative__system_reboot,
	AsebaNative__system_settings_read,
	AsebaNative__system_settings_write,
	AsebaNative__system_settings_flash,
	
	ASEBA_NATIVES_STD_FUNCTIONS,
	
	set_led,
	power_off,
	sound_record,
	sound_playback,
};

