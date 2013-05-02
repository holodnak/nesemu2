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
#include "mappers/chips/namcot-108.h"

static u8 mirror;

static void sync_34x3()
{
	u8 *reg = namcot108_getregs();

	mem_setchr2(0,(reg[0] & 0x3F) >> 1);
	mem_setchr2(2,(reg[1] & 0x3F) >> 1);
	mem_setchr1(4,reg[2] | 0x40);
	mem_setchr1(5,reg[3] | 0x40);
	mem_setchr1(6,reg[4] | 0x40);
	mem_setchr1(7,reg[5] | 0x40);
	mem_setprg8(0x8,reg[6]);
	mem_setprg8(0xA,reg[7]);
	mem_setprg16(0xC,0xFF);
}

static void sync_3453()
{
	sync_34x3();
	mem_setmirroring(mirror);
}

static void write_3453(u32 addr,u8 data)
{
	mirror = MIRROR_1L + ((data & 0x40) >> 6);
	namcot108_write(addr,data);
}

static void reset_34x3(int hard)
{
	namcot108_reset(sync_34x3,hard);
}

static void reset_3453(int hard)
{
	int i;

	namcot108_reset(sync_3453,hard);
	for(i=8;i<16;i++)
		mem_setwritefunc(i,write_3453);
}

MAPPER(B_NAMCOT_34x3,reset_34x3,0,0,namcot108_state);
MAPPER(B_NAMCOT_3453,reset_3453,0,0,namcot108_state);
