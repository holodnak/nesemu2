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

static void sync_0036()
{
	mem_setprg32(0x8,0);
	log_printf("sync_0036:  reg = %02X\n",reg);
	mem_setchr8(0,(reg >> 7));
}

static void sync_0037()
{
	mem_setprg32(8,(reg >> 3) & 1);
	log_printf("sync_0037:  reg = %02X\n",reg);
	mem_setchr8(0,reg & 7);
}

static void write45(u32 addr,u8 data)
{
	if(addr < 0x4020) {
		write4(addr,data);
		return;
	}
	log_printf("sa-003x.c:  write45:  $%04X = $%02X\n");
	if(addr & 0x100) {
		reg = data;
		sync();
	}
}

static void reset(int type,int hard)
{
	if(type == B_SA_0036)
		sync = sync_0036;
	else
		sync = sync_0037;
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

static void reset_0036(int hard)	{	reset(B_SA_0036,hard);	}
static void reset_0037(int hard)	{	reset(B_SA_0037,hard);	}

MAPPER(B_SA_0036,reset_0036,0,0,0,state);
MAPPER(B_SA_0037,reset_0037,0,0,0,state);
