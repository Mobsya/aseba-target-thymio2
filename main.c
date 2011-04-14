/* Firmware version. Increment at every major change */

/* 
History:
0: First production firmware
1: Wallsocket charger off mode fix.
*/
#define FW_VERSION 1

/* Firmware variant. Each variant of the firmware has it own number */

/* Variant list:
0: Standard one

*/
#define FW_VARIANT 0

#include <p24fxxxx.h>
#include <clock/clock.h>
#include <timer/timer.h>
#include <gpio/gpio.h>
#include <i2c/i2c.h>

#include "usb_uart.h"

#include "leds.h"
#include "pmp.h"
#include "analog.h"
#include "ir_prox.h"
#include "sound.h"
#include "playback.h"
#include "pwm_motor.h"
#include "pid_motor.h"
#include "mma7660.h"
#include "ntc.h"
#include "rc5.h"
#include "sd/diskio.h"
#include "sd.h"
#include "tone.h"
#include "button.h"
#include "behavior.h"
#include "mode.h"
#include "test.h"

#include <skel-usb.h>

//#error CHECK THIS !
   _CONFIG1 (0x3F69)
   _CONFIG2 (0x134D)
   _CONFIG3 (0x6000)
//_CONFIG1(JTAGEN_OFF & GCP_OFF & GWRP_OFF & BKBUG_OFF & COE_OFF & ICS_PGx1 & FWDTEN_OFF & WINDIS_OFF & FWPSA_PR32 & WDTPS_PS1);
//_CONFIG2(IESO_OFF & PLLDIV_DIV2 & PLL96DIS_ON & FNOSC_PRIPLL & FCKSM_CSDCMD & OSCIOFNC_OFF & IOL1WAY_OFF & DISUVREG_OFF & POSCMOD_HS) 
//_CONFIG3(WPEND_WPSTARTMEM & WPCFG_WPCFGDIS & WPDIS_WPEN) 

// Priority 2 is reserverd for SD operations ...
#define PRIO_SENSORS 6
// USB priority: 5
#define PRIO_RC5 4
#define PRIO_ACC 3
#define PRIO_NTC 3
// SD PRIO: 2 
#define PRIO_BEHAVIOR 1

#define CHARGE_ENABLE_DIR _TRISF1
#define CHARGE_500MA  _LATF0


#define TIMER_ANALOG TIMER_2
#define TIMER_RC5 TIMER_3
#define TIMER_BEHAVIOR TIMER_1
#define TIMER_NTC TIMER_45


void cb_1khz(void) {
	disk_timerproc();		
}

static void acc_cb(int x, int y, int z, int tap) {

	vmVariables.acc[0] = x;
	vmVariables.acc[1] = y;
	vmVariables.acc[2] = z;	
	if(tap)
		vmVariables.acc_tap = tap; // set only variable.
	
	SET_EVENT(EVENT_ACC);
}

static void ntc_callback(int temp) {
	vmVariables.ntc = temp;
	SET_EVENT(EVENT_TEMPERATURE);
	rc5_enable();
}

static void rc5_callback(unsigned int address, unsigned int command) {
	vmVariables.rc5_address = address;
	vmVariables.rc5_command = command;
	SET_EVENT(EVENT_RC5);
}

static void timer_ntc_callback(int timer) {
	rc5_disable();
	ntc_mesure();
}




void update_aseba_variables_write(void) {
}	

// This function is used to shutdown everything exept USB
// When this function return all the peripheral should be :
// 	- Disabled
// 	- In minimal power consuption mode
void switch_off(void) { 
	behavior_stop(B_ALL);
	
	analog_disable();
	pwm_motor_poweroff();
	prox_poweroff();
	sound_poweroff();
	sd_shutdown();
	leds_poweroff();
	mma7660_suspend();
	I2C3CONbits.I2CEN = 0; // Disable i2c.
	_MI2C3IE = 0;
	
	timer_disable(TIMER_NTC);
	timer_disable_interrupt(TIMER_NTC);
	
	ntc_shutdown();
	rc5_shutdown();

	CHARGE_500MA = 0; 
	// TODO: Force VA ?! Check me that refcount is correct
}

