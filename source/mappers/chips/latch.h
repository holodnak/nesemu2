#ifndef __latch_h__
#define __latch_h__

extern u8 latch_reg;
extern u32 latch_addr;

void latch_write(u32 addr,u8 data);
void latch_init(void (*sync)());
void latch_state(int mode,u8 *data);

#endif
