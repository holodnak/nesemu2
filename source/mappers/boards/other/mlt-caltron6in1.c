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

static u8 reg[2];

static void sync()
{
	mem_setprg32(8,reg[0] & 7);
	mem_setchr8(0,((reg[0] & 0x18) >> 1) | (reg[1] & 3));
	mem_setmirroring(((~reg[0]) >> 5) & 1);
}

static void write(u32 addr,u8 data)
{
	if(addr < 0x6800)
		reg[0] = addr & 0x3F;
	else if((addr >= 0x8000) && (reg[0] & 4))
		reg[1] = data & 3;
	sync();
}

static void reset(int hard)
{
	int i;

	for(i=6;i<16;i++)
		mem_setwritefunc(i,write);
	reg[0] = reg[1] = 0;
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_ARRAY_U8(reg,2);
	sync();
}

MAPPER(B_MLT_CALTRON6IN1,reset,0,0,state);
