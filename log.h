/*
        Thymio-II Firmware

        Copyright (C) 2012 Philippe Retornaz <philippe dot retornaz at epfl dot ch>,
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

#ifndef _LOG_H_
#define _LOG_H_

// To be called at 1Khz when in "on" mode
void log_poweron_tick(void);

// To be called by the poweroff code
void log_poweroff_tick(void);

void log_prepare_reset(void);

void log_dump(void * _f);

void log_init(void);

#define LOG_FLAG_INTERNAL		(0) // always not set
#define LOG_FLAG_BATTERY 		(1)
#define LOG_FLAG_ASEBABUG 		(2)
#define LOG_FLAG_VMCODESD		(3)
#define LOG_FLAG_PLAYBACKSD		(4)
#define LOG_FLAG_FLASHVMCODE	(5)
#define LOG_FLAG_RECORDSD		(6)
#define LOG_FLAG_MOTORUSED		(7)
#define LOG_FLAG_IRUSED			(8)
#define LOG_FLAG_NTCUSED		(9)
#define LOG_FLAG_SOUND			(10)
#define LOG_FLAG_LEDIR			(11)
#define LOG_FLAG_LEDRGB			(12)
#define LOG_FLAG_LEDCIRCLE		(13)
#define LOG_FLAG_ACCUSED		(14)
#define LOG_FLAG_BUTTONUSED		(15)
#define LOG_FLAG_SOUNDTRESH		(16)
#define LOG_FLAG_RC5USED		(17)
#define LOG_FLAG_LEDBUTTON		(18)
#define LOG_FLAG_LEDOTHER		(19)
#define LOG_FLAG_EVENTNTC		(20)
#define LOG_FLAG_EVENTTIMER		(21)

void log_set_flag(unsigned char flag);

void log_analyse_bytecode(void);

#endif
