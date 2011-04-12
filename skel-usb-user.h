// No multiple include protection since this file must ONLY BE INCLUDED FROM SKEL.h

// Should put the usb stuff here ... 


#define FLASH_END 0x15800

#define PRIO_USB 5

#define PRODUCT_ID 8

/* Send queue minimum size: 512+6+4+1 */
#define SEND_QUEUE_SIZE (512+6+4+1)

/* Recv queue minimum size: 512+6+4+1 */
#define RECV_QUEUE_SIZE (512+6+4+1)

/* This is the number of "private" variable the aseba script can have */
#define VM_VARIABLES_FREE_SPACE 512

/* THis is the maximum number of argument an aseba event can recieve */
#define VM_VARIABLES_ARG_SIZE 32

/* THE nuber of opcode an aseba script can have */
#define VM_BYTECODE_SIZE (766+768)	// Put here 766+768*a, where a is >= 0
#define VM_STACK_SIZE 32

struct _vmVariables {
	// NodeID
	sint16 id;
	// source
	sint16 source;
	// args
	sint16 args[VM_VARIABLES_ARG_SIZE];
	// fwversion
	sint16 fwversion[2];
	// Product ID
	sint16 productid;

	
	sint16 buttons[5];
	
	sint16 buttons_state[5];

	sint16 buttons_mean[5];
	sint16 buttons_noise[5];
	
	sint16 prox[7];
	sint16 ground_ambiant[2];
	sint16 ground_reflected[2];
	sint16 ground_delta[2];
	
	sint16 target[2];
	
	sint16 vbat[2];
	
	sint16 uind[2];
	sint16 pwm[2];
	
	sint16 acc[3];
	
	sint16 ntc;
	
	sint16 rc5_address;
	sint16 rc5_command;
	
	sint16 acc_tap;
	
	/*****
	---> PUT YOUR VARIABLES HERE <---
	******/
	
	sint16 freeSpace[VM_VARIABLES_FREE_SPACE];
};

enum Event
{
	EVENT_BUTTONS = 0,
	EVENT_MOTOR,
	EVENT_ACC,
	EVENT_TEMPERATURE,
	EVENT_RC5,
	EVENT_PROX,
	/****
	---> PUT YOUR EVENT NUMBER HERE <---
	Must be in the same order as in skel.c
	*****/
	EVENT_COUNT // Do not touch
};

// The content of this structure is implementation-specific
// The glue provide a way to store and retrive it from flash.
// The only way to write it is to do it from inside the VM (Native function)
// The native function access it as a integer array. So, use only int inside this structure
struct private_settings {
	/* ADD here the settings to save into flash */
	/* The minimum size is one integer, the maximum size is 95 integer (check done at compilation) */
	int settings[90];
};
	

