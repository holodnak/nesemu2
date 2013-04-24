#include "mappers/mapperinc.h"
#include "mappers/chips/latch.h"

static void sync()
{
	mem_setchr8(0,latch_reg & 7);
}

static void reset(int hard)
{
	mem_setvramsize(8);
	latch_init(sync);
	mem_setprg32(8,0);
}

MAPPER(B_CNROM,reset,0,0,0,latch_state);
