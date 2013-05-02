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

static u8 reg;
static writefunc_t write4;
static void (*sync)() = 0;

static void sync_72007()
{
	mem_setprg16(0x8,0);
	mem_setprg16(0xC,1);
	mem_setchr8(0,reg >> 7);
}

static void sync_72008()
{
	mem_setprg32(8,(reg >> 2) & 1);
	mem_setchr8(0,reg & 3);
}

static void write45(u32 addr,u8 data)
{
	if(addr < 0x4020) {
		write4(addr,data);
		return;
	}
	if(addr & 0x100) {
		reg = data;
		sync();
	}
}

static void reset(int type,int hard)
{
	if(type == B_SA_72007)
		sync = sync_72007;
	else
		sync = sync_72008;
	write4 = mem_getwritefunc(4);
	mem_setwritefunc(4,write45);
	mem_setwritefunc(5,write45);
	reg = 0;
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_U8(reg);
	sync();
}

static void reset_72007(int hard)	{	reset(B_SA_72007,hard);	}
static void reset_72008(int hard)	{	reset(B_SA_72008,hard);	}

MAPPER(B_SA_72007,reset_72007,0,0,state);
MAPPER(B_SA_72008,reset_72008,0,0,state);
