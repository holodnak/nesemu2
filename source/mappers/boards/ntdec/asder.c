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

static u8 command,reg[10];

static void sync()
{
	int chrhi = 0;

	if(reg[9] & 2)
		chrhi = reg[8];
	mem_setprg8(0x8,reg[0]);
	mem_setprg8(0xA,reg[1]);
	mem_setprg8(0xC,0xFE);
	mem_setprg8(0xE,0xFF);
	mem_setchr2(0,(reg[2] | ((chrhi << 6) & 0x100)) >> 1);
	mem_setchr2(2,(reg[3] | ((chrhi << 5) & 0x100)) >> 1);
	mem_setchr1(4,reg[4] | ((chrhi << 4) & 0x100));
	mem_setchr1(5,reg[5] | ((chrhi << 3) & 0x100));
	mem_setchr1(6,reg[6] | ((chrhi << 2) & 0x100));
	mem_setchr1(7,reg[7] | ((chrhi << 1) & 0x100));
	mem_setmirroring((reg[9] & 1) ^ 1);
}

static void write(u32 addr,u8 data)
{
	switch(addr & 0xE000) {
		case 0x8000:
			command = data & 7;
			break;
		case 0xA000:
			switch(command) {
				case 0:
				case 1:
					reg[command] = data;
					break;
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
					reg[command] = data;
					break;
			}
			sync();
			break;
		case 0xC000:
			reg[8] = data;
			sync();
			break;
		case 0xE000:
			reg[9] = data;
			sync();
			break;
	}
}

static void reset(int hard)
{
	int i;

	for(i=8;i<16;i++)
		mem_setwritefunc(i,write);
	if(hard) {
		command = 0;
		for(i=0;i<10;i++)
			reg[i] = 0;
	}
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_ARRAY_U8(reg,10);
	sync();
}

MAPPER(B_NTDEC_ASDER,reset,0,0,state);
