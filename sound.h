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

#ifndef _SOUND_H_
#define _SOUND_H_

#define SOUND_IBUFSZ 128

#define SOUND_OBUFSZ 128


typedef int (*sound_cb)(unsigned char * buffer);

void sound_new_sample(unsigned int sample);

void sound_ignore_next_sample(void);

void sound_init(void);
void sound_poweroff(void);

void sound_playback_enable(sound_cb cb);
void sound_playback_disable(void);
void sound_playback_hold(void);

void sound_set_mic_stat(int enabled);

//callback 
void sound_mic_buffer(unsigned char * b);

#endif