AsebaNativeFunctionDescription AsebaNativeDescription_poweroff = {
	"_poweroff",
	"Poweroff",
	{
		{0,0}
	}
};

void power_off(AsebaVMState *vm) {
 	play_sound_block(SOUND_POWEROFF);
	
	// Shutdown all peripherals ...
	switch_off();
	
	// Switch off USB
	// If we are connected to a PC, disconnect.
	// If we are NOT connected to a PC but 5V is present
	// ( == charger ) we need to keep the transciever on
	if(usb_uart_configured())
		USBDeviceDetach();
		
	// In any case, disable the usb interrupt. It's safer
	_USB1IE = 0;
	
	
	CHARGE_ENABLE_DIR = 1;
	
	analog_enter_poweroff_mode();
}

void _ISR _INT3Interrupt(void) {
	_INT3IF = 0;
	_INT3IE = 0; // ack & disable
	// Poweroff softirq
	power_off(NULL);
}

void update_aseba_variables_read(void) {
	// TODO: REMOVE ME (move to behavior ? /!\ behavior == IPL 1 !! race wrt aseba !)
	usb_uart_tick();
}

static void idle_without_aseba(void) {
	clock_idle(); // race WRT interrupt, but we have tons of periodic interrupt which will wake us ..
	usb_uart_tick(); // TODO: remove me (mote to behavior ?)	
}

int main(void)
{   
	int test_mode;
	int flash_present;
	clock_set_speed(16000000UL,16);	
	
	setup_pps();
	setup_io();
	
	leds_init();

	_LATF0 = 0; // Switch back to 100mA charge.
	
	// Switch on one led to say we are powered on
	leds_set(LED_BATTERY_0, 32);

	// Enable the poweroff softirq.
	_INT3IF = 0;
	_INT3IP = 1;
	_INT3IE = 1;

	// Sound must be enabled before analog, as 
	// The analog interrupt callback into sound processing ... 
	// But must be initialised _after_ leds as it use one led IO for enabling amp.
	sound_init();
	tone_init(); // Init tone generator
	
	pwm_motor_init();
	pid_motor_init();
	
	// We have to init sd statemachine before analog as analog irq is used for 1khz timer
	sd_init();
	
	// This is the horizontal prox. Vertical one are handled by the ADC
	// but ADC sync the motor mesurment with the prox, so we don't pullute it with noise ... 
	prox_init();
	
	
	analog_init(TIMER_ANALOG, PRIO_SENSORS);
	
	// Warning: We cannot use the SD before the analog init as some pin are on the analog port.

	ntc_init(ntc_callback,PRIO_NTC);
	

		
	i2c_init(I2C_3);
	i2c_init_master(I2C_3, 400000, PRIO_ACC);
	
	mma7660_init(I2C_3, MMA7660_DEFAULT_ADDRESS, acc_cb, PRIO_ACC);
	mma7660_set_mode(MMA7660_120HZ, 1);
	
	timer_init(TIMER_NTC, 1000, 3); // 32 bits timer
	timer_enable_interrupt(TIMER_NTC, timer_ntc_callback, PRIO_ACC);
	
	rc5_init(TIMER_RC5, rc5_callback, PRIO_RC5);
	
	timer_enable(TIMER_NTC);
	
	flash_present = init_aseba_and_usb();

	behavior_init(TIMER_BEHAVIOR, PRIO_BEHAVIOR);
	
	
	test_mode = sd_test_file_present();
	
	if(!test_mode)
		mode_init(flash_present);

	
	if( ! load_settings_from_flash()) {
		/* Todo */
	}
	
	play_sound(SOUND_POWERON);
	
	if(test_mode) {
		mma7660_set_mode(MMA7660_16HZ, 0);
		
		test_mode_start();
		while(1) 
			idle_without_aseba();
	}
	
	
	while(behavior_enabled(B_MODE)) 
		idle_without_aseba();
		
	mma7660_set_mode(MMA7660_16HZ, 0);
		
	vmVariables.fwversion[0] = FW_VERSION;
	vmVariables.fwversion[1] = FW_VARIANT;
	
	// Give full control to aseba. No way out (except reset).
	run_aseba_main_loop();
}

