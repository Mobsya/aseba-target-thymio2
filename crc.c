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

#include <p24Fxxxx.h>

#include "crc.h"

void crc_init(unsigned int polylen, unsigned int poly, unsigned int initial) {
	CRCCONbits.CRCGO = 0;
	
	CRCXOR = poly;
	CRCCON = polylen - 1;
	CRCWDAT = initial;
}	

static void crc_step(void) {
	CRCCONbits.CRCGO = 1;
	while(!CRCCONbits.CRCMPT);
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	CRCCONbits.CRCGO = 0;
}

void crc_process_8(const void * data, unsigned int size) {
	const unsigned char * p = (const unsigned char *) data;
	while(size--) {
		if(CRCCONbits.CRCFUL)
			crc_step();
		*((volatile unsigned char *) &CRCDAT) = *p++;
	};	
}

void crc_process_16(const unsigned int * data, unsigned int count) {
	while(count--) {
		if(CRCCONbits.CRCFUL)
			crc_step();
		CRCDAT = *data++;
	};
}	

unsigned int crc_finish(void) {

	if(CRCCONbits.CRCFUL)
		crc_step();

	if(CRCCONbits.PLEN > 8-1)
		 CRCDAT = 0x0;
	else
		*((volatile unsigned char *) &CRCDAT) = 0x0;
	crc_step();
	
	Nop();
	Nop();
	
	return CRCWDAT;
}	

