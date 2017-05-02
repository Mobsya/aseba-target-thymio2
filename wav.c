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

#include <string.h>
#include "wav.h"

#define RIFF 0x46464952UL
#define WAVE 0x45564157UL
#define FMT 0x20746d66UL
#define DATA 0x61746164UL

static const unsigned char wav_header[] = {
	0x52, 0x49, 0x46, 0x46, // "RIFF"
	0x00, 0x00, 0x00, 0x00, // Size
	0x57, 0x41, 0x56, 0x45, // "WAVE"
	0x66, 0x6d, 0x74, 0x20, // "fmt "
	0x10, 0x00, 0x00, 0x00, // 
	0x01, 0x00, 0x01, 0x00, // Mono
	0x84, 0x1e, 0x00, 0x00, // 7812Hz
	0x84, 0x1e, 0x00, 0x00, // 7812byte/s
	0x01, 0x00, 0x08, 0x00, // 8bits
	0x64, 0x61, 0x74, 0x61, // "data"
	0x00, 0x00, 0x00, 0x00  // Size
};

// Cannot put 44 byte on the stack, it's too much
static unsigned long __attribute__((far)) read_buffer[11];

int wav_init(FIL * file) {
	unsigned int read;
	
	f_read(file, read_buffer, sizeof(read_buffer), &read);
	if(read != sizeof(read_buffer)) 
		return -1;
	
	if(read_buffer[1] - 36 != read_buffer[10])
		return -1;
	
	if(read_buffer[6] != read_buffer[7])
		return -1;
		
	if(read_buffer[6] < 7800 || read_buffer[6] > 8000)
		return -1;
		
	read_buffer[6] = read_buffer[7] = 0x1e84;
	read_buffer[1] = read_buffer[10] = 0;
	
	if(memcmp(read_buffer,wav_header, sizeof(read_buffer)))
		return -1;
		
	return 0;
}

void wav_rewind(FIL * file) {
	f_lseek(file, sizeof(wav_header));
}	

int wav_create_header(FIL * file) {
	unsigned int ret;
	
	f_lseek(file, 0);
	
	f_write(file, wav_header, sizeof(wav_header), &ret);
	if(ret != sizeof(wav_header))
		return -1;
	return 0;
}	

void wav_finalize_header(FIL * file, unsigned long size) {
	unsigned int ret;
	unsigned long t = size + 36;
	
	if(f_lseek(file, 4) != FR_OK)
		return;
	
	f_write(file, &t, 4, &ret);
	if(ret != 4)
		return;
	
	if(f_lseek(file, 40) != FR_OK)
		return;
		
	f_write(file, &size, 4, &ret);
}

unsigned long wav_header_read_duration(FIL * file) {
	unsigned int read;
	//sanity checks about the integrity of the wave file
	f_read(file, read_buffer, sizeof(read_buffer), &read);
	if(read != sizeof(read_buffer)) 
		return 0;
	
	if(read_buffer[1] - 36 != read_buffer[10])
		return 0;
	
	if(read_buffer[6] != read_buffer[7])
		return 0;
		
	if(read_buffer[6] < 7800 || read_buffer[6] > 8000)
		return 0;
				
	return read_buffer[10];
}
