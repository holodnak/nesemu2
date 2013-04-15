#include "mappers/mapperinc.h"

static void (*sync)();
u8 latch_reg;
u32 latch_addr;

void latch_write(u32 addr,u8 data)
{
	latch_addr = addr;
	latch_reg = data;
	sync();
}

void latch_init(void (*s)())
{
	int i;

	sync = s;
	for(i=8;i<16;i++)
		mem_setwritefunc(i,latch_write);
	latch_reg = 0;
	latch_addr = 0;
	sync();
}

void latch_state(int mode,u8 *data)
{
	STATE_U8(latch_reg);
	STATE_U16(latch_addr);
	sync();
}
