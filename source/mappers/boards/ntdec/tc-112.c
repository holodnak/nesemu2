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

static u8 prg,chr[4];

static void sync()
{
	mem_setprg8(0x8,prg);
	mem_setprg8(0xA,0xFD);
	mem_setprg8(0xC,0xFE);
	mem_setprg8(0xE,0xFF);
	mem_setchr4(0,chr[0] >> 2);
	mem_setchr2(4,chr[1] >> 1);
	mem_setchr2(6,chr[2] >> 1);
}

static void write(u32 addr,u8 data)
{
	switch(addr & 3) {
		case 0:
		case 1:
		case 2:
			chr[addr & 3] = data;
			break;
		case 3:
			prg = data;
			break;
	}
	sync();
}

static void reset(int hard)
{
	mem_setwritefunc(6,write);	
	mem_setwritefunc(7,write);	
	if(hard) {
		prg = chr[0] = chr[1] = chr[2] = 0;
	}
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_U8(prg);
	STATE_ARRAY_U8(chr,3);
	sync();
}

MAPPER(B_NTDEC_TC112,reset,0,0,state);
