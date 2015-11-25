/*
        Thymio-II Firmware

        Copyright (C) 2013 Philippe Retornaz <philippe dot retornaz at epfl dot ch>,
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

#include <skel-usb.h>
#include <types/types.h>
#include <i2c/i2c.h>
#include <clock/clock.h>
#include "thymio-buffer.h"
#include "mma7660.h"
#include "rf.h"

#define RF_ADDRESS 0x42

#define REG_FIFO		0xFF
#define REG_FIFO_LEN 	0xFD
#define REG_STATUS		0xFC
#define REG_VERSION		0xFB
#define REG_POWEROFF	0x7F
#define REG_FLASH_SETTINGS	0x7E
#define REG_PAIRING		0x06
#define REG_PANID_H		0x05
#define REG_PANID_L		0x04
#define REG_NODEID_H	0x03
#define REG_NODEID_L	0x02
#define REG_CTRL 		0x01


static unsigned int i2c_bus;
static unsigned char status;
static unsigned char read_acc;

static void write(unsigned char reg, unsigned char b) {
	unsigned char buffer[2] = {reg,b};
	unsigned int flags;
	RAISE_IPL(flags, PRIO_COMMUNICATION);
	while(i2c_master_is_busy(i2c_bus));
	i2c_master_transfert_block(i2c_bus, RF_ADDRESS, buffer, 2, 0, 0);
	IRQ_ENABLE(flags);
}

static void read(unsigned char reg, unsigned char * b, unsigned int len) {
	unsigned int flags;
	RAISE_IPL(flags, PRIO_COMMUNICATION);
	while(i2c_master_is_busy(i2c_bus));
	i2c_master_transfert_block(i2c_bus, RF_ADDRESS,	&reg, 1, b, len);
	IRQ_ENABLE(flags);
}		

unsigned int rf_get_status(void) {
	return status;
}	

unsigned int rf_get_node_id(void) {
	unsigned int id = 1;
	read(REG_NODEID_L, (unsigned char *) &id, sizeof(id));
	return id;
}

void rf_set_node_id(unsigned int id) {
	write(REG_NODEID_L, (unsigned char) id&0xFF);
	write(REG_NODEID_H, (unsigned char) (id>>8));
}

void rf_init(int bus) {
	unsigned int flags;
	unsigned char reg = REG_STATUS;
	unsigned char s;
	unsigned char ret;
	
	clock_delay_us(5000); // we need to wait until the chip is ready ....
	
	RAISE_IPL(flags, PRIO_COMMUNICATION);
	i2c_bus = bus;
	// Wait until the bus is idle
	while(i2c_master_is_busy(i2c_bus));
	
	// Try to access the chip
	ret = i2c_master_transfert_block(i2c_bus, RF_ADDRESS, &reg, 1, &s, 1);
	
	if(!ret) {
		// FIXME
		// Try harder ...
		ret = i2c_master_transfert_block(i2c_bus, RF_ADDRESS, &reg, 1, &s, 1);
	}
	
	IRQ_ENABLE(flags);
	
	if (ret) {
		write(REG_CTRL, 3); // Put us in presence detect only
		status = RF_PRESENT | RF_LINK_UP;
	} else {
		status = 0;
	}	
}	

void rf_set_link(unsigned int link_status) {
	if(!(status & RF_PRESENT))
		return;
		
// FIXME Raise IPL ?
	if(link_status == RF_DOWN) {
		write(REG_CTRL, 0);
		status &= ~(RF_LINK_UP | RF_DATA_RX);
		return;
	}	
	if(link_status == RF_UP) {
		write(REG_CTRL, 1); 
		status &= ~RF_DATA_RX;
		status |= RF_LINK_UP;
		return;
	}
	
	if(link_status == RF_PRESENCE_ONLY) {
		write(REG_CTRL, 3);
		status &= ~RF_DATA_RX;
		status |= RF_LINK_UP;
		return;
	}
}

/* To test module interface, not planed to give this function to user
void rf_set_channel(unsigned char channel)
{
	if(!(status & RF_PRESENT))
		return;
	
	if(channel<3){//channel can be 0,1,2
		if (status & RF_LINK_UP)
			write(REG_CTRL,((channel + 1)<<3)+2);
		else
			write(REG_CTRL,(channel + 1)<<3);
	}
}
*/


#define I2C_IDLE 			0
#define I2C_ADDRESS 		1
#define I2C_REG_L			2
#define I2C_RESTART			3
#define I2C_ADDRESS2		4
#define I2C_READ_L1			5
#define I2C_READ_L1_ACK		6
#define I2C_READ_L2			7
#define I2C_READ_L2_ACK 	8
#define I2C_READ_FIFO		9
#define I2C_READ_FIFO_ACK	10
#define I2C_READ_FIFO_NACK	11
#define I2C_RESTART2		12
#define I2C_ADDRESS3		13
#define I2C_REG_F			14
#define I2C_DUMMY_W			15
#define I2C_WRITE_FIFO		16
#define I2C_STOP2			17
static unsigned char i2c_status;
static unsigned int fifo_read_len;
static unsigned char i2c_data;

