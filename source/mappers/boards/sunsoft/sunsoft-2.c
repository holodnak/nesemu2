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

static void sync()
{
	mem_setprg16(0x8,(latch_data >> 4) & 7);
	mem_setprg16(0xC,0xFF);
	if(nes->cart->chr.size == 0) {
		mem_setvram8(0,0);
	}
	else {
		mem_setchr8(0,(latch_data & 7) | ((latch_data >> 4) & 8));
		mem_setmirroring(MIRROR_1L + ((latch_data >> 3) & 1));
	}
}

static void reset(int hard)
{
	if(nes->cart->chr.size == 0) {
		mem_setvramsize(8);
	}
	latch_init(sync);
}

MAPPER(B_SUNSOFT_2,reset,0,0,latch_state);
