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
	u8 prg;

	prg = (latch_addr & 0x1F) << 1;
	prg |= (latch_addr & 0x300) >> 2;
	prg |= (latch_addr & 0x1000) >> 12;
	if(latch_addr & 0x800) {
		mem_setprg16(0x8,prg);
		mem_setprg16(0xC,prg);
	}
	else {
		mem_setprg32(0x8,prg >> 1);
	}
	mem_setvram8(0,0);
	if(latch_addr & 0x400)
		mem_setmirroring(MIRROR_1L);
	else
		mem_setmirroring(((latch_addr >> 13) & 1) ^ 1);
}

static void reset(int hard)
{
	mem_setvramsize(8);
	latch_reset(sync,hard);
}

MAPPER(B_BMC_GOLDENGAME260IN1,reset,0,0,latch_state);
