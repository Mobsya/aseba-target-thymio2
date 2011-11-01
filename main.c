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


/* Firmware version. Increment at every major change */

/* 
History:
0: First production firmware
1: Wallsocket charger off mode fix. This version is the production firmware.
2: Add rand() support and first public release, fix playback-recoreded sound, fix ground detection in explorer mode
3: Second public release, see wiki for changelog
*/
#define FW_VERSION 3

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

// The real configuration bits are set by the bootloader
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
#define PRIO_1KHZ 3
// SD PRIO: 2 
#define PRIO_BEHAVIOR 1

#define CHARGE_ENABLE_DIR _TRISF1
#define CHARGE_500MA  _LATF0


#define TIMER_ANALOG TIMER_2
#define TIMER_RC5 TIMER_3
#define TIMER_BEHAVIOR TIMER_1
#define TIMER_SLOW TIMER_4
#define TIMER_1KHZ TIMER_5

static unsigned int timer[2];

static void timer_1khz(int timer_id) {
	int i;
	
	disk_timerproc();
	
	for(i = 0; i < 2; i++) {
		if(vmVariables.timers[i]) {
			if(++timer[i] >= vmVariables.timers[i]) {
				timer[i] = 0;
				SET_EVENT(EVENT_TIMER0 + i);
			}
		}
	}
	
}

static void acc_cb(int x, int y, int z, int tap) {

	vmVariables.acc[0] = x;
	vmVariables.acc[1] = y;
	vmVariables.acc[2] = z;	
	if(tap) {
		SET_EVENT(EVENT_TAP);
		vmVariables.acc_tap = tap; // set only variable.
	}
	
	SET_EVENT(EVENT_ACC);
}

static int ntc_calib;
static void ntc_callback(int temp) {
	if(!ntc_calib) {
		vmVariables.ntc = temp;
		SET_EVENT(EVENT_TEMPERATURE);
	}
	rc5_enable();
}

static void rc5_callback(unsigned int address, unsigned int command) {
	vmVariables.rc5_address = address;
	vmVariables.rc5_command = command;
	SET_EVENT(EVENT_RC5);
}

static void ntc_tick(void) {
	rc5_disable();
	
	if(ntc_calib) {
		ntc_calib = 0;
		ntc_mesure();
	} else {
		ntc_calib = 1;
		ntc_calibrate();
	}	
}

static void timer_slow_callback(int timer) {
	// timer_slow is at 16 Hz
	static unsigned char ntc_prescaler = 0;
	if(++ntc_prescaler >= 8) {
		// We want a 2 Hz timer on the ntc
		ntc_prescaler = 0;
		ntc_tick();
	}
	
	mma7660_read_async();
}




void update_aseba_variables_write(void) {
	static unsigned int old_timer[2];
	int i;
	
	for(i = 0; i < 2; i++) {
		if(vmVariables.timers[i] != old_timer[i]) {
			old_timer[i] = vmVariables.timers[i];
			timer[i] = 0;
		}
	}
}	

// This function is used to shutdown everything exept USB
// When this function return all the peripheral should be :
// 	- Disabled
// 	- In minimal power consuption mode
void switch_off(void) { 
	behavior_stop(B_ALL);
	
	timer_disable(TIMER_SLOW);
	timer_disable_interrupt(TIMER_SLOW);
	
	analog_disable();
	pwm_motor_poweroff();
	prox_poweroff();
	sound_poweroff();
	sd_shutdown();
	leds_poweroff();
	mma7660_suspend();
	I2C3CONbits.I2CEN = 0; // Disable i2c.
	_MI2C3IE = 0;
	
	
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
	behavior_stop(B_ALL);
	
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
	int vm_present;
	int i;
	unsigned int seed;
	clock_set_speed(16000000UL,16);	
	
	setup_pps();
	setup_io();
	
	leds_init();

	CHARGE_500MA = 0; // Switch back to 100mA charge.
	
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
	
	// This is the horizontal prox. Vertical one are handled by the ADC
	// but ADC sync the motor mesurment with the prox, so we don't pullute it with noise ... 
	prox_init();
	
	
	analog_init(TIMER_ANALOG, PRIO_SENSORS);
	
	// Warning: We cannot use the SD before the analog init as some pin are on the analog port.

	ntc_init(ntc_callback,PRIO_NTC);
	

		
	i2c_init(I2C_3);
	i2c_init_master(I2C_3, 400000, PRIO_ACC);
	
	mma7660_init(I2C_3, MMA7660_DEFAULT_ADDRESS, acc_cb, 0);
	mma7660_set_mode(MMA7660_120HZ, 1);
	
	timer_init(TIMER_SLOW, 62500, 6);
	timer_enable_interrupt(TIMER_SLOW, timer_slow_callback, PRIO_ACC);
	
	rc5_init(TIMER_RC5, rc5_callback, PRIO_RC5);
	
	timer_enable(TIMER_SLOW);
	
	sd_init();
	timer_init(TIMER_1KHZ, 1000, 6);
	timer_enable_interrupt(TIMER_1KHZ, timer_1khz, PRIO_1KHZ);
	timer_enable(TIMER_1KHZ);
	
	vm_present = init_aseba_and_usb();

	vmVariables.fwversion[0] = FW_VERSION;
	vmVariables.fwversion[1] = FW_VARIANT;
	
	// SD file is more important than internal flash
	if(!sd_load_aseba_code())
		vm_present = 1;

	behavior_init(TIMER_BEHAVIOR, PRIO_BEHAVIOR);
	
	
	test_mode = sd_test_file_present();
	
	if(!test_mode)
		mode_init(vm_present);

	
	if( ! load_settings_from_flash()) {
		/* Todo */
	}
	
	play_sound(SOUND_POWERON);
	
	if(test_mode) {	
		test_mode_start();
		while(1) 
			idle_without_aseba();
	}
	
	
	while(behavior_enabled(B_MODE)) 
		idle_without_aseba();
	
	// get the random seed
	seed = 0;
	for(i = 0; i < 5; i++) {
		seed += vmVariables.buttons_mean[i];
		seed += vmVariables.buttons_noise[i];
	}
	seed += vmVariables.vbat[0];
	seed += vmVariables.vbat[1];
	
	for(i = 0; i < 3; i++) 
		seed += vmVariables.acc[i];
	
	AsebaSetRandomSeed(seed);
	
	for(i = 0; i < 3; i++)
		AsebaGetRandom();
	
	// Give full control to aseba. No way out (except reset).
	run_aseba_main_loop();
}

