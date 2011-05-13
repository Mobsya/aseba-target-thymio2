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

#ifndef _RC5_H
#define _RC5_H


typedef void (*rc5_cb)(unsigned int address, unsigned int command);

enum rc5_errors
{
	RC5_ERROR_BASE = 0x1300,
	RC5_ERROR_IMPOSSIBLE_STATE,			/**< Two RC5 cycles have elapsed while the previous bit was a 0. */
};

void rc5_init(unsigned int timer, rc5_cb ucb, int priority);
void rc5_shutdown(void);

void rc5_disable(void);
void rc5_enable(void);

extern unsigned char rc5_valid_flag;

#endif // _RC5_H

