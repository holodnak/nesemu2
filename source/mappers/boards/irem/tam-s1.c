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
	mem_setprg16(0x8,0xFF);
	mem_setprg16(0xC,latch_reg);
	mem_setvram8(0,0);
	switch(latch_reg >> 6) {
		case 0: mem_setmirroring(MIRROR_1L);	break;
		case 1: mem_setmirroring(MIRROR_H);		break;
		case 2: mem_setmirroring(MIRROR_V);		break;
		case 3: mem_setmirroring(MIRROR_1H);	break;
	}
}

static void reset(int hard)
{
	mem_setvramsize(8);
	latch_init(sync);
}

MAPPER(B_IREM_TAM_S1,reset,0,0,0,latch_state);
