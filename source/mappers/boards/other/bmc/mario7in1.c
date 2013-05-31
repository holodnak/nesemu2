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

static u8 reg,written;

static void sync()
{
	if(reg & 8) {
		mmc3_syncprg(0xF,(reg & 7) << 4);
	}
	else {
		mmc3_syncprg(0x1F,(reg & 6) << 4);
	}
	if(reg & 0x40) {
		mmc3_syncchr(0x7F,((reg & 0x20) << 4) | ((reg & 4) << 6) | ((reg & 0x10) << 3));
	}
	else {
		mmc3_syncchr(0xFF,((reg & 0x20) << 4) | ((reg & 4) << 6));
	}
	//kludge
	if(written) {
		mem_setwritefunc(6,0);
		mmc3_syncsram();
	}
	mmc3_syncmirror();
}

static void write(u32 addr,u8 data)
{
	if(written == 0)
		reg = data;
	written = 1;
	sync();
}

static void reset(int hard)
{
	if(hard) {
		mem_unsetcpu8(6);
		mem_setwritefunc(6,write);
		written = 0;
		reg = 0;
	}
	mmc3_reset(C_MMC3,sync,hard);
}

static void state(int mode,u8 *data)
{
	STATE_U8(reg);
	mmc3_state(mode,data);
}

MAPPER(B_BMC_MARIO7IN1,reset,mmc3_ppucycle,0,state);
