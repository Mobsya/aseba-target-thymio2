#ifndef _THYMIO_MEMORY_LAYOUT_H_
#define _THYMIO_MEMORY_LAYOUT_H_

#include "flash/flash.h"

// A chunk is a bytecode image
#define PAGE_PER_CHUNK ((VM_BYTECODE_SIZE*2+3 + INSTRUCTIONS_PER_PAGE*3 - 1) / (INSTRUCTIONS_PER_PAGE*3))
#if PAGE_PER_CHUNK == 0
#undef PAGE_PER_CHUNK
#define PAGE_PER_CHUNK 1
#endif

/* noload will tell the linker to allocate the space but NOT to initialise it. (left unprogrammed)*/
/* I cannot delare it as an array since it's too large ( > 65535 ) */

// Put everything in the same section, so we are 100% sure that the linker will put them contiguously.
// Force the address, since the linker sometimes put it in the middle of the code
#ifndef FLASH_END
#define FLASH_END 0x15800
#endif

#define NUMBER_OF_CHUNK 3

// Saveguard the bootloader (4 pages)
// Done by linker script...
//unsigned char __bootloader[INSTRUCTIONS_PER_PAGE * 2 * 4 - 2*4/* used by the config words */] __attribute((space(prog), section(".boot"), noload, address(FLASH_END - 0x1000)));

#define ASEBA_SETTINGS_ADDRESS (FLASH_END - (0x1000 + 0x400))
#define LOG_FLASH_ADDRESS (ASEBA_SETTINGS_ADDRESS - 0x800)
#define THYMIO_SETTINGS_ADDRESS (LOG_FLASH_ADDRESS - 0x400)


#define PAGE_0 (LOG_FLASH_ADDRESS)
#define PAGE_1 (PAGE_0 + INSTRUCTIONS_PER_PAGE*2)

#endif
