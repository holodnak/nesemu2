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
	u8 scramble[8] = {0,3,1,5,6,7,2,4};

	switch(addr & 0xE001) {
		case 0x8000:
			break;
		case 0x8001:
			mmc3_write(0xA000,data);
			break;
		case 0xA000:
			mmc3_write(0x8000,scramble[data & 7] | (data & 0xC0));
			break;
		case 0xA001:
			break;
		case 0xC000:
			mmc3_write(0x8001,data);
			break;
		case 0xC001:
			mmc3_write(0xC000,data);
			mmc3_write(0xC001,data);
			break;
		case 0xE000:
		case 0xE001:
			mmc3_write(addr,data);
			break;
	}
}

static void reset(int hard)
{
	int i;

	mmc3_reset(C_MMC3B,mmc3_sync,hard);
	for(i=8;i<16;i++)
		mem_setwritefunc(i,write);
}

MAPPER(B_HOSENKAN,reset,mmc3_ppucycle,0,mmc3_state);
