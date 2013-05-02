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
#include "mappers/chips/namcot-108.h"

static void sync()
{
	u8 *reg = namcot108_getregs();

	namcot108_sync();
	mem_setnt1(0x8,reg[0] >> 5);
	mem_setnt1(0x9,reg[0] >> 5);
	mem_setnt1(0xA,reg[1] >> 5);
	mem_setnt1(0xB,reg[1] >> 5);
	mem_setnt1(0xC,reg[2] >> 5);
	mem_setnt1(0xD,reg[3] >> 5);
	mem_setnt1(0xE,reg[4] >> 5);
	mem_setnt1(0xF,reg[5] >> 5);
}

static void reset(int hard)
{
	namcot108_reset(sync,hard);
}

MAPPER(B_NAMCOT_3425,reset,0,0,namcot108_state);
