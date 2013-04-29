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

static u8 prg,chr;
static writefunc_t write4;

static void sync()
{
	mem_setprg32(8,prg);
	mem_setchr8(0,chr);
}

static void write(u32 addr,u8 data)
{
	if(addr < 0x4020) {
		write4(addr,data);
		return;
	}
	prg = data >> 4;
	chr = data & 0xF;
	sync();
}

static void reset(int hard)
{
	write4 = mem_getwritefunc(4);
	mem_setwritefunc(4,write);
	mem_setwritefunc(5,write);
	mem_setsramsize(2);
	mem_setsram8(6,0);
	prg = chr = 0;
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_U8(prg);
	STATE_U8(chr);
	sync();
}

MAPPER(B_CNE_SHLZ,reset,0,0,0,state);
