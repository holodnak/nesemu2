#include "mappers/mapperinc.h"
#include "mappers/chips/mmc2.h"
#include "nes/memory.h"

static u8 prg,mirroring;
static u8 latch[2][2],latchstate[2];

void mmc2_sync()
{
	mem_setprg8(0x8,prg);
	mem_setprg8(0xA,0xD);
	mem_setprg8(0xC,0xE);
	mem_setprg8(0xE,0xF);
	mem_setchr4(0,latch[0][latchstate[0]]);
	mem_setchr4(4,latch[1][latchstate[1]]);
	mem_setmirroring(mirroring);
}

void mmc2_init(int hard)
{
	int i;

	for(i=8;i<0x10;i++)
		mem_setwritefunc(i,mmc2_write);
	latch[0][0] = latch[0][1] = 0;
	latch[1][0] = latch[1][1] = 0;
	latchstate[0] = latchstate[1] = 0;
	prg = 0;
	mirroring = 0;
	mmc2_sync();
}

void mmc2_write(u32 addr,u8 data)
{
	switch(addr & 0xF000) {
		case 0xA000:
			prg = data & 0xF;
			break;
		case 0xB000:
			latch[0][0] = data & 0x1F;
			break;
		case 0xC000:
			latch[0][1] = data & 0x1F;
			break;
		case 0xD000:
			latch[1][0] = data & 0x1F;
			break;
		case 0xE000:
			latch[1][1] = data & 0x1F;
			break;
		case 0xF000:
			mirroring = (data & 1) ^ 1;
			break;
	}
	mmc2_sync();
}

void mmc2_tile(u8 tile,int highpt)
{
	if((highpt & 1) == 0) {
		if(tile == 0xFD) {
			latchstate[0] = 0;
			mem_setchr4(0,latch[0][0]);
		}
		else if(tile == 0xFE) {
			latchstate[0] = 1;
			mem_setchr4(0,latch[0][1]);
		}
	}
	else {
		if(tile == 0xFD) {
			latchstate[1] = 0;
			mem_setchr4(4,latch[1][0]);
		}
		else if(tile == 0xFE) {
			latchstate[1] = 1;
			mem_setchr4(4,latch[1][1]);
		}
	}
}

void mmc2_state(int mode,u8 *data)
{
	STATE_U8(prg);
	STATE_U8(mirroring);
	STATE_ARRAY_U8(latch[0],2);
	STATE_ARRAY_U8(latch[1],2);
	STATE_U8(latchstate[0]);
	STATE_U8(latchstate[1]);
	mmc2_sync();
}
