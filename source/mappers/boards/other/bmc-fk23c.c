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

static u8 reg[8];

static void sync()
{
	//handle prg
	if((reg[0] & 7) == 4) {
		mem_setprg32(8,reg[1] >> 1);
	}
	else if((reg[0] & 7) == 3) {
		mem_setprg16(0x8,reg[1]);
		mem_setprg16(0xC,reg[1]);
	}
	else {
		mmc3_syncprg(0xFF,0);
		if(reg[3] & 2) {
			mem_setprg8(0xC,reg[4]);
			mem_setprg8(0xE,reg[5]);
		}
	}

	//handle chr
	if(reg[0] & 0x40) {
		mem_setchr8(0,reg[2]);
	}
	else {
		mmc3_syncchr(0xFF,0);
		if(reg[3] & 2) {
			mem_setchr1(1,reg[6] | ((reg[2] & 0x7F) << 3));
			mem_setchr1(3,reg[7] | ((reg[2] & 0x7F) << 3));
		}
	}
	mmc3_syncchr(0xFF,0);
	mmc3_syncmirror();
/*	if(nes.cart->chr.size)
		mmc3_syncchr(0xFF,0);
	else
		mmc3_syncvram(7,0);
	if(nes.cart->mirroring & 8)
	   mem_setmirroring(MIRROR_4);
	else
		mmc3_syncmirror();

	mmc3_syncsram();
	mmc3_syncprg(0xFF,0);
	mmc3_syncchr(0xFF,0);
	mmc3_syncmirror();
	mem_setprg32(8,0);
	mem_setprg8(0xE,0);*/
}

static void write_5000(u32 addr,u8 data)
{
	log_printf("bmc-fk23c.c:  write_5000:  $%04X = $%02X\n",addr,data);
}

static void write(u32 addr,u8 data)
{
	log_printf("bmc-fk23c.c:  write:  $%04X = $%02X\n",addr,data);
}

static void reset(int hard)
{
	int i;

	for(i=0;i<4;i++) {
		reg[i+0] = 0;
		reg[i+4] = 0xFF;
	}
	mmc3_reset(C_MMC3,sync,hard);
	mem_setwritefunc(5,write_5000);
	for(i=8;i<16;i++)
		mem_setwritefunc(i,write);

}

static void state(int mode,u8 *data)
{
	STATE_ARRAY_U8(reg,8);
	mmc3_state(mode,data);
}

MAPPER(B_BMC_FK23C,reset,0,0,mmc3_ppucycle,state);
