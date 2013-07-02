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

static u8 reg[2];

static void sync()
{
	u8 prglo,prghi,chr;

	prglo = (reg[1] >> 5) & ~(reg[1] >> 7);
	prghi = (reg[1] >> 5) | (reg[1] >> 7);
	chr = ((reg[0] >> 3) & 8) | (reg[0] & 7) | (reg[1] & 7);
	mem_setprg16(0x8,prglo);
	mem_setprg16(0xC,prghi);
	mem_setchr8(0,chr);
	mem_setmirroring((reg[1] & 8) ? MIRROR_H : MIRROR_V);
}

static void write(u32 addr,u8 data)
{
	switch(addr & 0x8800) {
		case 0x8000:
			reg[0] = data;
			break;
		case 0x8800:
			reg[1] = data;
			break;
	}
	sync();
}

static void reset(int hard)
{
	int i;

	for(i=8;i<16;i++)
		mem_setwritefunc(i,write);
	reg[0] = reg[1] = 0;
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_ARRAY_U8(reg,2);
	sync();
}

MAPPER(B_BMC_SUPERGK,reset,0,0,state);
