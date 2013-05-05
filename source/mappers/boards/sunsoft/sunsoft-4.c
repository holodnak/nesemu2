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

static u8 prg,chr[4],nt[2],mirror;

static void sync()
{
	mem_setprg16(0x8,prg);
	mem_setprg16(0xC,0xFF);
	mem_setchr2(0,chr[0]);
	mem_setchr2(2,chr[1]);
	mem_setchr2(4,chr[2]);
	mem_setchr2(6,chr[3]);
	if(mirror & 0x10) {
		if((mirror & 1) == 0) {
			mem_setchr1(0x8,nt[0] | 0x80);
			mem_setchr1(0x9,nt[1] | 0x80);
			mem_setchr1(0xA,nt[0] | 0x80);
			mem_setchr1(0xB,nt[1] | 0x80);
		}
		else {
			mem_setchr1(0x8,nt[0] | 0x80);
			mem_setchr1(0x9,nt[0] | 0x80);
			mem_setchr1(0xA,nt[1] | 0x80);
			mem_setchr1(0xB,nt[1] | 0x80);
		}
	}
	else
		mem_setmirroring((mirror & 1) ^ 1);
}

static void write(u32 addr,u8 data)
{
	switch(addr & 0xF000) {
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
			chr[(addr >> 12) & 3] = data;
			break;
		case 0xC000:
		case 0xD000:
			nt[(addr >> 12) & 1] = data;
			break;
		case 0xE000:
			mirror = data;
			break;
		case 0xF000:
			prg = data;
			break;
	}
	sync();
}

static void reset(int hard)
{
	int i;

	for(i=8;i<16;i++)
		mem_setwritefunc(i,write);
	if(hard) {
		prg = 0;
		chr[0] = chr[1] = chr[2] = chr[3] = 0;
		nt[0] = nt[1] = 0;
		mirror = 0;
	}
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_U8(prg);
	STATE_ARRAY_U8(chr,4);
	STATE_ARRAY_U8(nt,2);
	STATE_U8(mirror);
	sync();
}

MAPPER(B_SUNSOFT_4,reset,0,0,state);
