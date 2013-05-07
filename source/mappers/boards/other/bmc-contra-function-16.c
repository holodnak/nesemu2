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
	int bank,banks[4],x;

	bank = (latch_data << 1) & 0x7E;
	x = (latch_data >> 7) & 1;
	switch(latch_addr & 3) {
		case 0:
			banks[0] = bank | 0;
			banks[1] = bank | 1;
			banks[2] = bank | 2;
			banks[3] = bank | 3;
			break;
		case 1:
			banks[0] = bank | 0;
			banks[1] = bank | 1;
			banks[2] = 0x7E;
			banks[3] = 0x7F;
			break;
		case 2:
			banks[0] = bank;
			banks[1] = bank;
			banks[2] = bank;
			banks[3] = bank;
			break;
		case 3:
			banks[0] = bank | 0;
			banks[1] = bank | 1;
			banks[2] = bank | 0;
			banks[3] = bank | 1;
			break;
	}
	mem_setprg8(0x8,banks[0] ^ x);
	mem_setprg8(0xA,banks[1] ^ x);
	mem_setprg8(0xC,banks[2] ^ x);
	mem_setprg8(0xE,banks[3] ^ x);
	mem_setmirroring((latch_data & 0x40) ? MIRROR_H : MIRROR_V);
}

static void reset(int hard)
{
	mem_setvramsize(8);
	mem_setvram8(0,0);
	latch_init(sync);
}

MAPPER(B_BMC_CONTRA_FUNCTION_16,reset,0,0,latch_state);
