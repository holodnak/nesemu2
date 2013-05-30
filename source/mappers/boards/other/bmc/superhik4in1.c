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
#include "mappers/chips/mmc3.h"

static u8 reg;

static void sync()
{
	if(reg & 1)
		mmc3_syncprg(0xF,(reg & 0xC0) >> 2);
	else
		mem_setprg32(0x8,(reg & 0x30) >> 4);
	mmc3_syncchr(0x7F,(reg & 0xC0) << 1);
	mmc3_syncmirror();
}

static void write(u32 addr,u8 data)
{
	reg = data;
	sync();
}

static void reset(int hard)
{
	mmc3_reset(C_MMC3B,sync,hard);
	reg = 0;
	mem_setwritefunc(6,write);
	mem_setwritefunc(7,write);
}

static void state(int mode,u8 *data)
{
	STATE_U8(reg);
	mmc3_state(mode,data);
}

MAPPER(B_BMC_SUPERHIK4IN1,reset,0,mmc3_ppucycle,state);
