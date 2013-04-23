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

static int type;
static u8 cmd,prg,chr,mirror;
static readfunc_t read4;
static writefunc_t write4;

static void sync()
{
	mem_setprg16(0x8,(prg << 1) | 0);
	mem_setprg16(0xC,(prg << 1) | 1);
	mem_setchr8(0,chr);
//	log_printf("sync: prg = $%X (L:%X H:%X), chr = $%X, chr / 2 = $%X\n",prg,(prg<<1),(prg<<1)|1,chr,chr/2);
	switch(mirror) {
		case 0:
		case 1:
			mem_setmirroring(mirror & 1);
			break;
		case 2:
			mem_setmirroring2(0,1,1,1);
			break;
		case 3:
			mem_setmirroring(MIRROR_1H);
			break;
	}
}

static u8 read(u32 addr)
{
	if(addr < 0x4020)
		return(read4(addr));
	if(addr == 0x4100)
		return(~cmd & 0x3F);
	return(0);
}

static void write(u32 addr,u8 data)
{
	if(addr < 0x4020) {
		write4(addr,data);
		return;
	}
//	data &= 7;
	if((addr & 0x4101) == 0x4100) {
		cmd = data & 7;
	}
	else if((addr & 0x4101) == 0x4101) {
		log_printf("sachen-74ls374n:  write cmd %d = $%02X\n",cmd,data);
		switch(cmd) {
/*			case 0:
				prg = 0;
				chr = 3;
				break;
*/			case 2:
				chr &= ~8;
				chr |= (data & 1) << 3;
				break;
			case 4:
				chr &= ~4;
				chr |= (data & 1) << 2;
				break;
			case 5:
				prg = data & 7;
				break;
			case 6:
				chr &= ~3;
				chr |= (data & 3) << 0;
				break;
			case 7:
				mirror = (data & 6) >> 1;
				break;
			default:
				log_printf("sachen-74ls374n:  unknown write cmd %d (data = $%02X)\n",cmd,data);
				break;
		}
	}
	sync();
}

static void reset(int t,int hard)
{
	type = t;
	read4 = mem_getreadfunc(4);
	write4 = mem_getwritefunc(4);
	mem_setreadfunc(4,read);
	mem_setwritefunc(4,write);
	cmd = 0;
	prg = 0;
	chr = 3;
	mirror = 0;
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_U8(cmd);
	STATE_U8(prg);
	STATE_U8(chr);
	STATE_U8(mirror);
	sync();
}

static void reset_74ls374n(int hard)	{	reset(B_SACHEN_74LS374N,hard);	}
//static void reset_74ls374na(int hard)	{	reset(B_SACHEN_74LS374NA,hard);	}

MAPPER(B_SACHEN_74LS374N,reset_74ls374n,0,0,state);
//MAPPER(B_SACHEN_74LS374NA,reset_74ls374na,0,0,state);
