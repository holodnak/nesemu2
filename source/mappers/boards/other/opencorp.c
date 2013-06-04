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

static u8 prg,chr[8],chrhi[8];

static void sync()
{
	int i;

	mem_setwram8(0x6,0);
	mem_setprg16(0x8,prg);
	mem_setprg16(0xC,0xFF);
	for(i=0;i<8;i++)
		mem_setchr1(i,chr[i] | (chrhi[i] << 8));
}

static void write(u32 addr,u8 data)
{
	switch(addr) {
		case 0xC000:
		case 0xC001:
		case 0xC002:
		case 0xC003:
			chr[addr & 3] = data;
			break;
		case 0xC004:
		case 0xC005:
		case 0xC006:
		case 0xC007:
			chrhi[addr & 3] = data;
			break;
		case 0xC008:
		case 0xC009:
		case 0xC00A:
		case 0xC00B:
			chr[(addr & 3) + 4] = data;
			break;
		case 0xC00C:
		case 0xC00D:
		case 0xC00E:
		case 0xC00F:
			chrhi[(addr & 3) + 4] = data;
			break;
		case 0xC010:
			prg = data;	
			break;
		default:
			log_printf("opencorp.c:  unhandled write $%04X = $%02X\n",addr,data);
			break;
	}
	sync();
}

static void reset(int hard)
{
	mem_setwramsize(8);
	mem_setwritefunc(0xC,write);
	prg = 0;
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_U8(prg);
	STATE_ARRAY_U8(chr,8);
	STATE_ARRAY_U8(chrhi,8);
	sync();
}

MAPPER(B_OPENCORP,reset,0,0,state);