int i2c_cb(int i2c_id, unsigned char ** data, void * user, bool nack) {
	switch(i2c_status) {
	case I2C_IDLE:
		i2c_data = RF_ADDRESS << 1;
		*data = &i2c_data;
		i2c_status++;
		return I2C_MASTER_WRITE;
		
	case I2C_ADDRESS:
		if(nack) {
			i2c_status = I2C_STOP2;
			return I2C_MASTER_STOP;
		}
		i2c_status = I2C_REG_L;
		i2c_data = REG_FIFO_LEN;
		*data = &i2c_data;
		return I2C_MASTER_WRITE;
		
	case I2C_REG_L:
		if(nack) {
			i2c_status = I2C_STOP2;
			return I2C_MASTER_STOP;
		}
		i2c_status = I2C_RESTART;
		return I2C_MASTER_RESTART;
		
	case I2C_RESTART:
		i2c_data = RF_ADDRESS << 1 | 0x1;
		*data = &i2c_data;
		i2c_status = I2C_ADDRESS2;
		return I2C_MASTER_WRITE;
		
	case I2C_ADDRESS2:
		if(nack){
			i2c_status = I2C_STOP2;
			return I2C_MASTER_STOP;
		}
		i2c_status = I2C_READ_L1;
		*data = &i2c_data;
		return I2C_MASTER_READ;
		
	case I2C_READ_L1:
		i2c_status = I2C_READ_L1_ACK;
		fifo_read_len = i2c_data;
		return I2C_MASTER_ACK;
		
	case I2C_READ_L2:
		fifo_read_len |= i2c_data << 8;
		
		if(fifo_read_len && !AsebaFifoRxFull()) {
			i2c_status = I2C_READ_L2_ACK;
			return I2C_MASTER_ACK;
		} else {
			i2c_status = I2C_READ_FIFO_NACK;
			return I2C_MASTER_NACK;
		}		
	case I2C_READ_L1_ACK:
		i2c_status = I2C_READ_L2;
		*data = &i2c_data;
		return I2C_MASTER_READ;
		
	case I2C_READ_FIFO_ACK:
	case I2C_READ_L2_ACK:
		i2c_status = I2C_READ_FIFO;
		*data = &i2c_data;
		return I2C_MASTER_READ;
		
	case I2C_READ_FIFO:
		AsebaFifoPushToRx(i2c_data);
		status |= RF_DATA_RX;
		fifo_read_len--;
		if(fifo_read_len && !AsebaFifoRxFull()) {
			i2c_status = I2C_READ_FIFO_ACK;
			return I2C_MASTER_ACK;
		} else {
			i2c_status = I2C_READ_FIFO_NACK;
			return I2C_MASTER_NACK;
		}
	case I2C_READ_FIFO_NACK:
		// Check if we should write something.
		if(!AsebaFifoTxEmpty()) {
			i2c_status = I2C_RESTART2;
			return I2C_MASTER_RESTART;
			
		} else {
			// Nothing to send. End of communication.
			i2c_status = I2C_STOP2;
			return I2C_MASTER_STOP;
		}	
	case I2C_RESTART2:
		i2c_data = RF_ADDRESS << 1;
		*data = &i2c_data;
		i2c_status = I2C_ADDRESS3;
		return I2C_MASTER_WRITE;
	case I2C_ADDRESS3:
		if(nack) {
			i2c_status = I2C_STOP2;
			return I2C_MASTER_STOP;
		}
		*data = &i2c_data;
		i2c_data = REG_FIFO;
		i2c_status = I2C_REG_F;
		return I2C_MASTER_WRITE;
	case I2C_REG_F:
		if(nack || AsebaFifoTxEmpty()) {
			i2c_status = I2C_STOP2;
			return I2C_MASTER_STOP;
		} 
		i2c_status = I2C_WRITE_FIFO;
		i2c_data = AsebaFifoTxPeek();
		*data = &i2c_data;
		return I2C_MASTER_WRITE;
			
	case I2C_WRITE_FIFO:
		if(!nack) // The device accepted our write.
			AsebaFifoTxPop();

		if(nack || AsebaFifoTxEmpty()) {
			// RF fifo full ... or our fifo is empty.
			// In both case we have to stop writing.
			i2c_status = I2C_STOP2;
			return I2C_MASTER_STOP;
		} 
		// try a next write
		i2c_status = I2C_WRITE_FIFO;
		i2c_data = AsebaFifoTxPeek();
		*data = &i2c_data;
		return I2C_MASTER_WRITE;
		
	case I2C_STOP2:
		i2c_status = I2C_IDLE;
		if(read_acc) {
			read_acc = 0;
			// Read async will immediatly start a transfert.
			// We must reset the state machine first.
			i2c_master_reset(i2c_id);
			mma7660_read_async();
			return I2C_MASTER_QUIT;
		} else
			return I2C_MASTER_DONE;
	}
	// Never reached
	return I2C_MASTER_QUIT;	
}	

void rf_poll(void) {
	unsigned int flags;
	
	RAISE_IPL(flags, PRIO_COMMUNICATION);

	if(!(status & RF_PRESENT) || !(status & RF_LINK_UP)) {
		// RF not present or activated
		if(read_acc) {
			read_acc = 0;
			mma7660_read_async();
		}
	} else if(!i2c_master_is_busy(i2c_bus)) {
		// Try to read data from the device
		i2c_master_start_operations(i2c_bus, i2c_cb, NULL);
	} 	
		
	IRQ_ENABLE(flags);	
}	

void rf_schedule_acc_read(void) {
	read_acc = 1;
}

void rf_pairing_start(void) {
	status |= RF_PAIRING_MODE;
	write(REG_PAIRING, 0x1); 
}

void rf_pairing_stop(void) {
	status &= ~RF_PAIRING_MODE;
	write(REG_PAIRING, 0x0); 
}		

void rf_wakeup(void) {
	// Needs to be called *BEFORE* I2C init
	_LATE6 = 0;
	_TRISE6 = 0;
	Nop();
	Nop();
	Nop();
	_TRISE6 = 1;
}	

void rf_poweroff(void) {
	write(REG_POWEROFF, /* whatever */ 0x0);
}

void rf_flash_setting(void) {
	write(REG_FLASH_SETTINGS, /* whatever */ 0x0);
}

