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
4: Bugfix release
5: Limit motor current in soft, improve button detection, change timer use
6: Fix bug on MacOSX 10.7+, improve RC5 processing
7: IR Communication, SD access from VM, others improvments
8: Motors back EMF calibration, line following black/white level calibration storage in SD, emtpy bytecode detection, minors fixes
9: Modulo division by zero bug fix, native.c fixes, disconnection improvement, add setting mode
10: Change wireless nodeID, acc sensitivity, clean VM on load, aseba protocole version 5
*/
#define FW_VERSION 10

/* Firmware variant. Each variant of the firmware has it own number */

/* Variant list:
0: Standard one
1: Development one

*/
#define FW_VARIANT 1

#include <p24Fxxxx.h>
#include <string.h>
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
#include "log.h"
#include "rf.h"
#include "motor.h"

#include <skel-usb.h>

// The real configuration bits are set by the bootloader
   _CONFIG1 (0x3F69)
   _CONFIG2 (0x134D)
   _CONFIG3 (0x6000)
//_CONFIG1(JTAGEN_OFF & GCP_OFF & GWRP_OFF & BKBUG_OFF & COE_OFF & ICS_PGx1 & FWDTEN_OFF & WINDIS_OFF & FWPSA_PR32 & WDTPS_PS1);
//_CONFIG2(IESO_OFF & PLLDIV_DIV2 & PLL96DIS_ON & FNOSC_PRIPLL & FCKSM_CSDCMD & OSCIOFNC_OFF & IOL1WAY_OFF & DISUVREG_OFF & POSCMOD_HS) 
//_CONFIG3(WPEND_WPSTARTMEM & WPCFG_WPCFGDIS & WPDIS_WPEN) 

/* 4 Softirq are used on INT1-4 
 * INT1,INT2 are used for sound (IPL 2)
 * INT3 is used to trigger poweroff (IPL 1)
 * INT4 is used to handle the behavior (IPL 1)
 */

// Priority 2 is reserverd for SD operations ...
#define PRIO_SENSORS 6
// Need to be higher than communication priority
#define PRIO_I2C 5 
// Communication priority: 4
#define PRIO_RC5 4
#define PRIO_1KHZ 3
// SD PRIO: 2 
#define PRIO_BEHAVIOR 1

#define CHARGE_ENABLE_DIR _TRISF1
#define CHARGE_500MA  _LATF0

#define TIMER_ANALOG	TIMER_2
#define TIMER_RC5		TIMER_4
#define TIMER_1KHZ		TIMER_5
#define TIMER_IR_COMM	TIMER_1 // Timer number hardcoded into prox. processing code.

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

static void timer_slow(void) {
	// timer_slow is at 16 Hz
	static unsigned char ntc_prescaler = 0;
	if(++ntc_prescaler >= 8) {
		// We want a 2 Hz timer on the ntc
		ntc_prescaler = 0;
		ntc_tick();
	}
	
	// Yeah .... quite hackish, but the I2C is shared between RF and 
	// accelerometer. RF has to schedule the acc when the bus is idle.
	rf_schedule_acc_read();
}

static unsigned int timer[2];


static void timer_1khz(int timer_id) {
	int i;
	static unsigned char timer_20ms;
	static unsigned char timer_62ms;
	static unsigned char timer_10ms;
	
	disk_timerproc();
	
	log_poweron_tick();
	
	// Poll the RF at 100Hz
	if(++timer_10ms >= 10) {
		timer_10ms = 0;
		rf_poll();
	}	
	
	// Generate behavior softirq at 50Hz
	if(++timer_20ms >= 20) {
		timer_20ms = 0;
		behavior_trigger();
	}

	// Generate 16Hz, used by NTC & Acc.
	if(++timer_62ms >= 62) {
		timer_62ms = 0;
		timer_slow();
	}
	
	for(i = 0; i < 2; i++) {
		if(vmVariables.timers[i]) {
			if(++timer[i] >= vmVariables.timers[i]) {
				timer[i] = 0;
				SET_EVENT(EVENT_TIMER0 + i);
			}
		}
	}
	
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

	pid_motor_set_target((int *) vmVariables.target);
}	

static void wait_valid_vbat(void) {
        int i;
        vmVariables.vbat[0] = 0;
        barrier();

        for(i = 0; i < 10000; i++) {
                if(vmVariables.vbat[0])
                        break;
                clock_delay_us(1);
        }
}

