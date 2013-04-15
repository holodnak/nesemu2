#include "mappers/mapperinc.h"
#include "mappers/chips/mmc1.h"

static void (*sync)() = 0;
static u8 regs[4];
static u8 latch,latchpos;
static u8 lastreg;

void mmc1_sync()
{
	mmc1_syncmirror();
	if(nes.cart->prg.size > 0x40000) {
		mem_setprg16(0x8,(mmc1_getlowprg() & 0xf) | (mmc1_getlowchr() & 0x10));
		mem_setprg16(0xC,(mmc1_gethighprg() & 0xf) | (mmc1_getlowchr() & 0x10));
	}
	else
		mmc1_syncprg(0xF,0);
	if(nes.cart->chr.size)
		mmc1_syncchr(0x1F,0);
	else
		mmc1_syncvram(1,0);
	mmc1_syncsram();
}

int mmc1_getlowprg()
{
	if(regs[0] & 8) {
		if(regs[0] & 4)
			return(regs[3] & 0xF);
		else
			return(0);
	}
	else
		return(regs[3] & 0xE);
}

int mmc1_gethighprg()
{
	if(regs[0] & 8) {
		if(regs[0] & 4)
			return(0xF);
		else
			return(regs[3] & 0xF);
	}
	else
		return((regs[3] & 0xE) | 1);
}

int mmc1_getlowchr()
{
	if(regs[0] & 0x10)
		return(regs[1] & 0x1F);
	else
		return(regs[1] & 0x1E);
}

int mmc1_gethighchr()
{
	if(regs[0] & 0x10)
		return(regs[2] & 0x1F);
	else
		return((regs[1] & 0x1E) | 1);
}

void mmc1_init(void (*s)())
{
	int i;

	for(i=8;i<0x10;i++)
		mem_setwritefunc(i,mmc1_write);
	sync = s;
	regs[0] = 0x0C;
	regs[1] = regs[2] = regs[3] = 0x00;
	latch = 0;
	latchpos = 0;
	lastreg = 0;
	sync();
}

void mmc1_state(int mode,u8 *data)
{
	STATE_ARRAY_U8(regs,4);
	STATE_U8(latch);
	STATE_U8(latchpos);
	STATE_U8(lastreg);
	sync();
}

void mmc1_write(u32 addr,u8 data)
{
	u8 t = (u8)((addr >> 12) >> 1) & 3;

	if(data & 0x80) {
		latch = latchpos = 0;
		regs[0] |= 0xC;
		return;
	}
	if(t != lastreg)
		latch = latchpos = 0;
	lastreg = t;
	latch |= (data & 1) << latchpos++;
	if(latchpos == 5) {
		regs[t] = latch & 0x1F;
		latch = latchpos = 0;
		sync();
	}
}

void mmc1_syncmirror()
{
	switch(regs[0] & 3) {
		case 0:mem_setmirroring(MIRROR_1L);break;
		case 1:mem_setmirroring(MIRROR_1H);break;
		case 2:mem_setmirroring(MIRROR_V);break;
		case 3:mem_setmirroring(MIRROR_H);break;
	}
}

void mmc1_syncprg(int aand,int oor)
{
	mem_setprg16(0x8,(mmc1_getlowprg() & aand) | oor);
	mem_setprg16(0xC,(mmc1_gethighprg() & aand) | oor);
}

void mmc1_syncchr(int aand,int oor)
{
	mem_setchr4(0,(mmc1_getlowchr() & aand) | oor);
	mem_setchr4(4,(mmc1_gethighchr() & aand) | oor);
}

void mmc1_syncvram(int aand,int oor)
{
	mem_setvram4(0,(mmc1_getlowchr() & aand) | oor);
	mem_setvram4(4,(mmc1_gethighchr() & aand) | oor);
}

void mmc1_syncsram()
{
	if(regs[3] & 0x10)
		mem_unsetcpu8(6);
	else
		mem_setsram8(6,0);
}
