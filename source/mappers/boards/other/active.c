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
#include "mappers/chips/latch.h"

static u8 ram[4];
static readfunc_t read4;
static writefunc_t write4;

static void sync()
{
	u8 chip,prg,chr;

	chip = (latch_addr >> 11) & 3;

	//kludge for ines format
	chip = (chip == 3) ? 2 : chip;

	prg = (latch_addr >> 6) & 0x1F;
	prg |= chip << 5;
	chr = (latch_data & 3) | ((latch_addr & 0xF) << 2);
	if(latch_addr & 0x20) {
		mem_setprg16(0x8,prg);
		mem_setprg16(0xC,prg);
	}
	else
		mem_setprg32(8,prg >> 1);
	mem_setchr8(0,chr);
	mem_setmirroring(((latch_addr >> 13) & 1) ^ 1);
}

static u8 readram(u32 addr)
{
	if(addr < 0x4020)
		return(read4(addr));
	return(ram[addr & 3]);
}

static void writeram(u32 addr,u8 data)
{
	if(addr < 0x4020) {
		write4(addr,data);
		return;
	}
	ram[addr & 3] = data & 0xF;
}

static void reset(int hard)
{
	latch_reset(sync,hard);
	read4 = mem_getreadfunc(4);
	write4 = mem_getwritefunc(4);
	mem_setreadfunc(4,readram);
	mem_setreadfunc(5,readram);
	mem_setwritefunc(4,writeram);
	mem_setwritefunc(5,writeram);
}

static void state(int mode,u8 *data)
{
	STATE_ARRAY_U8(ram,4);
	latch_state(mode,data);
}

MAPPER(B_ACTIVE,reset,0,0,state);
