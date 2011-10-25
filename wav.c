#include "wav.h"

#define RIFF 0x46464952UL
#define WAVE 0x45564157UL
#define FMT 0x20746d66UL
#define DATA 0x61746164UL

static int check_int(FIL * file, unsigned int v) {
	unsigned int t;
	unsigned int read;
	
	f_read(file, &t, 2, &read);
	
	if(read != 2 || t != v)
		return -1;
	return 0;
}

static int check_long(FIL * file, unsigned long v) {
	unsigned long t;
	unsigned int read;
	
	f_read(file, &t, 4, &read);
	
	if(read != 4 || t != v)
		return -1;
	return 0;
}

static int get_long(FIL * file, unsigned long * v) {
	unsigned int read;
	
	f_read(file, v, 4, &read);
	
	if(read != 4)
		return -1;
	return 0;
}	

int wav_init(FIL * file) {
	unsigned long size;
	unsigned long hz;
	
	if(check_long(file, RIFF))
		return -1;
	
	if(get_long(file, &size))
		return -1;
	
	if(check_long(file, WAVE))
		return -1;
	
	if(check_long(file, FMT))
		return -1;
	
	if(check_long(file, 16))
		return -1;
	
	if(check_int(file, 1)) // PCM
		return -1;
	
	if(check_int(file, 1)) // Mono
		return -1;
	
	if(get_long(file, &hz))
		return -1;
	
	if(hz < 7800 || hz > 8000)
		return -1;
	
	if(check_long(file, hz))
		return -1;
	
	if(check_int(file, 1))
		return -1;
	
	if(check_int(file, 8)) // 8 bits
		return -1;
	
	if(check_long(file, DATA))
		return -1;
	
	if(check_long(file, size - 36))
		return -1;
		
	return 0;
}

void wav_rewind(FIL * file) {
	f_lseek(file, 44);
}	
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
