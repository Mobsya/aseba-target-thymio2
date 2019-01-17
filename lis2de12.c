/*
		Thymio-II Firmware

		Copyright (C) 2019 Michael Bonani <michael dot bonani at mobsya dot org>,
		Association Mobsya (http://www.mobsya.ch)

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

#include <i2c/i2c.h>
#include <error/error.h>
#include <clock/clock.h>

#include "lis2de12.h"
#include "regulator.h"

#define LIS2DE12_ID			0x33
#define LIS2DE12_WHO_AM_I	0x0F

#define CTRL_REG1			0x20
#define CTRL_REG2			0x21
#define CTRL_REG3			0x22
#define CTRL_REG4			0x23
#define CTRL_REG5			0x24
#define CTRL_REG6			0x25

#define STATUS_REG			0x27
/*TODO*/
#define XOUT				0x29
#define YOUT				0x2b
#define ZOUT				0x2d

#define FIFO_READ_START		0x28
#define FIFO_CTRL_REG		0x2E
#define FIFO_SRC_REG		0x2F

#define CLICK_CFG			0x38
#define CLICK_SRC			0x39
#define CLICK_THS			0x3A
#define TIME_LIMIT			0x3B
#define TIME_LATENCY		0x3C
#define TIME_WINDOW			0x3D

#define MODE_CONFIG_OFF		0x00
#define MODE_CONFIG_ON		0x0F

static __attribute((far)) int i2c_bus;
static __attribute((far)) int i2c_address;
static __attribute((far)) lis2de12_cb cb;
static __attribute((far)) char data[6];
static __attribute((far)) unsigned char reg;

static void lis2de12_i2c_cb(int i2c_id, bool status) {
	static int relaunched;
	int tap;
	
	tap = 0;//data[3] & 0x20;
	if (reg==ZOUT)
		cb((signed char)data[0], (signed char)data[1], (signed char)data[2], tap);
}

void lis2de12_read_async(void) {
	// Safety: If i2c is busy, ignore this
	if (i2c_master_is_busy(i2c_bus))
		return;

	// Read XOUT, YOUT, ZOUT, status and fire the callback
	reg = XOUT;
	i2c_master_transfert_async(i2c_bus, i2c_address, &reg, 1, (unsigned char *) data, 1, lis2de12_i2c_cb);
	while(i2c_master_is_busy(i2c_bus));
	reg = YOUT;
	i2c_master_transfert_async(i2c_bus, i2c_address, &reg, 1, (unsigned char *) data+1, 1, lis2de12_i2c_cb);
	while(i2c_master_is_busy(i2c_bus));
	reg = ZOUT;
	i2c_master_transfert_async(i2c_bus, i2c_address, &reg, 1, (unsigned char *) data+2, 1, lis2de12_i2c_cb);
}

static void write(unsigned char reg, unsigned char data) {
	unsigned char array[2];
	array[0] = reg;
	array[1] = data;
	i2c_master_transfert_block(i2c_bus, i2c_address,
			array, 2, NULL, 0);
}

int lis2de12_init(int i2c, unsigned char address, lis2de12_cb ucb, int prio) {
	char id;
	i2c_bus = i2c;
	i2c_address = address;
	cb = ucb;

	va_get(); // Enable Va LDO

	clock_delay_us(1300);
	reg=LIS2DE12_WHO_AM_I;
	i2c_master_transfert_block(i2c_bus,i2c_address,&reg,1,&id,1);
	if (id!=LIS2DE12_ID)
		return 0;
	/* Configure device */
	write(CTRL_REG1, MODE_CONFIG_OFF); // Reset
	/*  TODO fix interupt 
	if (prio)
		write(INT_SETUP, 1 << GINT); //Enable auto interrupt on update (GINT)
	else
		write(INT_SETUP, 0);*/

	/* Configure PIC */
	TRISDbits.TRISD7 = 1; // Set RD7 pin as input
	//	CNPU2bits.CN16PUE = 1;			// Enable internal pull-up resistor

	/* Configure interrupts */
	/* TODO fix interupt 
	if (prio) { 
		IPC4bits.CNIP = prio; // CN interrupt priority
		IFS1bits.CNIF = 0; // Clear flag
		CNEN2bits.CN16IE = 1; // Enable CN16 interrupt
		IEC1bits.CNIE = 1; // Enable CN interrupt
	}*/
	return 1;
}

void lis2de12_set_mode(int hz, int tap_en) {
	int flag = IEC1bits.CNIE;

	ERROR_CHECK_RANGE(hz, LIS2DE12_0HZ, LIS2DE12_5376HZ, LIS2DE12_ERROR_INVALID_PARAM);


	IEC1bits.CNIE = 0;

	while (i2c_master_is_busy(i2c_bus));

	// Set the device into standby mode
	write(CTRL_REG1, MODE_CONFIG_OFF);


	// Change
	if (hz == LIS2DE12_0HZ)
		// Stop the device
		return;
	
	// Write the Tap detection register
	// Axe Y and Z, treshold: 12 counts
	if (tap_en){
		//Disable X axis due to the vibration of motors
		write(CTRL_REG2, 0x94); //High pass filter in Normal mode, HPCP 01 and HPCLICK enable
		write(CLICK_CFG, 0x14); //Single click on y and z
	}
	else
		write(CLICK_CFG, 0x00);
	
	//write the tap debounce register
	// 21 counts debouce
	if (tap_en)
		write(CLICK_THS, 20); //Click treshold
	else
		write(CLICK_THS, 0);
	
	hz=hz<<4;
	// Enable the device
	write(CTRL_REG1,(hz | MODE_CONFIG_ON));

	IEC1bits.CNIE = flag;
}

void lis2de12_suspend(void) {
	IEC1bits.CNIE = 0;

	// Disable the device
	write(CTRL_REG1, MODE_CONFIG_OFF);

	va_put(); // disable LDO
}
/*
void _ISR _CNInterrupt(void) {
	// OK
	IFS1bits.CNIF = 0;


	// One interrupt for all CN pins!

	// Check the state of the pin
	if (PORTDbits.RD7 != 1)
		return;

	// Initiate the data transfer
	lis2de12_read_async();

}*/

