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

static u8 mode = 0;

static void sync()
{
	//multicart mode
	if(mode) {
		if(latch_data & 0x20) {
			mem_setprg16(0x8,(latch_data & 0x1F) + 8);
			mem_setprg16(0x8,(latch_data & 0x1F) + 8);
		}
		else {
			mem_setprg32(8,((latch_data >> 1) & 0xF) + 4);
		}
		mem_setmirroring((latch_data >> 6) & 1);
	}

	//contra mode
	else {
		mem_setprg16(0x8,latch_data);
		mem_setprg16(0xC,7);
		mem_setmirroring(MIRROR_V);
	}
	mem_setvram8(0,0);
}

static void reset(int hard)
{
	if(hard)
		mode = 1;
	mode ^= 1;
	mem_setvramsize(8);
	latch_reset(sync,hard);
}

MAPPER(B_BMC_22IN1,reset,0,0,latch_state);
