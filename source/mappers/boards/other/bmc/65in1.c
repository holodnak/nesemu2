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
#include "mappers/chips/latch.h"

static readfunc_t oldread;
static u8 bankmode,dip;

static void sync()
{
	u8 prglo,prghi,chr;

	bankmode = (latch_addr >> 8) & 1;
	prglo = ((latch_addr >> 4) & 0xE) & ~(~latch_addr >> 7 & 1);
	prghi = ((latch_addr >> 4) & 0xE) | (~latch_addr >> 7 & 1);
	chr = latch_addr & 0xF;
	mem_setprg16(0x8,prglo);
	mem_setprg16(0xC,prghi);
	mem_setchr8(0,chr);
	mem_setmirroring(((latch_addr >> 3) & 1) ^ 1);
}

static u8 read(u32 addr)
{
	if(bankmode && addr >= 0x8000) {
		return(dip);
	}
	return(oldread(addr));
}

static void write(u32 addr,u8 data)
{
	log_printf("bmc/65in1.c:  write $%04X = $%02X\n",addr,data);
	latch_write(addr,data);
}

static void reset(int hard)
{
	int i;

	dip = 0;
	latch_reset(sync,hard);
	oldread = cpu_getreadfunc();
	cpu_setreadfunc(read);
	for(i=8;i<16;i++)
		mem_setwritefunc(i,write);
}

static void state(int mode,u8 *data)
{
	STATE_U8(bankmode);
	latch_state(mode,data);
}

MAPPER(B_BMC_65IN1,reset,0,0,state);
