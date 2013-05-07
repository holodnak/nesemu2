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
#include "mappers/chips/latch.h"

static int revision;
static u8 reg[4];
static readfunc_t read4;
static writefunc_t write4;

static void sync()
{
	mem_setprg32(8,(reg[2] >> 2) & 3);
	if(revision == B_TXC_22211B)
		mem_setchr8(0,(((latch_data ^ reg[2]) >> 3) & 2) | (((latch_data ^ reg[2]) >> 5) & 1));
	else
		mem_setchr8(0,reg[2] & 3);
}

static u8 read_reg(u32 addr)
{
	if(addr < 0x4020)
		return(read4(addr));
	if(addr == 0x4100) {
		u8 o = 0x40;

		if(revision == B_TXC_22211C)
			o = 0x41;
		return((reg[1] ^ reg[2]) | o);
	}
	return(0);
}

static void write_reg(u32 addr,u8 data)
{
	if(addr < 0x4020)
		write4(addr,data);
	else if(addr >= 0x4100 && addr < 0x4104)
		reg[addr & 3] = data;
}

static void reset(int r,int hard)
{
	revision = r;
	read4 = mem_getreadfunc(4);
	write4 = mem_getwritefunc(4);
	mem_setreadfunc(4,read_reg);
	mem_setwritefunc(4,write_reg);
	reg[0] = reg[1] = reg[2] = reg[3] = 0;
	latch_init(sync);
	mem_setprg32(8,0);
}

static void state(int mode,u8 *data)
{
	STATE_ARRAY_U8(reg,4);
	latch_state(mode,data);
}

static void reset_txc_22211a(int hard)	{	reset(B_TXC_22211A,hard);	}
static void reset_txc_22211b(int hard)	{	reset(B_TXC_22211B,hard);	}
static void reset_txc_22211c(int hard)	{	reset(B_TXC_22211C,hard);	}

MAPPER(B_TXC_22211A,reset_txc_22211a,0,0,state);
MAPPER(B_TXC_22211B,reset_txc_22211b,0,0,state);
MAPPER(B_TXC_22211C,reset_txc_22211c,0,0,state);
