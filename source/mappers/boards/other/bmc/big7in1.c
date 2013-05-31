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
static writefunc_t writeA;

static void sync()
{
	if(reg >= 6) {
		mmc3_syncprg(0x1F,reg << 4);
		mmc3_syncchr(0xFF,reg << 7);
	}
	else {
		mmc3_syncprg(0x0F,reg << 4);
		mmc3_syncchr(0x7F,reg << 7);
	}
	mmc3_syncsram();
	mmc3_syncmirror();
}

static void write(u32 addr,u8 data)
{
	if(addr & 1) {
		reg = data & 7;
		sync();
	}
	writeA(addr,data);
}

static void reset(int hard)
{
	mmc3_reset(C_MMC3,sync,hard);
	reg = 0;
	writeA = mem_getwritefunc(0xA);
	mem_setwritefunc(0xA,write);
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_U8(reg);
	mmc3_state(mode,data);
}

MAPPER(B_BMC_BIG7IN1,reset,mmc3_ppucycle,0,state);
