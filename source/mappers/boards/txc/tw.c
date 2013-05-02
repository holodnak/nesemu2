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

static void sync()
{
	mem_setprg32(8,reg);
	mmc3_syncchr(0xFF,0);
	mmc3_syncmirror();
	mmc3_syncsram();
}

static u8 read(u32 addr)
{
//	log_printf("txc-tw:  read from $%04X\n",addr);
	if(addr == 0x5FF0)
		return(0x50);
	return(0);
}

static void write(u32 addr,u8 data)
{
	if(addr < 0x4020) {
		write4(addr,data);
		return;
	}
//	log_printf("txc-tw:  write to $%04X = $%02X\n",addr,data);
	reg = (data & 0xF) | (data >> 4);
	sync();
}

static void reset(int hard)
{
	write4 = mem_getwritefunc(4);
	mem_setreadfunc(5,read);
	mem_setwritefunc(4,write);
	reg = 0;
	mmc3_reset(C_MMC3A,sync,hard);
}

static void state(int mode,u8 *data)
{
	STATE_U8(reg);
	mmc3_state(mode,data);
	sync();
}

MAPPER(B_TXC_TW,reset,mmc3_ppucycle,0,state);
