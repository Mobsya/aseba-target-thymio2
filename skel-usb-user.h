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

// No multiple include protection since this file must ONLY BE INCLUDED FROM SKEL.h

// Should put the usb stuff here ... 


#define FLASH_END 0x15800

#define PRIO_COMMUNICATION 4

#define PRODUCT_ID 8

/* Send queue minimum size: 512+6+4+1 */
#define SEND_QUEUE_SIZE (512+6+4+1)

/* Recv queue minimum size: 512+6+4+1+USB_MTU */
#define RECV_QUEUE_SIZE (512+6+4+1+64)

/* This is the number of "private" variable the aseba script can have */
#define VM_VARIABLES_FREE_SPACE 512

/* THis is the maximum number of argument an aseba event can recieve */
#define VM_VARIABLES_ARG_SIZE 32

/* THE nuber of opcode an aseba script can have */
#define VM_BYTECODE_SIZE (766+768)	// Put here 766+768*a, where a is >= 0
#define VM_STACK_SIZE 32

struct _vmVariables {
	// NodeID
	int16_t id;
	// source
	int16_t source;
	// args
	int16_t args[VM_VARIABLES_ARG_SIZE];
	// fwversion
	int16_t fwversion[2];
	// Product ID
	int16_t productid;

	
	int16_t buttons[5];
	
	int16_t buttons_state[5];

	int16_t buttons_mean[5];
	int16_t buttons_noise[5];
	
	int16_t prox[7];
        int16_t sensor_data[7];
        int16_t intensity[7];
        int16_t rx_data;
        int16_t ir_tx_data;


	int16_t ground_ambiant[2];
	int16_t ground_reflected[2];
	int16_t ground_delta[2];
	
	int16_t target[2];
	
	int16_t vbat[2];
	int16_t imot[2];
	
	int16_t uind[2];
	int16_t pwm[2];
	
	int16_t acc[3];
	
	int16_t ntc;
	
	int16_t rc5_address;
	int16_t rc5_command;
	
	int16_t sound_level;
	int16_t sound_tresh;
	int16_t sound_mean;
	
	int16_t timers[2];
	
	int16_t acc_tap;
	
	int16_t sd_present;
	/*****
	---> PUT YOUR VARIABLES HERE <---
	******/
	
	int16_t freeSpace[VM_VARIABLES_FREE_SPACE];
};

enum Event
{
	EVENT_B_BACKWARD = 0,
	EVENT_B_LEFT,
	EVENT_B_CENTER,
	EVENT_B_FORWARD,
	EVENT_B_RIGHT,
	EVENT_BUTTONS,
	EVENT_PROX,
        EVENT_DATA,
	EVENT_TAP,
	EVENT_ACC,
	EVENT_MIC,
	EVENT_SOUND_FINISHED,
	EVENT_TEMPERATURE,
	EVENT_RC5,
	EVENT_MOTOR,
	// Must be consecutive
	EVENT_TIMER0,
	EVENT_TIMER1,

// Maximum count: 32
	EVENT_COUNT // Do not touch
};

// The content of this structure is implementation-specific
// The glue provide a way to store and retrive it from flash.
// The only way to write it is to do it from inside the VM (Native function)
// The native function access it as a integer array. So, use only int inside this structure
struct private_settings {
	/* ADD here the settings to save into flash */
	/* The minimum size is one integer, the maximum size is 95 integer (check done at compilation) */
	int sound_shift;
        int prox_min[7];
        int mot256[2];
	int prox_ground_max[2];
	int settings[80];
};
	

