#ifndef __mmc4_h__
#define __mmc4_h__

#include "types.h"

void mmc4_init(int hard);
void mmc4_write(u32 addr,u8 value);
void mmc4_tile(u8 tile,int highpt);
void mmc4_state(int mode,u8 *spot);

#endif
