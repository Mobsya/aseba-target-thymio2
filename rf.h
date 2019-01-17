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

#ifndef _RF_H_
#define _RF_H_

void rf_init(int bus);

/* Return the Aseba node ID of the RF part */
unsigned int rf_get_node_id(void);
/* Set the Aseba node ID of the RF part */
void rf_set_node_id(unsigned int id);
/* Set the configuration of the RF part */
void rf_set_conf(unsigned int channel,unsigned int panid);
/* Return the network ID of the RF part */
unsigned int rf_get_network_id(void);

/* Poll the rf status, should be called at the I2C access priority level */
void rf_poll(void);

// Module is there ... 
#define RF_PRESENT 		(1 << 0)
// Module is enable and forward all messages
#define RF_LINK_UP		(1 << 1)
// Module has detected a PC beacon
#define RF_PC_PRESENT		(1 << 2)
// Module has detected a node beacon
#define RF_NEIGHBOR_PRESENT	(1 << 3)
// Module has got some data
#define RF_DATA_RX		(1 << 4)
// Module is in pairing mode 
#define RF_PAIRING_MODE (1 << 5)

unsigned int rf_get_status(void);

#define RF_DOWN 		0x0
#define RF_UP			0x1
// Listen to presence msg only. 
#define RF_PRESENCE_ONLY	0x2
void rf_set_link(unsigned int link_status);


void rf_schedule_acc_read(int acc_type);
void rf_poweroff(void);
void rf_flash_setting(void);

// Do some OOB on SCL to wakeup the CC2533.
void rf_wakeup(void);

void rf_pairing_start();
void rf_pairing_stop();

#endif