// This function is used to shutdown everything exept USB
// When this function return all the peripheral should be :
// 	- Disabled
// 	- In minimal power consuption mode
void switch_off(void) { 
	behavior_stop(B_ALL);
	
	timer_disable(TIMER_1KHZ);
	timer_disable_interrupt(TIMER_1KHZ);
	
	_LVDIE = 0;

    // Why waiting on valid vbat ?
    //   => We use an aseba variable, the user may corrupt it
    wait_valid_vbat(); // ir autocalibration is using it.

	analog_disable();
	pwm_motor_poweroff();
	prox_poweroff();
	save_settings();
	sound_poweroff();
	sd_shutdown();
	leds_poweroff();
	mma7660_suspend();
	rf_poweroff();
	
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
        unsigned int flags;

    // Protect against two racing poweroff:
    //  One from the softirq (button)
    //  One from the VM
    RAISE_IPL(flags,1);

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

void _ISR _LVDInterrupt(void) {
	_LVDIF = 0;
	_LVDIE = 0;
	_INT3IF = 1;
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

	motor_get_vind((int *) vmVariables.uind);
}

static void idle_without_aseba(void) {
	clock_idle(); // race WRT interrupt, but we have tons of periodic interrupt which will wake us ..
	update_aseba_variables_read();
	update_aseba_variables_write();
}

int main(void)
{   
	int test_mode;
	int vm_present;
	int i;
	unsigned int seed;

	// Needs to be called ASAP as rf need a looooooong time to wake up.
	// This function is just sending a pulse over the SCL line.
	rf_wakeup();
	
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

	// We need the settings for the horizontal prox.
	load_settings_from_flash();


	for (i = 0; i < 2; i++) {
		// Settings is definitely wrong....
		if(settings.mot256[i] <= 0)
			settings.mot256[i] = 256;

		// 1024 (AD resolution is 10 bits) * 256 / 9 fits in signed 16 bits.
		if (settings.mot256[i] < 9)
			settings.mot256[i] = 9;
	}
	
	// This is the horizontal prox. Vertical one are handled by the ADC
	// but ADC sync the motor mesurment with the prox, so we don't pullute it with noise ...
	
	timer_init(TIMER_IR_COMM, 0,-1); // The period will be changed later.
	prox_init(PRIO_SENSORS);  // Same priority as analog (maybe should be at 7 ...)
	
	// Warning: We cannot use the SD before the analog init as some pin are on the analog port.
	analog_init(TIMER_ANALOG, PRIO_SENSORS);

        wait_valid_vbat();
        
	log_init(); // We will need to read vbat to be sure we can flash.

	ntc_init(ntc_callback, PRIO_1KHZ);

//	i2c_init(I2C_3);

	i2c_init_master(I2C_3, 400000, PRIO_I2C);
	I2C3CON = 0x9000;

	
	mma7660_init(I2C_3, MMA7660_DEFAULT_ADDRESS, acc_cb, 0);
	mma7660_set_mode(MMA7660_120HZ, 1);
	
	rc5_init(TIMER_RC5, rc5_callback, PRIO_RC5);
	
	sd_init();

	timer_init(TIMER_1KHZ, 1000, 6);
	timer_enable_interrupt(TIMER_1KHZ, timer_1khz, PRIO_1KHZ);
	
	rf_init(I2C_3);
	
	timer_enable(TIMER_1KHZ);
	
	sd_log_file();
	
	vm_present = init_aseba_and_fifo();
	
	if(vm_present) 
		log_analyse_bytecode();

	vmVariables.fwversion[0] = FW_VERSION;
	vmVariables.fwversion[1] = FW_VARIANT;
	
	// SD file is more important than internal flash
	if(!sd_load_aseba_code()) {
		log_set_flag(LOG_FLAG_VMCODESD);
		vm_present = 1;
		log_analyse_bytecode();
	}

	// Behavior is on INT4 (softirq trigged by 1khz timer).
	behavior_init(PRIO_BEHAVIOR);
	
	
	test_mode = sd_test_file_present();
	
	if(!test_mode)
		mode_init(vm_present);

	
	
	// Enable the LVD interrupt
	_LVDIE = 1;
	
	play_sound(SOUND_POWERON);
	
	if(test_mode) {	
		test_mode_start();
		while(1) 
			idle_without_aseba();
	}
	
	while(behavior_enabled(B_MODE|B_SETTING)) 
		idle_without_aseba();
	
	// If usb did not put us out of behavior mode, then start the rf link
	if(!usb_uart_serial_port_open() && (rf_get_status() & RF_PRESENT)) {
		rf_set_link(RF_UP);
	}

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
	
	//test if SD card is present
	vmVariables.sd_present = !sd_user_open("_TESTSD");
	sd_user_open(NULL);
	
	// Give full control to aseba. No way out (except reset).
	run_aseba_main_loop();
}



void AsebaVMResetCB(AsebaVMState *vm) {
	leds_set_circle(0,0,0,0,0,0,0,0);
	leds_set_body_rgb(0,0,0);
	leds_set(LED_SOUND,0);
	leds_set(LED_RC,0);
	behavior_start(B_LEDS_ACC);
	behavior_start(B_LEDS_NTC);
	behavior_start(B_LEDS_MIC);
	behavior_start(B_LEDS_PROX);
	behavior_start(B_SOUND_BUTTON);
	behavior_start(B_LEDS_MIC);
	behavior_start(B_LEDS_RC5);
	prox_disable_network();
	events_flags[0] = 0;
	events_flags[1] = 0;
	memset(vm->variables, 0, vm->variablesSize*sizeof(sint16));
	vmVariables.id = vmState.nodeId;
	vmVariables.productid = PRODUCT_ID;
	vmVariables.fwversion[0] = FW_VERSION;
	vmVariables.fwversion[1] = FW_VARIANT;
	vmVariables.sd_present = !sd_user_open("_TESTSD");
	sd_user_open(NULL);	
}
