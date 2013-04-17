/*
        Thymio-II Firmware

        Copyright (C) 2011 Florian Vaussard <florian dot vaussard at epfl dot ch>,
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

#include <types/types.h>
#include <types/uc.h>
#include <timer/timer.h>
#include <error/error.h>

#include "rc5.h"
#include "regulator.h"

typedef enum{
	STOP,
	RECEIVE,
} states;

#define		THRESHOLD_0	24000
#define		THRESHOLD_1	35556
#define		THRESHOLD_2	49778

// Proto
void m_reset_state_machine(void);
void m_flush_buffer(void);
void m_throw_cb(void);

// Static variables
static rc5_cb cb;
static unsigned int m_timer;
static states m_state;
static unsigned int previous_ic;

static unsigned int rc5_data;
static int rc5_index;		// Point to the next bit of rc5_data, from MSB to LSB
				// Negative index possible in case of errors

unsigned char rc5_valid_flag;

static void timer_cb(int timer_id)
{
	// Test rc5_index: fire the callback or reset the state
	if (rc5_index == 1) {
		// Frame finished
		m_throw_cb();
	} else if (rc5_index == 2) {
		// Last sequence was 1.0 and there is no falling edge for the 0 -> not detected
		if (rc5_data & (1 << (rc5_index + 1))) {
			// previous = 1
			// -> 0
			rc5_index--;
			m_throw_cb();
		} else {
			// Oops... was a 0 -> should not happen
			m_reset_state_machine();
		}
	} else {
		// Oops... Error! Reset
		m_reset_state_machine();
	}
}

void rc5_init(unsigned int timer, rc5_cb ucb, int priority)
{
	va_get();
	
	cb = ucb;
	m_timer = timer;

	// Reset the state machine
	m_state = STOP;

	// Configure IC9 (RP26)
	IC9CON1bits.ICTSEL = 7;		// Fosc/2
	IC9CON1bits.ICI = 0;		// Interrupt on every capture
//	IC9CON1bits.ICM = 3;		// Capture on rising edges
	IC9CON1bits.ICM = 2;		// Capture on falling edges
//	IC9CON2bits.SYNCSEL = 30;	// IC9 (0b11110) as sync source
	IC9CON2bits.SYNCSEL = 0;	// No sync source -> free running
	IC9CON2bits.ICTRIG = 0;		// Synchro operation

	// Configure the "watchdog" timer
	timer_init(m_timer, 6, 3);	// 6 ms
	timer_enable_interrupt(m_timer, timer_cb, priority);
	
	// Enable Interrupt
	_IC9IP = priority;
	_IC9IF = 0;
	_IC9IE = 1;
}

void rc5_shutdown(void)
{
	// Disable interrupt
	_IC9IE = 0;

	// Shutdown IC9
	IC9CON1bits.ICM = 0;

	// Reset
	m_reset_state_machine();
	
	va_put();
}

// Temporally disable interrupt
void rc5_disable(void)
{
	// Disable interrupt
	_IC9IE = 0;
	// Shutdown IC9
	IC9CON1bits.ICM = 0; 

	// Reset
	m_reset_state_machine();
}

// Re-enable the interrupt
void rc5_enable(void)
{
	// Re-enable interrupt
	_IC9IE = 1;
	// Reenable IC9
	IC9CON1bits.ICM = 2;
}

// As the name says...
void m_reset_state_machine(void)
{
	m_state = STOP;
	timer_disable(m_timer);
	// Flush the buffer
	m_flush_buffer();
}

// Flush the IC buffer
void m_flush_buffer(void)
{
	unsigned int var;

	// Empty the buffer
	while (IC9CON1bits.ICBNE){
		var = IC9BUF;
	}
}

// At the completion of a frame, fire the callback
void m_throw_cb(void)
{
	// Save the data, so we can perform a reset
	static unsigned int previous_rc5 = 0;
	unsigned int rc5_data_save = rc5_data; 		// Avoid inconsistency if rc5_data changes in the interrupt

	m_reset_state_machine();

	if (cb) {
		
		// Start bit valid ? 
		if(rc5_data_save >> 15) {
			rc5_valid_flag = 1;
			// Check if data has changed
			if (rc5_data_save != previous_rc5) {
				unsigned char command;
				previous_rc5 = rc5_data_save;
				command = (rc5_data_save >> 2) & 0b111111;
				command |= ((rc5_data_save & (1 << 14)) >> 8) ^ (1 << 6); // "Extended" bit, inverted
				cb((rc5_data_save >> 8) & 0b11111, command);
			}
			// else: only a repeated frame -> do nothing
		}
	}
}

// ISR
void _ISR _IC9Interrupt(){
	// ACK
	_IC9IF = 0;

	// State machine
	if (m_state == STOP){
		// New frame
		m_state = RECEIVE;
		// Store the current value as start reference
		previous_ic = IC9BUF;
		// First bit = start bit -> 1
		rc5_index = 15;
		rc5_data = (1 << rc5_index);
		rc5_index--;
		// Start the watchdog
		timer_set_value(m_timer, 0);
		timer_enable(m_timer);
		return;
	} else {
		// Reset the watchdog
		timer_set_value(m_timer, 0);
	}

	// Overflow?
	if (IC9CON1bits.ICOV) {
		// Overflow has occured, reset
		m_reset_state_machine();
		return;
	}

	unsigned int current_value;
	unsigned int delta;

	// Read the buffer
	while(IC9CON1bits.ICBNE){
		current_value = IC9BUF;
		delta = current_value - previous_ic;		// Takes into account the overflow (while delta < 4ms)
		previous_ic = current_value;
		// Quantify the time elapsed between two falling edges
		if ((delta > THRESHOLD_0) && (delta < THRESHOLD_1)) {
			// 1 RC5 cycle
			// Current bit is equal to the previous one
			rc5_data |= ((rc5_data & (1 << (rc5_index + 1))) >> 1);
			rc5_index--;
		} else if ((delta > THRESHOLD_1) && (delta < THRESHOLD_2)) {
			// 1.5 RC5 cycles
			// If previous bit is 0, next one is 1
			// If previous bit is 1, next ones are 00
			if (rc5_data & (1 << (rc5_index + 1))) {
				// previous = 1
				// -> 00
				rc5_index -= 2;
			} else {
				// previous = 0
				// -> 1
				rc5_data |= (1 << rc5_index);
				rc5_index--;
			}
		} else if (delta > THRESHOLD_2) {
			// 2 RC5 cycles
			// Previous bit MUST be 1, next ones are 01
			// If previous bit is 0 --> Error !!!
			if (rc5_data & (1 << (rc5_index + 1))) {
				// previous = 1
				// -> 01
				rc5_index--;
				rc5_data |= (1 << rc5_index);
				rc5_index--;
			} else {
				// previous = 0
				m_reset_state_machine();
				return;
			}
		} else {	// delta < THRESHOLD_0
			// Noise -> we are lost... Reset
			m_reset_state_machine();
			return;
		}

		// Check rc5_index consistency
		if (rc5_index < 1) {
			// Oops...
			m_reset_state_machine();
		}
	}
}



