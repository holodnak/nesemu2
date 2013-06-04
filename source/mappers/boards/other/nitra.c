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
#include "mappers/chips/mmc3.h"

static void write(u32 addr,u8 data)
{
//	data = addr & 0xFF;
//	addr = (addr & 0xE000) | (addr >> 10 & 1);
	mmc3_write((addr & 0xE000) | (addr >> 10 & 1),addr & 0xFF);
}

static void reset(int hard)
{
	int i;

	mmc3_reset(C_MMC3B,mmc3_sync,hard);
	for(i=8;i<16;i++)
		mem_setwritefunc(i,write);
}

MAPPER(B_NITRA,reset,mmc3_ppucycle,0,mmc3_state);
