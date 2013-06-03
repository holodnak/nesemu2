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

static u8 prg,outerprg;

static void sync()
{
	mem_setprg16(0x8,outerprg | prg);
	mem_setprg16(0xC,outerprg | 0xF);
	mem_setvram8(0,0);
}

static void write(u32 addr,u8 data)
{
	if(addr < 0xA000) {
		if(data & 8)
			outerprg = (data << 4) & 0x70;
	}
	else if(addr >= 0xC000) {
		prg = data & 0xF;
	}
	else
		return;
	sync();
}

static void reset(int hard)
{
	int i;

	for(i=8;i<16;i++) {
		if(i == 0xA || i == 0xB)
			continue;
		mem_setwritefunc(i,write);
	}
	mem_setvramsize(8);
	prg = outerprg = 0;
	sync();
}

static void state(int mode,u8 *data)
{
	sync();
}

MAPPER(B_CAMERICA_GOLDENFIVE,reset,0,0,state);
