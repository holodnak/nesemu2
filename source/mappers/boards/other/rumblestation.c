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
	mem_setprg32(8,((reg[1] & 0xF) << 1) | (reg[0] & 1));
	mem_setchr8(0,((reg[1] & 0xF0) >> 1) | ((reg[0] & 0x70) >> 4));
}

//write high chr/prg bits (write addresses $6000-7FFF)
static void writehi(u32 addr,u8 data)
{
	reg[1] = data;
	sync();
}

//write low chr/prg bits (write addresses $8000-FFFF)
static void writelo(u32 addr,u8 data)
{
	reg[0] = data;
	sync();
}

static void reset(int hard)
{
	int i;

	for(i=6;i<8;i++)
		mem_setwritefunc(i,writehi);
	for(i=8;i<16;i++)
		mem_setwritefunc(i,writelo);
	reg[0] = 0;
	reg[1] = 0;
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_U8(reg[0]);
	STATE_U8(reg[1]);
	sync();
}

MAPPER(B_RUMBLESTATION,reset,0,0,state);
