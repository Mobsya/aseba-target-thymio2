#ifndef _CRC_H_
#define _CRC_H_

void crc_init(unsigned int polylen, unsigned int poly, unsigned int initial);

void crc_process_8(const void * data, unsigned int size);
void crc_process_16(const unsigned int * data, unsigned int count);
unsigned int crc_finish(void);


#endif
