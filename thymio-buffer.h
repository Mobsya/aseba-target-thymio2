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

#ifndef _THYMIO_BUFFER_H_
#define _THYMIO_BUFFER_H_

#include <vm/vm.h>
#include <common/types.h>


/************** 
 * Interface USB-HW (Interrupt) <-> Usb buffer 
 **************/

/** The MTU of the underling hardware */
#define ASEBA_USB_MTU	64

/** callback from the usb layer asking for more data
 * put the datas in the pointer and return the size written to send more data
 * return 0 to send nothing
 */
unsigned char AsebaTxReady(unsigned char *data);

/** callback from the usb layer, data is a pointer to the data and size is the size .... 
 * Return true if the data where consumed, false if not.
 */
int AsebaUsbBulkRecv(unsigned char *data, unsigned char size);



/*************
 * Interface Fifos <-> Aseba VM (Main() code)
 *************/

void AsebaSendBuffer(AsebaVMState *vm, const uint8_t * data, uint16_t length);
uint16_t AsebaGetBuffer(AsebaVMState *vm, uint8_t* data, uint16_t maxLength, uint16_t* source);

/*************
 * Interface Fifos <-> RF part
 *************/
void AsebaFifoPushToRx(unsigned char c);
int AsebaFifoRxFull(void);
unsigned char AsebaFifoTxPop(void);
unsigned char AsebaFifoTxPeek(void);
int AsebaFifoTxEmpty(void);
 

/*************
 * Init, and other random stuff 
 *************/
void AsebaFifoInit(unsigned char * sendQueue, size_t sendQueueSize, unsigned char * recvQueue, size_t recvQueueSize);
int AsebaFifoRecvBufferEmpty(void);
int AsebaFifoTxBusy(void);

/*@}*/

#endif
