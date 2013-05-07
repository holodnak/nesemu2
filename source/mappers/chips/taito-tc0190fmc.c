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

static u8 prg[2],chr[6];
static u8 mirror;

void tc0190fmc_sync()
{
	mem_setprg8(0x8,prg[0]);
	mem_setprg8(0xA,prg[1]);
	mem_setprg16(0xC,0xFF);
	mem_setchr2(0,chr[0]);
	mem_setchr2(2,chr[1]);
	mem_setchr1(4,chr[2]);
	mem_setchr1(5,chr[3]);
	mem_setchr1(6,chr[4]);
	mem_setchr1(7,chr[5]);
	mem_setmirroring(mirror);
}

void tc0190fmc_write(u32 addr,u8 data)
{
	switch(addr & 0xA003) {
		case 0x8000:
			prg[0] = data & 0x3F;
			mirror = ((data >> 6) & 1) ^ 1;
			break;
		case 0x8001:
			prg[1] = data;
			break;
		case 0x8002:
		case 0x8003:
			chr[addr & 1] = data;
			break;
		case 0xA000:
		case 0xA001:
		case 0xA002:
		case 0xA003:
			chr[(addr & 3) + 2] = data;
			break;
	}
	tc0190fmc_sync();
}

void tc0190fmc_reset(int hard)
{
	int i;

	for(i=8;i<0xC;i++)
		mem_setwritefunc(i,tc0190fmc_write);
	prg[0] = prg[1] = 0;
	for(i=0;i<6;i++) {
		chr[i] = 0;
	}
	mirror = 0;
	tc0190fmc_sync();
}

void tc0190fmc_state(int mode,u8 *data)
{
	STATE_ARRAY_U8(prg,2);
	STATE_ARRAY_U8(chr,6);
	STATE_U8(mirror);
	tc0190fmc_sync();
}
