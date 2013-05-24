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

static readfunc_t read4;

static u8 read(u32 addr)
{
	if(addr < 0x4020)
		return(read4(addr));
	if(addr & 0x100)
		return((~addr & 0x3F) | (nes->cpu.x & 0xC0));
	return(0xFF);
}

static void reset(int hard)
{
	read4 = mem_getreadfunc(4);
	mem_setreadfunc(4,read);
	mem_setreadfunc(5,read);
	mem_setprg32(8,0);
	mem_setchr8(0,0);
}

MAPPER(B_SACHEN_TCA01,reset,0,0,0);
