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

static u8 prg[3],chr[2],mirror;

static void sync()
{
	mem_setprg8(0x8,prg[0]);
	mem_setprg8(0xA,prg[1]);
	mem_setprg8(0xC,prg[2]);
	mem_setprg8(0xE,-1);
	mem_setchr4(0,chr[0]);
	mem_setchr4(4,chr[1]);
	if(mirror)
		mem_setmirroring(MIRROR_H);
	else
		mem_setmirroring(MIRROR_V);
}

static void write(u32 addr,u8 data)
{
	data &= 0xF;
	switch(addr & 0xF000) {
		case 0x8000:
			prg[0] = data;
			break;
		case 0x9000:
			mirror = data & 1;
			chr[0] = (chr[0] & 0xF) | ((data & 2) << 3);
			chr[1] = (chr[1] & 0xF) | ((data & 4) << 2);
			break;
		case 0xA000:
			prg[1] = data;
			break;
		case 0xB000:
			break;
		case 0xC000:
			prg[2] = data;
			break;
		case 0xD000:
			break;
		case 0xE000:
			chr[0] = (chr[0] & 0x10) | data;
			break;
		case 0xF000:
			chr[1] = (chr[1] & 0x10) | data;
			break;
	}
	sync();
}

static void reset(int hard)
{
	int i;

	for(i=8;i<0x10;i++)
		mem_setwritefunc(i,write);
	prg[0] = 0;
	prg[1] = 0;
	prg[2] = 0;
	chr[0] = 0;
	chr[1] = 0;
	mirror = 0;
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_ARRAY_U8(prg,3);
	STATE_ARRAY_U8(chr,2);
	STATE_U8(mirror);
	sync();
}

MAPPER(B_VRC1,reset,0,0,state);
