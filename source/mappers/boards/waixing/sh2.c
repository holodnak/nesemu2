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

static u8 latch[2][2],latchstate[2];
static readfunc_t ppuread;

static INLINE void syncchr()
{
	u8 bank;

	bank = mmc3_getchrreg(latchstate[0] ? 2 : 0) >> 2;
	if(bank == 0)
		mem_setvram4(0,0);
	else
		mem_setchr4(0,bank);
	bank = mmc3_getchrreg(latchstate[1] ? 6 : 4) >> 2;
	if(bank == 0)
		mem_setvram4(4,0);
	else
		mem_setchr4(4,bank);
}

static void sync()
{
	mmc3_syncprg(0xFF,0);
	syncchr();
	mem_setmirroring(mmc3_getmirror() ^ 1);
	mmc3_syncsram();
}

static u8 readtile(u32 addr)
{
	switch(addr & 0xFFF8) {
		case 0x0FD8:
			latchstate[0] = 0;
			syncchr();
			break;
		case 0x0FE8:
			latchstate[0] = 1;
			syncchr();
			break;
		case 0x1FD8:
			latchstate[1] = 0;
			syncchr();
			break;
		case 0x1FE8:
			latchstate[1] = 1;
			syncchr();
			break;
	}
	return(ppuread(addr));
}

static void reset(int hard)
{
	mem_setvramsize(4);
	latch[0][0] = latch[0][1] = 0;
	latch[1][0] = latch[1][1] = 0;
	latchstate[0] = latchstate[1] = 0;
	ppuread = ppu_getreadfunc();
	ppu_setreadfunc(readtile);
	mmc3_reset(C_MMC3B,sync,hard);
}

static void ppucycle()
{
	mmc3_ppucycle();
}

static void state(int mode,u8 *data)
{
	mmc3_state(mode,data);
}

MAPPER(B_WAIXING_SH2,reset,ppucycle,0,state);
