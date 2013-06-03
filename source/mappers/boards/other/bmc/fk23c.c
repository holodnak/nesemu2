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

/*
	mmc3 extra commands:
		8  - prg 8kb bank at $C
		9  - prg 8kb bank at $E
		10 - chr 1kb bank at $400
		11 - chr 1kb bank at $C00

		** these are enabled with reg3 bit1 **

	reg0: xxxx-xMMM
	   M - prg mode.
		    4 = 32kb mode
			 3 = 16kb mode
			 All others use mmc3 standard

	reg3: xxxx-xxCx
		C - enable $400/$C00 kbyte banks and swapping prg of $C/$E
*/

static u8 reg[8],chr;

static void syncprg(int a,int o)
{
	int i,n;

	n = reg[0] & 7;
	if(n == 4 || n == 3)
		return;
	for(i=0;i<4;i++) {
		if((reg[3] & 2) == 0 || i < 2) {
			n = mmc3_getprgbank(i);
			if (reg[0] & 3)
				n = (n & (0x3F >> (reg[0] & 3))) | (reg[1] << 1);
			mem_setprg8(8 + (i * 2),n);
		}
		else {
			if(i == 2)
				mem_setprg8(0xC,reg[4]);
			else if(i == 3)
				mem_setprg8(0xE,reg[5]);
		}
	}
}

static void syncchr(int a,int o)
{
	int i;

	for(i=0;i<8;i++) {
		if((reg[0] & 0x40)) {
			if((reg[3] & 2) && (i == 1 || i == 3)) {
				mem_setchr1(1,((reg[2] & 0x7F) << 3) | (reg[6 + ((i - 1) / 2)] & 0xFF));
				continue;
			}
		}
		mem_setchr1(i,(reg[2] & 0x7FU) << 3 | mmc3_getchrbank(i));
	}
}

static void sync()
{
	if((reg[0] & 7) == 4) {
		mem_setprg32(8,reg[1] >> 1);
	}
	else if((reg[0] & 7) == 3) {
		mem_setprg16(0x8,reg[1]);
		mem_setprg16(0xC,reg[1]);
	}
	else {
		syncprg(0xFF,0x00);
	}
	if(reg[0] & 0x40) {
		mem_setchr8(0,reg[2] | chr);
	}
	else {
		syncchr(0x3FF,0x00);
	}
	mmc3_syncsram();
	mmc3_syncmirror();
}

static void write_5000(u32 addr,u8 data)
{
	switch(addr) {
		case 0x5FF0:
		case 0x5FF1:
		case 0x5FF2:
		case 0x5FF3:
			reg[addr & 3] = data;
			sync();
			break;
	}
}

static void write(u32 addr,u8 data)
{
	u8 command;

	if(reg[0] & 0x40) {
		chr = (reg[0] & 0x30) ? 0 : data & 3;
		sync();
		return;
	}
	switch(addr & 0xE001) {
		case 0x8000:	mmc3_write(addr,data);	break;
		case 0x8001:
			command = mmc3_getcommand() & 0xF;
			if(command < 8) {
				mmc3_write(addr,data);
			}
			else if(reg[3] << 2) {
				switch(command) {
					case 8:
					case 9:
					case 10:
					case 11:
						reg[4 + (command & 3)] = data;
						sync();
						break;
				}
			}
			break;
		case 0xA000:	mmc3_write(addr,data);		break;
		case 0xA001:	mmc3_write(addr,data);		break;
		case 0xC000:	mmc3_write(addr,data);		break;
		case 0xC001:	mmc3_write(addr,data);		break;
		case 0xE000:	mmc3_write(addr,data);		break;
		case 0xE001:	mmc3_write(addr,data);		break;
	}
}

static void reset(int hard)
{
	int i;

	for(i=0;i<8;i++) {
		reg[i] = 0xFF;
	}
	chr = 0;
	mem_setvramsize(8);
	mmc3_reset(C_MMC3B,sync,hard);
	mem_setwritefunc(5,write_5000);
	for(i=8;i<16;i++)
		mem_setwritefunc(i,write);
}

static void state(int mode,u8 *data)
{
	STATE_ARRAY_U8(reg,8);
	STATE_U8(chr);
	mmc3_state(mode,data);
}

MAPPER(B_BMC_FK23C,reset,mmc3_ppucycle,0,state);
