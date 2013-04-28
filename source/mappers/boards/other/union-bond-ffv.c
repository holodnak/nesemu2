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

static u8 prg;

static void sync()
{
	mem_setsram8(6,0);
	mem_setprg32(8,prg);
	mem_setvram8(0,0);
}

static void write(u32 addr,u8 data)
{
	switch(addr & 0x7300) {
		case 0x5000:
			prg = data;
			break;
	}
	sync();
}

static void reset(int hard)
{
	mem_setsramsize(2);
	mem_setvramsize(8);
	mem_setwritefunc(0x5,write);
	mem_setwritefunc(0xD,write);
	prg = 0xFF;
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_U8(prg);
	sync();
}

MAPPER(B_UNION_BOND_FFV,reset,0,0,0,state);
