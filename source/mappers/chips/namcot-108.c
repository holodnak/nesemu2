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

static void (*sync)();
static u8 reg[8],index;

u8 namcot108_getindex()
{
	return(index);
}

u8 *namcot108_getregs()
{
	return(reg);
}

void namcot108_sync()
{
	mem_setchr2(0,reg[0] >> 1);
	mem_setchr2(2,reg[1] >> 1);
	mem_setchr1(4,reg[2]);
	mem_setchr1(5,reg[3]);
	mem_setchr1(6,reg[4]);
	mem_setchr1(7,reg[5]);
	mem_setprg8(0x8,reg[6]);
	mem_setprg8(0xA,reg[7]);
	mem_setprg16(0xC,0xFF);
}

void namcot108_write(u32 addr,u8 data)
{
	switch(addr) {
		case 0x8000:
			index = data;
			break;
		case 0x8001:
			reg[index & 7] = data;
			sync();
			break;
	}
}

void namcot108_reset(void (*syncfunc)(),int hard)
{
	int i;

	for(i=8;i<16;i++)
		mem_setwritefunc(i,namcot108_write);
	for(i=0;i<8;i++)
		reg[i] = 0;
	index = 0;
	sync = syncfunc;
	sync();
}

void namcot108_state(int mode,u8 *data)
{
	STATE_U8(index);
	STATE_ARRAY_U8(reg,8);
	sync();
}
