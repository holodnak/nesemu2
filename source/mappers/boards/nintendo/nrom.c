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

static void reset_nrom(int hard)
{
	mem_setprg16(0x8,0);
	mem_setprg16(0xC,-1);
	if(nes->cart->chr.size)
		mem_setchr8(0,0);
	else
		mem_setvram8(0,0);
}

static void reset_nrom_sram(int hard)
{
	reset_nrom(hard);
	mem_setsramsize(2);
	mem_setsram8(6,0);
}

MAPPER(B_NINTENDO_NROM,reset_nrom,0,0,0);
MAPPER(B_NINTENDO_NROM_SRAM,reset_nrom_sram,0,0,0);
