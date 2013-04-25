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

static u8 prg,chr[2];

static void sync()
{
	mem_setprg32(8,prg);
	mem_setchr4(0,chr[0]);
	mem_setchr4(4,chr[1]);
}

static void write(u32 addr,u8 data)
{
	switch(addr) {
		case 0x7FFD:	prg = data;		break;
		case 0x7FFE:	chr[0] = data;	break;
		case 0x7FFF:	chr[1] = data;	break;
	}
	sync();
}

static void reset(int hard)
{
	mem_setwritefunc(7,write);
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_U8(prg);
	STATE_ARRAY_U8(chr,2);
	sync();
}

MAPPER(B_NINA_001,reset,0,0,0,state);
