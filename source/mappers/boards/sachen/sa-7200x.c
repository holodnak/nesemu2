#include "mappers/mapperinc.h"

static int type;
static u8 reg;
static writefunc_t write4;
static void (*sync)() = 0;

static void sync_72007()
{
	mem_setprg16(0x8,0);
	mem_setprg16(0xC,1);
	mem_setchr8(0,reg >> 7);
}

static void sync_72008()
{
	mem_setprg32(8,(reg >> 2) & 1);
	mem_setchr8(0,reg & 3);
}

static void write45(u32 addr,u8 data)
{
	if(addr < 0x4020) {
		write4(addr,data);
		return;
	}
	if(addr & 0x100) {
		reg = data;
		sync();
	}
}

static void reset(int t,int hard)
{
	type = t;
	if(type == B_SA_72007)
		sync = sync_72007;
	else
		sync = sync_72008;
	write4 = mem_getwritefunc(4);
	mem_setwritefunc(4,write45);
	mem_setwritefunc(5,write45);
	reg = 0;
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_U8(reg);
	sync();
}

static void reset_72007(int hard)	{	reset(B_SA_72007,hard);	}
static void reset_72008(int hard)	{	reset(B_SA_72008,hard);	}

MAPPER(B_SA_72007,reset_72007,0,0,state);
MAPPER(B_SA_72008,reset_72008,0,0,state);
