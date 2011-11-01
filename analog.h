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
// It also check USB voltage and wake up if we get plugged to a computer
void __attribute__((noreturn)) analog_enter_poweroff_mode(void);

// Disable the analog sampling
void analog_disable(void);

#endif
