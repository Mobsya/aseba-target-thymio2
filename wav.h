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

#endif
