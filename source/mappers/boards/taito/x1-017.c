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

static u8 prg[3],chr[6],ramenable[3];
static u8 mirror,chrinvert;
static u8 *sram;

static void sync()
{
	mem_setprg8(0x8,prg[0]);
	mem_setprg8(0xA,prg[1]);
	mem_setprg8(0xC,prg[2]);
	mem_setprg8(0xE,0xFF);
	mem_setchr2(0 ^ chrinvert,chr[0] >> 1);
	mem_setchr2(2 ^ chrinvert,chr[1] >> 1);
	mem_setchr1(4 ^ chrinvert,chr[2]);
	mem_setchr1(5 ^ chrinvert,chr[3]);
	mem_setchr1(6 ^ chrinvert,chr[4]);
	mem_setchr1(7 ^ chrinvert,chr[5]);
	mem_setmirroring(mirror);
}

static u8 read_low(u32 addr)
{
	if(addr < 0x6800) {
		if(ramenable[0] == 0xCA)
			return(sram[addr & 0x1FFF]);
	}
	else if(addr < 0x7000) {
		if(ramenable[0] == 0x69)
			return(sram[addr & 0x1FFF]);
	}
	else if(addr < 0x7400) {
		if(ramenable[0] == 0x84)
			return(sram[addr & 0x1FFF]);
	}
	return((u8)(addr >> 8));
}

static void write_low(u32 addr,u8 data)
{
	if(addr < 0x6800) {
		if(ramenable[0] == 0xCA)
			sram[addr & 0x1FFF] = data;
	}
	else if(addr < 0x7000) {
		if(ramenable[0] == 0x69)
			sram[addr & 0x1FFF] = data;
	}
	else if(addr < 0x7400) {
		if(ramenable[0] == 0x84)
			sram[addr & 0x1FFF] = data;
	}
	else {
		switch(addr) {
			case 0x7EF0:
			case 0x7EF1:
			case 0x7EF2:
			case 0x7EF3:
			case 0x7EF4:
			case 0x7EF5:
				chr[addr & 7] = data;
				break;
			case 0x7EF6:
				mirror = data & 1;
				chrinvert = (data & 2) << 1;
				break;
			case 0x7EF7:
			case 0x7EF8:
			case 0x7EF9:
				ramenable[addr - 0x7EF7] = data;
				break;
			case 0x7EFA:
			case 0x7EFB:
			case 0x7EFC:
				prg[addr - 0x7EFA] = data >> 2;
				break;
		}
		sync();
	}
}

static void reset(int hard)
{
	int i;

	mem_setreadfunc(6,read_low);
	mem_setreadfunc(7,read_low);
	mem_setwritefunc(6,write_low);
	mem_setwritefunc(7,write_low);
	prg[0] = prg[1] = prg[2] = 0;
	for(i=0;i<6;i++) {
		chr[i] = 0;
	}
	mirror = 0;
	mem_setsramsize(2);
	mem_setsram8(6,0);
	sram = mem_getreadptr(6);
	mem_unsetcpu8(6);
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_ARRAY_U8(prg,3);
	STATE_ARRAY_U8(chr,6);
	STATE_ARRAY_U8(ramenable,3);
	STATE_U8(mirror);
	STATE_U8(chrinvert);
	sync();
}

MAPPER(B_TAITO_X1_017,reset,0,0,state);
