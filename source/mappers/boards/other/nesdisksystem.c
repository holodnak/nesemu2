/***************************************************************************
*   Copyright (C) 2013-2016 by James Holodnak                             *
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
static writefunc_t write4;

static u8 read(u32 addr)
{
	//default handler
	return(read4(addr));
}

static void write(u32 addr, u8 data)
{
	write4(addr, data);
}

static void reset(int hard)
{
	read4 = mem_getreadfunc(4);
	write4 = mem_getwritefunc(4);
	mem_setreadfunc(4, read);
	mem_setwritefunc(4, write);
	mem_setvramsize(8);
	mem_setwramsize(32);

	mem_setprg32(0x5, 0);
	mem_setprg4(0xF, 0xFF);
	mem_setvram8(0, 0);
}

MAPPER(B_NES_DISK_SYSTEM, reset, 0, 0, 0);
