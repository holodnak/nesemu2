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

static u8 prg[2],chr[8],mirror,mode;
static writefunc_t write4;

static void syncprg()
{
	if(mode & 2) {
		mmc3_syncprg(0xFF,0x00);
	}
	else {
		mem_setprg8(0x8,prg[0]);
		mem_setprg8(0xA,prg[1]);
		mem_setprg8(0xC,0xFE);
		mem_setprg8(0xE,0xFF);
	}
}

static void syncchr()
{
	u8 modes[4] = {5,5,3,1};
	int i,bank;

	if(mode & 2) {
		for(i=0;i<8;i++) {
			bank = mmc3_getchrreg(i);
			bank |= mode << modes[(i >> 1) ^ ((mmc3_getcommand() >> 6) & 2)] & 0x100;
			mem_setchr1(i,bank);
		}
	}
	else {
		for(i=0;i<8;i++) {
			mem_setchr1(i,chr[i]);
		}
	}
}

static void sync()
{
	syncprg();
	syncchr();
	mmc3_syncsram();
	mmc3_syncmirror();
}

static void write_mode(u32 addr,u8 data)
{
	if(addr < 0x4020) {
		write4(addr,data);
		return;
	}
	log_printf("sl1632.c:  mode:  $%04X = $%02X\n",addr,data);
	if((addr & 0xFF00) == 0x4100) {
		log_printf("sl1632.c:  mode:  $%04X = $%02X\n",addr,data);
		mode = data;
		sync();
	}
}

static void write(u32 addr,u8 data)
{
	int n,shift;

	if((addr & 0xA131) == 0xA131) {
		log_printf("sl1632.c:  mode:  $%04X = $%02X\n",addr,data);
		mode = data;
	}

	if(mode & 2) {
		mmc3_write(addr,data);
	}
	else {
		if(addr >= 0xB000 && addr <= 0xE003) {
			shift = ((addr & 1) << 2);
			n = ((((addr & 2) | (addr >> 10)) >> 1) + 2) & 7;
			chr[n] = (chr[n] & (0xF0 >> shift)) | ((data & 0x0F) << shift);
		}
		else {
			switch(addr & 0xF003) {
				case 0x8000:
					prg[0] = data;
					break;
				case 0x9000:
					mirror = data;
					break;
				case 0xA000:
					prg[1] = data;
					break;
			}
		}
		sync();
	}
	log_printf("sl1632.c:  write:  $%04X = $%02X\n",addr,data);
}

static void reset(int hard)
{
	int i;

	mode = 0;
	if(hard) {
		for(i=0;i<8;i++) {
			prg[i & 1] = 0;
			chr[i] = 0;
		}
		mirror = 0;
	}
	mmc3_reset(C_MMC3B,sync,hard);
	write4 = mem_getwritefunc(4);
	for(i=4;i<16;i++) {
		if(i < 8)
			mem_setwritefunc(i,write_mode);
		else
			mem_setwritefunc(i,write);
	}
	sync();
}

static void ppucycle()
{
	mmc3_ppucycle();
}

static void state(int mode,u8 *data)
{
	mmc3_state(mode,data);
}

MAPPER(B_REXSOFT_SL1632,reset,ppucycle,0,state);
