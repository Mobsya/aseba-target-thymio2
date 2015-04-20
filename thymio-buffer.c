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
#include "rf.h"
#include <types/types.h>
#include <usb/usb.h>
#include <usb/usb_device.h>
#include "usb_function_cdc.h"
#include "thymio-buffer.h"
#include "usb_uart.h"

struct fifo {
	unsigned char * buffer;
	size_t size;
	size_t insert;
	size_t consume;
};

static struct {
	struct fifo rx;
	struct fifo tx;
} AsebaFifo;

// No communication bus presents
#define MODE_DISCONNECTED	0x2
// USB is connected _AND_ running (DTE bit set)
#define MODE_USB		0x1
// RF link is present
#define MODE_RF			0x0
static unsigned char connection_mode;

// We have a delay on reconnection
#define RECONNECTION_DELAY 100
unsigned int reconnection_delay = 0;

/* Basic assumption in order to protect concurrent access to the fifos:
	- If the code in "main()" access the fifo it need to disable the interrupts 
*/

static inline size_t get_used(struct fifo * f) {
	size_t ipos, cpos;
	ipos = f->insert;
	cpos = f->consume;
	if (ipos >= cpos)
		return ipos - cpos;
	else
		return f->size - cpos + ipos;
}

static inline size_t get_free(struct fifo * f) {
	return f->size - 1 - get_used(f);
}

/* you MUST ensure that you pass correct size to thoses two function, 
 * no check are done .... 
 */
static inline void memcpy_out_fifo(unsigned char * dest, struct fifo * f, size_t size) {
	while(size--) {
		*dest++ = f->buffer[f->consume++];
		if(f->consume == f->size)
			f->consume = 0;
	}
}

static inline void memcpy_to_fifo(struct fifo * f, const unsigned char * src, size_t size) {
	while(size--) {
		f->buffer[f->insert++] = *src++;
		if(f->insert == f->size)
			f->insert = 0;
	}
}

static inline void fifo_peek(unsigned char * d, struct fifo * f,size_t size) {
	int ct = f->consume;
	while(size--) {
		*d++ = f->buffer[ct++];
		if(ct == f->size)
			ct = 0;
	}
}	

static inline void fifo_reset(struct fifo * f) {
	f->insert = f->consume = 0;
}

void AsebaFifoPushToRx(unsigned char c) {
	memcpy_to_fifo(&AsebaFifo.rx, &c, 1);
}	

int AsebaFifoRxFull(void) {
	return !get_free(&AsebaFifo.rx);
}

unsigned char AsebaFifoTxPop(void) {
	unsigned char c;
	memcpy_out_fifo(&c,&AsebaFifo.tx,1);
	return c;
}
unsigned char AsebaFifoTxPeek(void) {
	unsigned char c;
	fifo_peek(&c, &AsebaFifo.tx, 1);
	return c;
}	

int AsebaFifoTxEmpty(void) {
	return !get_used(&AsebaFifo.tx);
}	 

void AsebaFifoCheckConnectionMode(void) {
	if(usb_uart_serial_port_open()) {
		if(connection_mode == MODE_USB)
			return; // Nothing to do ...

		// Put the RF link down if it was up
		if(rf_get_status() & RF_LINK_UP) 
			rf_set_link(RF_DOWN);

		// We are switching to usb, reset the fifo and make the switch
		fifo_reset(&AsebaFifo.tx);
		fifo_reset(&AsebaFifo.rx);
		connection_mode = MODE_USB;
		reconnection_delay = RECONNECTION_DELAY;             
		return;
	}

	// No usb, so try RF.
	if(rf_get_status() & RF_PRESENT) {
		if(connection_mode == MODE_RF)
			return; // Nothing to do
		
		fifo_reset(&AsebaFifo.tx);
		fifo_reset(&AsebaFifo.rx);

		// We are switching *from* usb, start the RF link
		if(!(rf_get_status() & RF_LINK_UP))
			rf_set_link(RF_UP);
		
		connection_mode = MODE_RF;
		return;
	}

	// No RF, No usb ... 
	fifo_reset(&AsebaFifo.tx);
	fifo_reset(&AsebaFifo.rx);
	
	connection_mode = MODE_DISCONNECTED;
}

/* USB interrupt part */
// They can be called from the main, but with usb interrupt disabled, so it's OK
static int tx_busy;
static int debug;

