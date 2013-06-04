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

static u8 reg[3];
static readfunc_t oldread;

static void sync()
{
	if(reg[0] & 0x40) {
		mem_setprg32(8,(reg[0] & 0xE) | (reg[1] & 1));
		mem_setchr8(0,((reg[0] & 0xE) << 2) | ((reg[2] >> 4) & 7));
	}
	else {
		mem_setprg32(8,reg[0] & 0xF);
		mem_setchr8(0,((reg[0] & 0xF) << 2) | ((reg[2] >> 4) & 3));
	}
	mem_setmirroring((reg[0] >> 7) ^ 1);
}

static void write(u32 addr,u8 data)
{
	switch(addr & 0xFFF8) {
		case 0xFF80:
		case 0xFF88:
		case 0xFF90:
		case 0xFF98:
			if((reg[0] & 0x3F) == 0) {
				reg[0] = data;
				sync();
			}
			break;
		case 0xFFE8:
		case 0xFFF0:
			reg[2] = data;
			sync();
			break;
	}
}

static u8 read(u32 addr)
{
	u8 data = oldread(addr);

	if(addr >= 0x8000 && (addr & 0xF80) == 0xF80)
		write(addr,data);
	return(data);
}

static void reset(int hard)
{
	oldread = cpu_getreadfunc();
	cpu_setreadfunc(read);
	mem_setwritefunc(0xF,write);
	if(hard)
		reg[0] = reg[1] = reg[2] = 0;
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_ARRAY_U8(reg,3);
	sync();
}

MAPPER(B_AVE_D1012,reset,0,0,state);
