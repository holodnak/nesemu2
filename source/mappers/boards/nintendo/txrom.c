#include "mappers/mapperinc.h"
#include "mappers/chips/mmc3.h"

static void reset(int hard)
{
	mmc3_init(mmc3_sync);
}

MAPPER(B_TxROM,reset,0,mmc3_cycle,mmc3_state);
