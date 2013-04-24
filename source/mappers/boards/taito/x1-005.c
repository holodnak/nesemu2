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

enum taito_x1_005_e {
	TAITO_X1_005,
	TAITO_X1_005A,
};

static u8 prg[3],chr[6];
static u8 mirror,ramenable;
static u8 ram[128];
static u8 type;

static void sync()
{
	mem_setprg8(0x8,prg[0]);
	mem_setprg8(0xA,prg[1]);
	mem_setprg8(0xC,prg[2]);
	mem_setprg8(0xE,0xFF);
	mem_setchr2(0,chr[0] >> 1);
	mem_setchr2(2,chr[1] >> 1);
	mem_setchr1(4,chr[2]);
	mem_setchr1(5,chr[3]);
	mem_setchr1(6,chr[4]);
	mem_setchr1(7,chr[5]);
	mem_setmirroring(mirror);
}

static u8 read_low(u32 addr)
{
	if(addr >= 0x7F00) {
		if(ramenable == 0xA3) {
			return(ram[addr & 0x7F]);
		}
	}
	return((u8)(addr >> 8));
}

static void write_low(u32 addr,u8 data)
{
	if(addr >= 0x7F00) {
		if(ramenable == 0xA3) {
			ram[addr & 0x7F] = data;
		}
	}
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
		case 0x7EF7:
			mirror = data & 1;
			break;
		case 0x7EF8:
		case 0x7EF9:
			ramenable = data;
			break;
		case 0x7EFA:
		case 0x7EFB:
			prg[0] = data;
			break;
		case 0x7EFC:
		case 0x7EFD:
			prg[1] = data;
			break;
		case 0x7EFE:
		case 0x7EFF:
			prg[2] = data;
			break;
	}
	sync();
}

static void reset(int t,int hard)
{
	int i;

	type = t;
	mem_setreadfunc(7,read_low);
	mem_setwritefunc(7,write_low);
	prg[0] = prg[1] = prg[2] = 0;
	for(i=0;i<6;i++) {
		chr[i] = 0;
	}
	mirror = 0;
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_ARRAY_U8(prg,3);
	STATE_ARRAY_U8(chr,6);
	STATE_U8(mirror);
	STATE_ARRAY_U8(ram,128);
	sync();
}

static void reset_x1_005(int hard)
{
	reset(TAITO_X1_005,hard);
}

static void reset_x1_005a(int hard)
{
	reset(TAITO_X1_005A,hard);
}

MAPPER(B_TAITO_X1_005,reset_x1_005,0,0,0,state);
MAPPER(B_TAITO_X1_005A,reset_x1_005a,0,0,0,state);
