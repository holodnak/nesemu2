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

static void sync()
{
	mem_setprg32(8,(reg >> 4) | reg);
	if(nes->cart->chr.size == 0)
		mem_setvram8(0,0);
	else
		mem_setchr8(0,0);
}

static u8 read(u32 addr)
{
	if(addr == 0x5FF0)
		return(0x50);
	return(0);
}

static void write(u32 addr,u8 data)
{
//	log_printf("txc-mxmdhtwo:  write to $%04X = $%02X\n",addr,data);
	reg = data;
	sync();
}

static void reset(int hard)
{
	int i;

	for(i=8;i<0x10;i++)
		mem_setwritefunc(i,write);
	mem_setreadfunc(5,read);
	mem_setvramsize(8);
	mem_setwramsize(8);
	mem_setwram8(6,0);
	reg = 0;
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_U8(reg);
	sync();
}

MAPPER(B_TXC_MXMDHTWO,reset,0,0,state);
