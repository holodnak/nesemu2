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

static u8 *prg6;
static u8 *sram7;

//reading
static u8 read6(u32 addr)	{	return(prg6[addr & 0x7FF]);	}
static u8 read7(u32 addr)	{	return(sram7[addr & 0x7FF]);	}

//writing
static void write7(u32 addr,u8 data)	{	sram7[addr & 0x7FF] = data;	}

static void reset(int hard)
{
	mem_setsramsize(2);

	//initialize all bank pointers
	mem_setprg4(6,8);
	mem_setsram4(7,0);
	mem_setprg32(8,0);
	mem_setchr8(0,0);

	//get pointers to the data
	prg6 = mem_getreadptr(6);
	sram7 = mem_getreadptr(7);

	//remove pointers to memory
	mem_setreadptr(6,0);
	mem_setreadptr(7,0);
	mem_setwriteptr(7,0);

	//insert the function pointers from here
	mem_setreadfunc(6,read6);
	mem_setreadfunc(7,read7);
	mem_setwritefunc(7,write7);
}

MAPPER(B_MARIO1_MALEE2,reset,0,0,0,0);
