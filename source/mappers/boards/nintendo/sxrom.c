#include "mappers/mapperinc.h"
#include "mappers/chips/mmc1.h"

static void reset(int hard)
{
	if(nes.cart->chr.size == 0)
		mem_setvramsize(8);
	mem_setsramsize(2);
	mmc1_init(mmc1_sync);
}

MAPPER(B_SxROM,reset,0,0,mmc1_state);
