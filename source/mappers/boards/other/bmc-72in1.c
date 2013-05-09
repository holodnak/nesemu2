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

static void sync()
{
	u8 prg,chr,hi;

	hi = (latch_addr >> 8) & 0x40;
	prg = (latch_addr >> 6) & 0x3F;
	prg |= hi;
	chr = latch_addr & 0x3F;
	chr |= hi;
	if(((latch_addr >> 12) & 1) == 0) {
		mem_setprg32(8,prg >> 1);
	}
	else {
		mem_setprg16(0x8,prg);
		mem_setprg16(0xC,prg);
	}
	mem_setchr8(0,chr);
	mem_setmirroring(((latch_addr >> 13) & 1) ^ 1);
}

static u8 read(u32 addr)
{
	if(addr >= 0x5800)
		return(ram[addr & 3]);
	return((u8)(addr >> 8));
}

static void write(u32 addr,u8 data)
{
	if(addr >= 0x5800)
		ram[addr & 3] = data;
}

static void reset(int hard)
{
	latch_init(sync);
	mem_setreadfunc(5,read);
	mem_setwritefunc(5,write);
	if(hard) {
		ram[0] = ram[1] = ram[2] = ram[3] = 0;
	}
}

static void state(int mode,u8 *data)
{
	STATE_ARRAY_U8(ram,4);
	latch_state(mode,data);
}

MAPPER(B_BMC_72IN1,reset,0,0,state);

