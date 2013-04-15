#include "mappers/mapperinc.h"
#include "mappers/chips/c_latch.h"

static void sync()
{
	mem_setprg16(0x8,latch_reg);
	mem_setprg16(0xC,0xFF);
	mem_setvram8(0,0);
}

static void reset(int hard)
{
	mem_setvramsize(8);
	latch_init(sync);
}

MAPPER(B_UxROM,reset,0,0,latch_state);
