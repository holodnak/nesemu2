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

static u8 reg[2],latch;

static void sync()
{
	switch(reg[0] & 0x70) {

		//swap 16kb at $8000, $C000 is last in the chunk
		case 0x00:
		case 0x20:
		case 0x40:
		case 0x60:
			mem_setprg16(0x8,(reg[0] & 0xF) | ((reg[1] & 1) << 5) | ((reg[0] & 0x20) >> 1));
			mem_setprg16(0xC,0x1F | ((reg[1] & 1) << 5));
			break;

		//swap 32kb at $8000
		case 0x50:
			mem_setprg32(8,(reg[0] & 0xF) | ((reg[1] & 1) << 4));
			break;

		//swap 16kb at $8000 (alternate), $C000 is last in the chunk
		case 0x70:
			mem_setprg16(0x8,(reg[0] & 0xF) | ((reg[1] & 1) << 5) | ((reg[0] & 0x8) << 1));
			mem_setprg16(0xC,0x1F | ((reg[1] & 1) << 5));
			break;
	}
	mem_setwram8(6,0);
	mem_setvram8(0,0);
}

static u8 read(u32 addr)
{
	switch(addr & 0x7700) {
		case 0x5100:
			return(latch);
	}
	return((u8)(addr >> 8));
}

static void write(u32 addr,u8 data)
{
	switch(addr & 0x7300) {
		case 0x5000:
			reg[0] = data;
			break;
		case 0x5100:
			reg[1] = data;
			break;
		case 0x5300:
			latch = data;
			break;
	}
	sync();
}

static void reset(int hard)
{
	mem_setwramsize(8);
	mem_setvramsize(8);
	mem_setwritefunc(0x5,write);
	mem_setwritefunc(0xD,write);
	mem_setreadfunc(0x5,read);
	reg[0] = 0x7F;
	reg[1] = 0;
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_ARRAY_U8(reg,2);
	STATE_U8(latch);
	sync();
}

MAPPER(B_WAIXING_FFV,reset,0,0,state);
