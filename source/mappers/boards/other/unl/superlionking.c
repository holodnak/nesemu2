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

static u8 prg,valid;

static void sync()
{
	mmc3_syncprg(0xFF,0x00);
	mmc3_syncchr(0xFF,0x00);
	mmc3_syncmirror();
	if(prg & 0x80) {
		mem_setprg16(0x8,prg & 0x1F);
		mem_setprg16(0xC,prg & 0x1F);
	}
}

static void writelo(u32 addr,u8 data)
{
	prg = data;
	sync();
}

static void write(u32 addr,u8 data)
{
	u8 scramble[8] = {0,3,1,5,6,7,2,4};

	switch(addr & 0xE000) {
		case 0x8000:
			mmc3_write(0xA000,data);
			break;
		case 0xA000:
			mmc3_write(0x8000,scramble[data & 7] | (data & 0xC0));
			valid = 1;
			break;
		case 0xC000:
			if(valid) {
				mmc3_write(0x8001,data);
				valid = 0;
			}
			break;
		case 0xE000:
			if(addr == 0xE002) {
				mmc3_write(0xE000,data);
			}
			else if(addr == 0xE003) {
				mmc3_write(0xC000,data);
				mmc3_write(0xC001,data);
				mmc3_write(0xE001,data);
			}
			break;
	}
}

static void reset(int hard)
{
	int i;

	prg = 0;
	valid = 1;
	mmc3_reset(C_MMC3B,mmc3_sync,hard);
	for(i=8;i<16;i++)
		mem_setwritefunc(i,write);
	mem_setwritefunc(5,writelo);
	mem_setwritefunc(6,writelo);
	mem_setwritefunc(7,writelo);
}

static void state(int mode,u8 *data)
{
	STATE_U8(prg);
	STATE_U8(valid);
	mmc3_state(mode,data);
}

MAPPER(B_UNL_SUPERLIONKING,reset,mmc3_ppucycle,0,state);