unsigned char AsebaTxReady(unsigned char *data) {
	size_t size = get_used(&AsebaFifo.tx);
	
	// Do not send anything on usb if we are not in usb mode
	if(size == 0 || connection_mode != MODE_USB || reconnection_delay > 0) {
		tx_busy = 0;
		debug = 0;
		return 0;
	}
	
	if(size > ASEBA_USB_MTU)
		size = ASEBA_USB_MTU;
	
	memcpy_out_fifo(data, &AsebaFifo.tx, size);
	debug ++;
	return size;
}

int AsebaUsbBulkRecv(unsigned char *data, unsigned char size) {
	// Ignore all data if we are not in usb mode
	AsebaFifoCheckConnectionMode();
	
	if(connection_mode != MODE_USB)
		return 0;
	
	// we have received a packet, meaning that the computer is connected, so reset reconnection delay
	reconnection_delay = 0;
	
	size_t free = get_free(&AsebaFifo.rx);
	
	if(size > free)
		return 1;
	
	memcpy_to_fifo(&AsebaFifo.rx, data, size);
	
	return 0;
}

/* RF Part */


/* main() part */

void AsebaSendBuffer(AsebaVMState *vm, const uint8 *data, uint16 length) {
	int flags;
	unsigned char mode = connection_mode;


	barrier(); // Force the compiler to capture mode 


	// Here we must loop until we can send the data.
	// BUT if we are disconnected we simply drop the data.
	if(mode == MODE_DISCONNECTED)
		return;
	
	// Sanity check, should never be true
	if (length < 2)
		return;
	
	do {
		RAISE_IPL(flags, PRIO_COMMUNICATION);

		if(mode != connection_mode) {
			// the connection medium changed under our feet, let's drop this packet
			// No need to reset the fifo it has already been done.
			IRQ_ENABLE(flags);
			break;
		}

		if(get_free(&AsebaFifo.tx) >= length + 4) {
			length -= 2;
			memcpy_to_fifo(&AsebaFifo.tx, (unsigned char *) &length, 2);
			memcpy_to_fifo(&AsebaFifo.tx, (unsigned char *) &vm->nodeId, 2);
			memcpy_to_fifo(&AsebaFifo.tx, (unsigned char *) data, length + 2);
			
			// Will callback AsebaUsbTxReady
			if(mode == MODE_USB) {
				if (!tx_busy) {
					tx_busy = 1;
					USBCDCKickTx();
				}
			}
			
			length = 0;
		}
		
		IRQ_ENABLE(flags);
	} while(length);
}

uint16 AsebaGetBuffer(AsebaVMState *vm, uint8 * data, uint16 maxLength, uint16* source) {
	int flags;
	uint16 ret = 0;
	size_t u;
	// Touching the FIFO, mask the interrupt ...
	RAISE_IPL(flags, PRIO_COMMUNICATION);
	
	AsebaFifoCheckConnectionMode();

	u = get_used(&AsebaFifo.rx);

	/* Minium packet size == len + src + msg_type == 6 bytes */
	if(u >= 6) {
		int len;
		fifo_peek((unsigned char *) &len, &AsebaFifo.rx, 2);
		
		if (u >= len + 6) {
			memcpy_out_fifo((unsigned char *) &len, &AsebaFifo.rx, 2);
			memcpy_out_fifo((unsigned char *) source, &AsebaFifo.rx, 2);
	
			// msg_type is not in the len but is always present
			len = len + 2;
			/* Yay ! We have a complete packet ! */
			if(len > maxLength)
				len = maxLength;

			memcpy_out_fifo(data, &AsebaFifo.rx, len);
			ret = len;
		}
	}	
	if(connection_mode == MODE_USB)
		USBCDCKickRx();

	IRQ_ENABLE(flags);
	return ret;
}

void AsebaFifoInit(unsigned char * sendQueue, size_t sendQueueSize, unsigned char * recvQueue, size_t recvQueueSize) {
	AsebaFifo.tx.buffer = sendQueue;
	AsebaFifo.tx.size = sendQueueSize;
	
	AsebaFifo.rx.buffer = recvQueue;
	AsebaFifo.rx.size = recvQueueSize;
}

int AsebaFifoRecvBufferEmpty(void) {
	// We are called with interrupt disabled ! Check if rx contain something meaningfull
	
	int u;
	
	u = get_used(&AsebaFifo.rx);
	if(u > 6) {
		int len;
		fifo_peek((unsigned char *) &len, &AsebaFifo.rx, 2);
		if (u >= len + 6) 
			return 0;
	}
	return 1;
}

int AsebaFifoTxBusy(void) {
	return tx_busy;
}
