#ifndef _ANALOG_H_
#define _ANALOG_H_
// Special ADC handling for thymio, we need 
// To synchronize motor, capacitive touch, and sensors
// on the same ADC.

// Moreover the capacitive touch is used as a power on switch
// So we need to use the adc in a very special way.

void analog_init(int t, int prio);

// this will put the pic in "Poweroff" mode.
// You should already have switched off almost everything
// this will unconditionnaly enter sleep mode and wake up 
// every X ms to poll for _ONE_ capacitive touch button 
// And wake up if it's pressed for more than Y ms.
// (X and Y TBD)
// TODO: check interaction with USB
void __attribute__((noreturn)) analog_enter_poweroff_mode(void);

// Disable the analog sampling
void analog_disable(void);


void cb_1khz(void);

#endif
