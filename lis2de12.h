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

#ifndef _LIS2_DE12_H_
#define _LIS2_DE12_H_

typedef void (*lis2de12_cb)(int x, int y, int z, int tap);

#define LIS2DE12_DEFAULT_ADDRESS	(0x19)

// 0Hz == Power-down mode
#define LIS2DE12_0HZ		0x0
#define LIS2DE12_1HZ		0x1
#define LIS2DE12_10HZ		0x2
#define LIS2DE12_25HZ		0x3
#define LIS2DE12_50HZ		0x4
#define LIS2DE12_100HZ		0x5
#define LIS2DE12_200HZ		0x6
#define LIS2DE12_400HZ		0x7
#define LIS2DE12_1620HZ		0x8
#define LIS2DE12_5376HZ		0x9

/** Errors  LIS2DE12 can throw */
enum lis2de12_errors {
	LIS2DE12_ERROR_BASE = 0x3000,
	LIS2DE12_ERROR_INVALID_PARAM, /**< Invalid parameters. */
};

// Warning, the prio level is the internal interrupt level.
// if prio == 0 no interrupt is used (except for i2c transfert) thus you need to call
// lis2de12_read_async yourself
// The callback will be called with the same priority as the i2c bus interrupt.
int lis2de12_init(int i2c, unsigned char address, lis2de12_cb cb, int prio);
void lis2de12_set_mode(int hz, int tap_en, int fifo);

void lis2de12_read_async(void);
void lis2de12_read_async_fifo(void);

// PM
void lis2de12_suspend(void);

#endif // _LIS2_DE12_H_
