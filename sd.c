#include <types/types.h>

#include "sd/diskio.h"
#include "sd/ff.h"
#include "regulator.h"
#include "sound.h"
#include "behavior.h"

static FATFS fs; // SD fat 
static FIL read_file; // Read handle
static FIL write_file; // Write handle

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
	
	if(read == 0) {
		behavior_notify_sd(BEHAVIOR_STOP | BEHAVIOR_SD_READ);
		return 0; // End of playback	
	} else
		return 1; // Still some data ... 
}


// return 1 if file found
// return 0 if not found
int sd_play_file(const char * file) {
	// start to play a file ... 
	unsigned int flags;
	int ret;
	
	RAISE_IPL(flags, SD_PRIO);
	
	f_close(&read_file); // Close the last read file
	if(f_open(&read_file, file, FA_READ) == FR_OK) {
		sound_playback_disable();
		sound_playback_enable(sd_play_cb);
		behavior_notify_sd(BEHAVIOR_START | BEHAVIOR_SD_READ);
		ret = 1;
	} else {
		behavior_notify_sd(BEHAVIOR_STOP | BEHAVIOR_SD_READ);
		ret = 0;
	}
	
	IRQ_ENABLE(flags);
	
	return ret;
}

static int record;

void sound_mic_buffer(unsigned char *b) {
	unsigned int written;
	if(record) {
		f_write(&write_file, b, SOUND_IBUFSZ, &written);
		if(written != SOUND_IBUFSZ) {
			record = 0;
			behavior_notify_sd(BEHAVIOR_STOP | BEHAVIOR_SD_WRITE);
			f_close(&write_file);
		}
	}
}
void sd_start_record(const char * file) {
	unsigned int flags;
	RAISE_IPL(flags, SD_PRIO);
	
	// Make sure the file is closed
	f_close(&write_file);
	
	if(!f_open(&write_file, file, FA_WRITE | FA_CREATE_ALWAYS)) {
		behavior_notify_sd(BEHAVIOR_START | BEHAVIOR_SD_WRITE);
		record = 1;
	}
	
	IRQ_ENABLE(flags);
}
void sd_stop_record(void) {
	unsigned int flags;
	RAISE_IPL(flags, SD_PRIO);
	
	record = 0;
	behavior_notify_sd(BEHAVIOR_STOP | BEHAVIOR_SD_WRITE);
	f_close(&write_file);
	
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

