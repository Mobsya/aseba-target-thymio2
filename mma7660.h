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

#ifndef _MMA_7660_H_
#define _MMA_7660_H_

typedef void (*mma7660_cb)(int x, int y, int z, int tap);

#define MMA7660_DEFAULT_ADDRESS	(0x4c)

// 0Hz == disabled
#define MMA7660_120HZ		0x0
#define MMA7660_64HZ		0x1
#define MMA7660_32HZ		0x2
#define MMA7660_16HZ		0x3
#define MMA7660_8HZ		0x4
#define MMA7660_4HZ		0x5
#define MMA7660_2HZ		0x6
#define MMA7660_1HZ		0x7
#define MMA7660_0HZ		0x8

/** Errors MMA7660 can throw */
enum mma7660_errors
{
	MMA7660_ERROR_BASE = 0x2000,
	MMA7660_ERROR_INVALID_PARAM,	/**< Invalid parameters. */
};

// Warning, the prio level is the internal interrupt level.
// if prio == 0 no interrupt is used (except for i2c transfert) thus you need to call
// mma7660_read_async yourself
// The callback will be called with the same priority as the i2c bus interrupt.
void mma7660_init(int i2c, unsigned char address, mma7660_cb cb, int prio);
void mma7660_set_mode(int hz, int tap_en);

void mma7660_read_async(void);

// PM
void mma7660_suspend(void);

#endif // _MMA_7660_H
