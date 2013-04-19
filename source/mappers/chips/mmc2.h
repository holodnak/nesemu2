#ifndef __mmc2_h__
#define __mmc2_h__

#include "types.h"

void mmc2_sync();
void mmc2_init(int hard);
void mmc2_write(u32 addr,u8 value);
void mmc2_tile(u8 tile,int highpt);
void mmc2_state(int mode,u8 *spot);

#endif
