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
static writefunc_t write4;

static void mmc3_syncchrlo(int a,int o)
{
	int i;

	for(i=0;i<4;i++)
		mem_setchr1(i,(mmc3_getchrbank(i) & a) | o);
}

static void mmc3_syncchrhi(int a,int o)
{
	int i;

	for(i=4;i<8;i++)
		mem_setchr1(i,(mmc3_getchrbank(i) & a) | o);
}

static void sync()
{
	mmc3_syncprg(0xFF,0x00);
	mmc3_syncchrlo(0x1FF,(reg << 8) & 0x100);
	mmc3_syncchrhi(0x1FF,(reg << 4) & 0x100);
	mmc3_syncmirror();
	mmc3_syncsram();
}

static void write(u32 addr,u8 data)
{
	if(addr < 0x4020) {
		write4(addr,data);
		return;
	}
	reg = data;
}

static void reset(int hard)
{
	mmc3_reset(C_MMC3B,sync,hard);
	write4 = mem_getwritefunc(4);
	mem_setwritefunc(4,write);
	mem_setwritefunc(5,write);
	reg = 0;
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_U8(reg);
	mmc3_state(mode,data);
}

MAPPER(B_REXSOFT_DBZ5,reset,mmc3_ppucycle,0,state);
