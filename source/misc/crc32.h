#ifndef __crc32_h__
#define __crc32_h__

#include "types.h"

u32 crc32(unsigned char *block,unsigned int length);
void crc32_gentab();

#endif
