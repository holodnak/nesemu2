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

static u8 reg[2];

static void sync()
{
	mmc3_syncprg(0xFF,0x00);
	if(reg[0] & 0x80) {
		mem_setprg16(8,reg[0] & 0xF);
	}
	mmc3_syncchr(0xFF,(reg[1] & 1) << 8);
	mmc3_syncmirror();
}

static void write(u32 addr,u8 data)
{
	log_printf("other/kasing.c:  write $%04X = $%02X\n");
	reg[addr & 1] = data;
	sync();
}

static void reset(int hard)
{
	if(hard)
		reg[0] = reg[1] = 0;
	mmc3_reset(C_MMC3B,sync,hard);
	mem_setwritefunc(6,write);
	mem_setwritefunc(7,write);
}

static void state(int mode,u8 *data)
{
	STATE_ARRAY_U8(reg,2);
	mmc3_state(mode,data);
}

MAPPER(B_KASING,reset,mmc3_ppucycle,0,state);
