#include "mappers/mapperinc.h"
#include "mappers/chips/c_mmc1.h"

static void reset(int hard)
{
	mem_setvramsize(8);
	mem_setsramsize(2);
	mmc1_init(mmc1_sync);
}

MAPPER(B_SxROM,reset,0,0,mmc1_state);
