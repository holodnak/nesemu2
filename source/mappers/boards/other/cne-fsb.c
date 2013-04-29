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

static u8 reg[8];
static u8 *sram6;

static void sync()
{
	mem_setprg8(0x8,reg[0]);
	mem_setprg8(0xA,reg[1]);
	mem_setprg8(0xC,reg[2]);
	mem_setprg8(0xE,reg[3]);
	mem_setchr2(0,reg[4]);
	mem_setchr2(2,reg[5]);
	mem_setchr2(4,reg[6]);
	mem_setchr2(6,reg[7]);
}

static u8 read6(u32 addr)
{
	if(addr >= 0x6800)
		return(sram6[addr & 0xFFF]);
	return((u8)(addr >> 8));
}

static void write6(u32 addr,u8 data)
{
	if(addr >= 0x6800)
		sram6[addr & 0xFFF] = data;
	else {
		reg[addr & 7] = data;
		sync();
	}
}

static void reset(int hard)
{
	int i;

	mem_setsramsize(2);
	mem_setsram8(6,0);
	sram6 = mem_getwriteptr(6);
	mem_unsetcpu8(6);
	mem_setreadfunc(6,read6);
	mem_setwritefunc(6,write6);
	for(i=0;i<8;i++)
		reg[i] = 0;
	reg[3] = 0xFF;
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_ARRAY_U8(reg,8);
	sync();
}

MAPPER(B_CNE_FSB,reset,0,0,0,state);
