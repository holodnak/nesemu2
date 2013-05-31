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

static u8 mode,bank;

static void sync()
{
	if(mode & 1)
		mem_setprg8(0x6,bank | 0x23);
	else
		mem_setprg8(0x6,bank | 0x2F);
	if(mode == 2)
		mem_setprg16(0x8,(bank >> 1) | 1);
	else
		mem_setprg16(0x8,bank >> 1);
	if(mode & 1)
		mem_setprg16(0xC,(bank >> 1) | 1);
	else
		mem_setprg16(0xC,(bank >> 1) | 7);
	mem_setmirroring((mode == 3) ? MIRROR_H : MIRROR_V);
}

static void write67(u32 addr,u8 data)
{
	mode = ((data >> 3) & 2) | ((data >> 1) & 1);
	sync();
}

static void write_upper(u32 addr,u8 data)
{
	bank = (data & 0xF) << 2;
	if(bank & 4)
		mode = ((data >> 3) & 2) | (mode & 1);
	sync();
}

static void reset(int hard)
{
	int i;

	for(i=6;i<8;i++)
		mem_setwritefunc(i,write67);
	for(i=8;i<16;i++)
		mem_setwritefunc(i,write_upper);
	mem_setvramsize(8);
	mem_setvram8(0,0);
	mode = 1;
	bank = 0;
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_U8(mode);
	STATE_U8(bank);
	sync();
}

MAPPER(B_BMC_BALLGAMES11IN1,reset,0,0,state);
