#ifndef __mmc1_h__
#define __mmc1_h__

#include "types.h"

void mmc1_sync();
void mmc1_init(void (*ssync)());
void mmc1_state(int mode,u8 *spot);
void mmc1_write(u32 addr,u8 value);
void mmc1_syncmirror();
void mmc1_syncprg(int aand,int oor);
void mmc1_syncchr(int aand,int oor);
void mmc1_syncvram(int aand,int oor);
void mmc1_syncsram();
int mmc1_getlowprg();
int mmc1_gethighprg();
int mmc1_getlowchr();
int mmc1_gethighchr();

#endif
