/***************************************************************************
 *   Copyright (C) 2013 by James Holodnak                                  *
 *   jamesholodnak@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/* 
	differences in mmc1 chip types (from the wiki):
		MMC1A: PRG RAM is always enabled.
		MMC1B: PRG RAM is enabled by default.
		MMC1C: PRG RAM is disabled by default. 
*/

#include "mappers/mapperinc.h"
#include "mappers/chips/mmc1.h"

static void (*sync)() = 0;
static u8 regs[4];
static u8 latch,latchpos;
static u8 lastreg;
static int type;

void mmc1_sync()
{
	mmc1_syncmirror();
	if(nes->cart->prg.size > 0x40000) {
		mem_setprg16(0x8,(mmc1_getlowprg() & 0xf) | (mmc1_getlowchr() & 0x10));
		mem_setprg16(0xC,(mmc1_gethighprg() & 0xf) | (mmc1_getlowchr() & 0x10));
	}
	else
		mmc1_syncprg(0xF,0);
	if(nes->cart->chr.size)
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

void mmc1_reset(int t,void (*s)(),int hard)
{
	int i;

	type = t;
	for(i=8;i<0x10;i++)
		mem_setwritefunc(i,mmc1_write);
	sync = s;
	if(hard) {
		regs[0] = 0x0C;
		regs[1] = regs[2] = regs[3] = 0x00;
		if(type == C_MMC1C)
			regs[3] = 0x10;
		latch = 0;
		latchpos = 0;
		lastreg = 0;
	}
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
	if(nes->cart->wram.size == 16 * 1024) {
		mem_setwram8(6,(mmc1_getlowchr() >> 3) & 1);
		return;
	}
	else if(nes->cart->wram.size == 32 * 1024) {
		mem_setwram8(6,(mmc1_getlowchr() >> 2) & 3);
		return;
	}

	if(type == C_MMC1A) {
		mem_setwram8(6,0);
	}
	else {
		if(regs[3] & 0x10)
			mem_unsetcpu8(6);
		else
			mem_setwram8(6,0);
	}
}
