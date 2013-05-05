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

static u8 select,mode;
static u8 outerprg,prg,chr;

static void sync()
{
	int banklo,bankhi,outerbank,bank,shift,mask;

	shift = (mode >> 4) & 3;
	mask = (1 << shift) - 1;
	outerbank = (outerprg & ~mask) | (prg & mask);
	bank = ((outerprg & ~mask) << 1) | (prg & ((mask << 1) | 1));
	//prg bank mode
	switch((mode >> 2) & 3) {
		//32kb mode
		case 0:
		case 1:
			banklo = (outerbank << 1) | 0;
			bankhi = (outerbank << 1) | 1;
			break;
		//swap out C000-FFFF 
		case 2:
			banklo = (outerprg << 1) | 0;
			bankhi = bank;
			break;
		//swap out 8000-BFFF
		case 3:
			banklo = bank;
			bankhi = (outerprg << 1) | 1;
			break;
	}
	mem_setprg16(0x8,banklo);
	mem_setprg16(0xC,bankhi);
	mem_setvram8(0,chr);
	switch(mode & 3) {
		case 0:	mem_setmirroring(MIRROR_1L);	break;
		case 1:	mem_setmirroring(MIRROR_1H);	break;
		case 2:	mem_setmirroring(MIRROR_V);	break;
		case 3:	mem_setmirroring(MIRROR_H);	break;
	}
}

static void write_select(u32 addr,u8 data)
{
	select = data;
}

static void write_register(u32 addr,u8 data)
{
	switch(select) {
		case 0x00:
			chr = data & 3;
			if((mode & 2) == 0)
				mode = (mode & 0xFE) | ((data >> 4) & 1);
			break;
		case 0x01:
			prg = data & 0xF;
			if((mode & 2) == 0)
				mode = (mode & 0xFE) | ((data >> 4) & 1);
			break;
		case 0x80:
			mode = data & 0x3F;
			break;
		case 0x81:
			outerprg = data & 0x3F;
			break;
	}
	sync();
}

static void reset(int hard)
{
	int i;

	mem_setvramsize(8 * 4);
	mem_setwritefunc(5,write_select);
	for(i=8;i<16;i++)
		mem_setwritefunc(i,write_register);
	if(hard) {
		select = 0;
		prg = 0xF;
		chr = 3;
		mode = 0x3F;
		outerprg = 0x3F;
	}
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_U8(select);
	STATE_U8(mode);
	STATE_U8(outerprg);
	STATE_U8(prg);
	STATE_U8(chr);
	sync();
}

MAPPER(B_TEPPLES,reset,0,0,state);
