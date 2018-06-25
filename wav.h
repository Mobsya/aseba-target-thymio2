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

#ifndef _WAV_H_
#define _WAV_H_

#include "sd/ff.h"

// If wav file is in correct format:
//   - return 0
//	 - move the read position to the begginig of the data
// If wav file is not in correct format:
//   - return -1, file read position is in unknown state
int wav_init(FIL * file);

// Move back the read position to the beggining of the data
// without checking that the file is valid.
void wav_rewind(FIL * file);

// reserve some space for the header
// -1 if fail, 0 if OK
int wav_create_header(FIL * file);

// Write the wav header. Size is the payload size (== number of sample)
void wav_finalize_header(FIL * file, unsigned long size);

// Read the duration if wav file is in correct format
unsigned long wav_header_read_duration(FIL * file);
#endif
