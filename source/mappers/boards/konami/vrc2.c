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

static u8 vrc2a_map[] = {0,1,2,3,1};
static u8 vrc2b_map[] = {0,2,1,3,0};
static u8 vrc2c_map[] = {0,1,2,3,0};
static u8 *map;
static u8 prg[2],chr[8],mirror,latch;

static void sync()
{
	int i;

	if(nes->cart->wram.size)
		mem_setwram4(7,0);
	mem_setprg8(0x8,prg[0]);
	mem_setprg8(0xA,prg[1]);
	mem_setprg16(0xC,0xFF);
	for(i=0;i<8;i++)
		mem_setchr1(i,chr[i] >> map[4]);
	switch(mirror) {
		case 0:	mem_setmirroring(MIRROR_V);	break;
		case 1:	mem_setmirroring(MIRROR_H);	break;
		case 2:	mem_setmirroring(MIRROR_1L);	break;
		case 3:	mem_setmirroring(MIRROR_1H);	break;
	}
}

static u8 read_latch(u32 addr)
{
	return(((addr >> 8) & 0xFE) | latch);
}

static void write_latch(u32 addr,u8 data)
{
	latch = data & 1;
}

static void write(u32 addr,u8 data)
{
	data &= 0x1F;
	switch(addr & 0xF000) {
		case 0x8000:
			prg[0] = data;
			break;
		case 0x9000:
			mirror = data & 1;
			break;
		case 0xA000:
			prg[1] = data;
			break;
		case 0xB000:
			switch(map[addr & 3]) {
				case 0: chr[0] = (chr[0] & 0xF0) | ((data << 0) & 0x0F); break;
				case 1: chr[1] = (chr[1] & 0xF0) | ((data << 0) & 0x0F); break;
				case 2: chr[0] = (chr[0] & 0x0F) | ((data << 4) & 0xF0); break;
				case 3: chr[1] = (chr[1] & 0x0F) | ((data << 4) & 0xF0); break;
			}
			break;
		case 0xC000:
			switch(map[addr & 3]) {
				case 0: chr[2] = (chr[2] & 0xF0) | ((data << 0) & 0x0F); break;
				case 1: chr[3] = (chr[3] & 0xF0) | ((data << 0) & 0x0F); break;
				case 2: chr[2] = (chr[2] & 0x0F) | ((data << 4) & 0xF0); break;
				case 3: chr[3] = (chr[3] & 0x0F) | ((data << 4) & 0xF0); break;
			}
			break;
		case 0xD000:
			switch(map[addr & 3]) {
				case 0: chr[4] = (chr[4] & 0xF0) | ((data << 0) & 0x0F); break;
				case 1: chr[5] = (chr[5] & 0xF0) | ((data << 0) & 0x0F); break;
				case 2: chr[4] = (chr[4] & 0x0F) | ((data << 4) & 0xF0); break;
				case 3: chr[5] = (chr[5] & 0x0F) | ((data << 4) & 0xF0); break;
			}
			break;
		case 0xE000:
			switch(map[addr & 3]) {
				case 0: chr[6] = (chr[6] & 0xF0) | ((data << 0) & 0x0F); break;
				case 1: chr[7] = (chr[7] & 0xF0) | ((data << 0) & 0x0F); break;
				case 2: chr[6] = (chr[6] & 0x0F) | ((data << 4) & 0xF0); break;
				case 3: chr[7] = (chr[7] & 0x0F) | ((data << 4) & 0xF0); break;
			}
			break;
	}
	sync();
}

static void reset(int revision)
{
	int i;

	switch(revision) {
		default:
		case B_KONAMI_VRC2A: map = vrc2a_map; break;
		case B_KONAMI_VRC2B: map = vrc2b_map; break;
		case B_KONAMI_VRC2C: map = vrc2c_map; break;
	}
	mem_setreadfunc(6,read_latch);
	mem_setwritefunc(6,write_latch);
	for(i=8;i<0x10;i++)
		mem_setwritefunc(i,write);
	for(i=0;i<8;i++) {
		prg[i & 1] = 0;
		chr[i] = 0;
	}
	mirror = 0;
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_ARRAY_U8(prg,2);
	STATE_ARRAY_U8(chr,8);
	STATE_U8(mirror);
	STATE_U8(latch);
	sync();
}

static void reset_vrc2a(int hard)	{	reset(B_KONAMI_VRC2A);	}
static void reset_vrc2b(int hard)	{	reset(B_KONAMI_VRC2B);	}
static void reset_vrc2c(int hard)	{	reset(B_KONAMI_VRC2C);	}

MAPPER(B_KONAMI_VRC2A,reset_vrc2a,0,0,state);
MAPPER(B_KONAMI_VRC2B,reset_vrc2b,0,0,state);
MAPPER(B_KONAMI_VRC2C,reset_vrc2c,0,0,state);
