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

static u8 reg[4];
static u8 regindex;

static void sync()
{
	mmc3_syncprg(~reg[3] & 0x3F,reg[1]);
	if(nes->cart->chr.size)
		mmc3_syncchr(0xFF >> ((~reg[2]) & 0xF),reg[0] | ((reg[2] & 0xF0) << 4));
	else
		mmc3_syncvram(7,0);
	mmc3_syncmirror();
	mmc3_syncsram();
	if((reg[3] & 0x40) == 0)
		mem_unsetcpu8(6);
}

static void write(u32 addr,u8 data)
{
	if((reg[3] & 0x40) == 0) {
		reg[regindex++] = data;
		regindex &= 3;
		sync();
	}
}

static void reset(int hard)
{
	mmc3_reset(C_MMC3B,sync,hard);
	mem_unsetcpu8(6);
	mem_setwritefunc(6,write);
	mem_setwritefunc(7,write);
	reg[0] = reg[1] = reg[2] = reg[3] = 0;
	regindex = 0;
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_ARRAY_U8(reg,4);
	STATE_U8(regindex);
	mmc3_state(mode,data);
}

MAPPER(B_BMC_SUPERHIKXIN1,reset,0,mmc3_ppucycle,state);
