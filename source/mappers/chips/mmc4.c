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

#include "mappers/mapperinc.h"
#include "mappers/chips/mmc4.h"
#include "nes/memory.h"

static u8 prg,mirroring;
static u8 latch[2][2],latchstate[2];
static readfunc_t ppuread;

static u8 readtile(u32 addr)
{
	switch(addr & 0xFFF8) {
		case 0x0FD8:
			latchstate[0] = 0;
			mem_setchr4(0,latch[0][0]);
			break;
		case 0x0FE8:
			latchstate[0] = 1;
			mem_setchr4(0,latch[0][1]);
			break;
		case 0x1FD8:
			latchstate[1] = 0;
			mem_setchr4(4,latch[1][0]);
			break;
		case 0x1FE8:
			latchstate[1] = 1;
			mem_setchr4(4,latch[1][1]);
			break;
	}
	return(ppuread(addr));
}

void mmc4_sync()
{
	mem_setsram8(0x6,0);
	mem_setprg16(0x8,prg);
	mem_setprg16(0xC,0xF);
	mem_setchr4(0,latch[0][latchstate[0]]);
	mem_setchr4(4,latch[1][latchstate[1]]);
	mem_setmirroring(mirroring);
}

void mmc4_init(int hard)
{
	int i;

	for(i=8;i<0x10;i++)
		mem_setwritefunc(i,mmc4_write);
	mem_setsramsize(2);
	latch[0][0] = latch[0][1] = 0;
	latch[1][0] = latch[1][1] = 0;
	latchstate[0] = latchstate[1] = 0;
	prg = 0;
	mirroring = 0;
	ppuread = ppu_getreadfunc();
	ppu_setreadfunc(readtile);
	mmc4_sync();
}

void mmc4_write(u32 addr,u8 data)
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
	mmc4_sync();
}

void mmc4_state(int mode,u8 *data)
{
	STATE_U8(prg);
	STATE_U8(mirroring);
	STATE_ARRAY_U8(latch[0],2);
	STATE_ARRAY_U8(latch[1],2);
	STATE_U8(latchstate[0]);
	STATE_U8(latchstate[1]);
	mmc4_sync();
}
