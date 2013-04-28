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

#define BMC_70IN1		0
#define BMC_70IN1B	1

static int revision;
static u8 mode,mirror;
static u8 bankhi,banklo,chrbank;

static void sync()
{
	switch(mode) {
		case 0x00:
		case 0x10:
			mem_setprg16(0x8,bankhi | banklo);
			mem_setprg16(0xC,bankhi | 7);
			break;
		case 0x20:
			mem_setprg32(0x8,(bankhi | banklo) >> 1);
			break;
		case 0x30:
			mem_setprg16(0x8,bankhi | banklo);
			mem_setprg16(0xC,bankhi | banklo);
			break;
	}
	if(revision == BMC_70IN1)
		mem_setchr8(0,chrbank);
	else
		mem_setvram8(0,0);
	mem_setmirroring(mirror);
}

static void write_reg(u32 addr,u8 data)
{
	if(addr & 0x4000) {
		mode = addr & 0x30;
		banklo = addr & 7;
	}
	else {
		mirror = ((addr & 0x20) >> 5) ^ 1;
		if(revision == BMC_70IN1B)
			bankhi = (addr & 3) << 3;
		else
			chrbank = addr & 7;	
	}
	sync();
}

static void reset(int r,int hard)
{
	int i;

	revision = r;
	for(i=8;i<16;i++) {
//		mem_setread(i,read);
		mem_setwritefunc(i,write_reg);
	}
	mem_setvramsize(8);
	mode = 0;
	bankhi = banklo = 0;
	sync();
//	hw_switch++;
//	hw_switch &= 0xF;
}

static void reset_70in1(int hard)
{
	reset(BMC_70IN1,hard);
}

static void reset_70in1b(int hard)
{
	reset(BMC_70IN1B,hard);
}

static void state(int mode,u8 *data)
{
	STATE_U8(mode);
	STATE_U8(mirror);
	STATE_U8(bankhi);
	STATE_U8(banklo);
	STATE_U8(chrbank);
	sync();
}

MAPPER(B_70IN1,reset_70in1,0,0,0,state);
MAPPER(B_70IN1B,reset_70in1b,0,0,0,state);
