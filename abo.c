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

#include "abo.h"
#include "crc.h"
#include <transport/buffer/vm-buffer.h>
#include <common/consts.h>
#include <skel-usb.h>

#include <string.h>

#define ABO_VERSION 0
#define CRC_POLY 0x1021
#define CRC_SIZE 16

static void add_crc_string(const char * s) {
	unsigned long len = strlen(s);
	if(len & 0x1)
		len++;
	crc_process_8(s, len);
}

static unsigned int vmdesc_crc(void) {
	unsigned int i;
	unsigned int j;
	const AsebaVMDescription *vmDescription = AsebaGetVMDescription(&vmState);
	const AsebaVariableDescription* namedVariables = vmDescription->variables;
	const AsebaLocalEventDescription* localEvents = AsebaGetLocalEventsDescriptions(&vmState);
	const AsebaNativeFunctionDescription* const * nativeFunctionsDescription = AsebaGetNativeFunctionsDescriptions(&vmState);

	crc_init(CRC_SIZE, CRC_POLY, 0);
	
	crc_process_8(&vmState.bytecodeSize, 2);
	crc_process_8(&vmState.variablesSize, 2);
	crc_process_8(&vmState.stackSize, 2);
	
	for(i = 0; namedVariables[i].name; i++) {
		crc_process_8(&namedVariables[i].size, 2);
		add_crc_string(namedVariables[i].name);
	}
	
	for(i = 0; localEvents[i].name; i++)
		add_crc_string(localEvents[i].name);
	
	for(i = 0; nativeFunctionsDescription[i]; i++) {
		add_crc_string(nativeFunctionsDescription[i]->name);
		for(j = 0; nativeFunctionsDescription[i]->arguments[j].size; j++) {
			crc_process_8(&nativeFunctionsDescription[i]->arguments[j].size, 2);
			add_crc_string(nativeFunctionsDescription[i]->arguments[j].name);
		}
	}
	return crc_finish();
}	

static unsigned int do_crc(const void * data, unsigned int size) {
	unsigned char zero = 0;
	crc_init(CRC_SIZE, CRC_POLY, 0);
	crc_process_8(data, size);
	
	// Make sure we are aligned on 16bits
	if(size & 0x1)
		crc_process_8(&zero, 1);
	
	return crc_finish();
}

static unsigned int nodename_crc(void) {
	const AsebaVMDescription *vmDescription = AsebaGetVMDescription(&vmState);
	int len = strlen(vmDescription->name);
	
	// If odd size, compute the crc with the 0
	if(len & 0x1)
		len++;
	
	return do_crc(vmDescription->name, len);
}

int abo_load(FIL * f) {
	unsigned long magic;
	unsigned int t;
	unsigned int read;
	unsigned int len;
	
	// Magic ID
	f_read(f, &magic, sizeof(magic), &read);
	if(read != sizeof(magic) || magic != 0x4F4241UL) 
		return -1;
	
	// Binary file version
	f_read(f, &t, sizeof(t), &read);
	if(read != sizeof(t) || t != ABO_VERSION)
		return -2;
	
	// Protocol version
	f_read(f, &t, sizeof(t), &read);
	if(read != sizeof(t) || t != ASEBA_PROTOCOL_VERSION)
		return -3;
	
	// Product ID
	f_read(f, &t, sizeof(t), &read);
	if(read != sizeof(t) || t != PRODUCT_ID)
		return -4;
	
	// Firmware version
	f_read(f, &t, sizeof(t), &read);
	if(read != sizeof(t) || t != vmVariables.fwversion[0])
		return -5;
		
	// Node ID
	f_read(f, &t, sizeof(t), &read);
	if(read != sizeof(t) || t != vmVariables.id)
		return -6;
	
			
	// Node name (CRC)
	f_read(f, &t, sizeof(t), &read);
	if(read != sizeof(t) || nodename_crc() != t)
		return -7;
	
	// VM description (CRC)
	f_read(f, &t, sizeof(t), &read);
	if(read != sizeof(t) || vmdesc_crc() != t)
		return -8;
	
	// Bytecode size
	f_read(f, &len, sizeof(len), &read);
	if(read != sizeof(len) || len > VM_BYTECODE_SIZE)
		return -9;
	
	// Bytecode payload
	f_read(f, vmState.bytecode, len * 2, &read);
	if(read != len * 2) 
		goto out_clean;
	
	// Bytecode CRC
	f_read(f, &t, sizeof(t), &read);
	if(read != sizeof(t) || do_crc(vmState.bytecode, 2*len) != t)
		goto out_clean;

	return 0;
	
out_clean:
	memset(vmState.bytecode, 0, VM_BYTECODE_SIZE * 2);
	return -11;
}
