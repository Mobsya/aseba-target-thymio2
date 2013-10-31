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

#include <types/types.h>

#include "sd/diskio.h"
#include "sd/ff.h"
#include "regulator.h"
#include "sound.h"
#include "behavior.h"
#include "wav.h"
#include "abo.h"
#include "playback.h"
#include "log.h"

static __attribute((far)) FATFS fs; // SD fat 
static __attribute((far)) FIL read_file; // Read handle
static __attribute((far)) FIL write_file; // Write handle
static __attribute((far)) FIL user_file; // Read/write file accessible from the VM
static unsigned char l;

#define SD_PRIO 2

// Basic rule: ALL SD ACCESS IS DONE AT IPL == 1 !! 

DWORD get_fattime(void) {
	return ((2011UL - 1980) << 25)
			| ((18UL) << 21) 
			| ((2UL) << 16)
			| (0 << 11)
			| (0 << 5)
			| (0 >> 1);
}



void sd_init(void) {
	va_get(); // For the SD card refcounting not done inside it
	f_mount(0,&fs); // Cannot fail (it's just initialisation)	
}

void sd_shutdown(void) {
	unsigned int flags;
	RAISE_IPL(flags, SD_PRIO);
	
	f_close(&read_file);
	f_close(&write_file);
	f_close(&user_file);
	f_mount(0,0);
	
	_TRISB14 = 1;
	SPI2STATbits.SPIEN = 0;			/* Disable SPI2 */
	
	va_put();	
	
	IRQ_ENABLE(flags);
}

static int sd_play_cb(unsigned char * buffer) {
	unsigned int read;
	unsigned int i;
	f_read(&read_file, buffer, SOUND_OBUFSZ, &read);
	
	// Fill the remaining bytes with middle value
	for(i = read; i < SOUND_OBUFSZ; i++)
		buffer[i] = 127;
		
	// sd buffer size 512, so we want to stay 512 aligned, 
	// So even if we are at the end, we fill the end of the buffer with 0
	// So next read, we stay aligned (loop case only).
	if(read != SOUND_OBUFSZ && l) {
		// If loop and EOF soon
		wav_rewind(&read_file);
		read = SOUND_OBUFSZ;
	}
	
	if(read == 0) {
		behavior_notify_sd(BEHAVIOR_STOP | BEHAVIOR_SD_READ);
		playback_notify_eop();
		return 0; // End of playback	
	} else
		return 1; // Still some data ... 			
}


// return 1 if file found
// return 0 if not found
int sd_play_file(const char * file, int loop) {
	// start to play a file ... 
	unsigned int flags;
	int ret;

	
	RAISE_IPL(flags, SD_PRIO);
	
	f_close(&read_file); // Close the last read file
	if(f_open(&read_file, file, FA_READ) == FR_OK && !wav_init(&read_file)) {
		sound_playback_hold();
		
		l = loop;
		
		sound_playback_enable(sd_play_cb);
		behavior_notify_sd(BEHAVIOR_START | BEHAVIOR_SD_READ);
		
		log_set_flag(LOG_FLAG_PLAYBACKSD);
		
		ret = 1;
	} else {
		behavior_notify_sd(BEHAVIOR_STOP | BEHAVIOR_SD_READ);
		ret = 0;
	}
	
	IRQ_ENABLE(flags);
	
	return ret;
}

static int record;
static unsigned long sample_count;

static void close_write_file(void) {
	record = 0;
	behavior_notify_sd(BEHAVIOR_STOP | BEHAVIOR_SD_WRITE);
	
	// Check if the file is open
	if(!write_file.fs)
		return;
		
	// If we have writtend at least one sample
	if(sample_count)
		wav_finalize_header(&write_file, sample_count);
		
	sample_count = 0;
	f_close(&write_file);
}	

void sound_mic_buffer(unsigned char *b) {
	unsigned int written;
	if(record) {
		f_write(&write_file, b, SOUND_IBUFSZ, &written);
		sample_count += written;
		
		if(written != SOUND_IBUFSZ) 
			close_write_file();
	}
}
void sd_start_record(const char * file) {
	unsigned int flags;
	RAISE_IPL(flags, SD_PRIO);
	// Make sure the file is closed
	close_write_file();
	
	if(!f_open(&write_file, file, FA_WRITE | FA_CREATE_ALWAYS)) {
		sample_count = 0;
		if(!wav_create_header(&write_file)) {	
			log_set_flag(LOG_FLAG_RECORDSD);		
			behavior_notify_sd(BEHAVIOR_START | BEHAVIOR_SD_WRITE);
			record = 1;
		} else 
			f_close(&write_file);
	}
	
	IRQ_ENABLE(flags);
}
void sd_stop_record(void) {
	unsigned int flags;
	RAISE_IPL(flags, SD_PRIO);
	
	close_write_file();
	
	IRQ_ENABLE(flags);
}

void sd_log_file(void) {
	unsigned int flags;
	
	RAISE_IPL(flags,SD_PRIO);
	
	if(f_open(&write_file, "_LOGDUMP.#@!", FA_WRITE | FA_OPEN_EXISTING) == FR_OK){
		log_dump(&write_file);
		f_close(&write_file);
	}
	
	IRQ_ENABLE(flags);
}

int sd_test_file_present(void) {
	unsigned int flags;
	int ret;
	RAISE_IPL(flags,SD_PRIO);
	
	if(f_open(&read_file, "_TESTMOD.#@!", FA_READ) == FR_OK) {
		f_close(&read_file);
		ret = 1;
	} else {
		ret = 0;
	}
	
	IRQ_ENABLE(flags);
	
	return ret;
}

int sd_load_aseba_code(void) {
	unsigned int flags;
	int ret;
	RAISE_IPL(flags, SD_PRIO);
	if(f_open(&read_file, "VMCODE.ABO", FA_READ) == FR_OK) {
		ret = abo_load(&read_file);
		f_close(&read_file);
	} else {
		ret = -1;
	}
	
	IRQ_ENABLE(flags);
	return ret;
}		

int sd_user_open(char * name) {
	int ret = 0;
	unsigned int flags;

	RAISE_IPL(flags, SD_PRIO);
	f_close(&user_file);
	if(name) {
		if(f_open(&user_file, name, FA_READ | FA_WRITE | FA_OPEN_ALWAYS) != FR_OK) {
			ret = -1;
		}
	}
	IRQ_ENABLE(flags);
	return ret;
}

int sd_user_seek(unsigned long offset) {
	int ret;
	unsigned int flags;

	RAISE_IPL(flags, SD_PRIO);
	if(f_lseek(&user_file, offset) == FR_OK)
		ret = 0;
	else
		ret = -1;
	IRQ_ENABLE(flags);
	return ret;
}

unsigned int sd_user_read(unsigned char * data, unsigned int size) {
	unsigned int ret;
	unsigned int flags;
	unsigned int read;
	
	RAISE_IPL(flags, SD_PRIO);
	
	if(f_read(&user_file, data, size, &read) == FR_OK) 
		ret = read;
	else
		ret = 0;

	IRQ_ENABLE(flags);
	return ret;
}

unsigned int sd_user_write(unsigned char * data, unsigned int size) {
	unsigned int ret;
	unsigned int flags;
	unsigned int written;
	RAISE_IPL(flags, SD_PRIO);
	if(f_write(&user_file, data, size, &written) == FR_OK)
		ret = written;
	else
		ret = 0;
	IRQ_ENABLE(flags);
	return ret;
}
