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

static u8 prg,mode,mirror;

static void sync()
{
	if(mode == 0) {
		mem_setprg32(8,prg >> 1);
	}
	else {
		mem_setprg16(0x8,prg);
		mem_setprg16(0xC,prg);
	}
	mem_setvram8(0,0);
	mem_setmirroring(mirror);
}

static void write(u32 addr,u8 data)
{
	if(addr & 1) {
		prg &= 0x3F;
		prg |= (data & 1) << 6;
	}
	else {
		prg &= 0x40;
		prg |= ((data >> 2) & 0x20) | (data & 0x1F);
		mode = (data >> 5) & 1;
		mirror = (data >> 6) & 1;
	}
	sync();
}

static void reset(int hard)
{
	int i;

	mem_setvramsize(8);
	for(i=8;i<16;i++) {
		mem_setwritefunc(i,write);	
	}
	if(hard) {
		prg = mode = mirror = 0;
	}
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_U8(prg);
	STATE_U8(mode);
	STATE_U8(mirror);
	sync();
}

MAPPER(B_BMC_76IN1,reset,0,0,state);
