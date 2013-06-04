#ifndef __crc32_h__
#define __crc32_h__

#include "types.h"

void crc32_gentab();
u32 crc32(unsigned char *block,unsigned int length);
u32 crc32_byte(u8 data,u32 crc);
u32 crc32_block(u8 *data,u32 length,u32 crc);

#endif
