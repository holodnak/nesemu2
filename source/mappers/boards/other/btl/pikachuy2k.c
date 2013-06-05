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

static u8 reg,xor;
static writefunc_t writeA;
static u8 *wram;

static void sync()
{
	mmc3_syncprg(0xFF,0);
	mmc3_syncchr(0xFF,0);
	mmc3_syncmirror();
}

static u8 read_wram(u32 addr)
{
	return(wram[addr & 0x1FFF] ^ (xor & reg));
}

static void write_wram(u32 addr,u8 data)
{
	wram[addr & 0x1FFF] = data;
}

static void write(u32 addr,u8 data)
{
	switch(addr & 0xF000) {
		case 0x8000:
		case 0x9000:
			if((addr & 1) == 0)
				reg = 0;
			break;
		case 0xA000:
			if(addr & 1)
				xor = data;
			break;
	}
	mmc3_write(addr,data);
}

static void reset(int hard)
{
	mmc3_reset(C_MMC3,sync,hard);
	reg = 0xFF;
	mem_setwramsize(2);
	wram = nes->cart->wram.data;
	mem_unsetcpu8(6);
	mem_setreadfunc(6,read_wram);
	mem_setreadfunc(7,read_wram);
	mem_setwritefunc(6,write_wram);
	mem_setwritefunc(7,write_wram);
	mem_setwritefunc(0x8,write);
	mem_setwritefunc(0x9,write);
	mem_setwritefunc(0xA,write);
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_U8(reg);
	STATE_U8(xor);
	mmc3_state(mode,data);
}

MAPPER(B_BTL_PIKACHUY2K,reset,mmc3_ppucycle,0,state);
