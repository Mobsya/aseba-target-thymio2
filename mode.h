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

#ifndef _MODE_H_
#define _MODE_H_

void mode_init(int vm_enabled);

enum mode {
	MODE_MENU = 0, // It's also the entry point for the user mode
	MODE_FOLLOW,
	MODE_EXPLORER,
	MODE_ACC,
	MODE_DRAW,
	MODE_SOUND,
	MODE_LINE,
	MODE_RC5,
	MODE_SIDE,
	MODE_MAX = MODE_SIDE,
};

// Return the current mode.
// -1 mean we are in VM mode
int mode_get(void);

// Mode is in fact a behavior, but a special one 
// Which manage other behavior. So it has it's own tick 
void mode_tick(void);

int pulse_get(void);

#endif
