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

/*
bmc/65in1.c:  write $8100 = $A2
bmc/65in1.c:  write $8001 = $00

bmc/65in1.c:  write $F044 = $00

bmc/65in1.c:  write $F06E = $00

bmc/65in1.c:  write $F282 = $00

bmc/65in1.c:  write $F2AB = $00
*/
static void sync()
{
	u8 prg = 0;
	u8 chr = 0;

	chr = latch_addr & 7;
	mem_setprg32(8,prg);
	mem_setchr8(0,chr);
}

static void write(u32 addr,u8 data)
{
	log_printf("bmc/65in1.c:  write $%04X = $%02X\n",addr,data);
	latch_write(addr,data);
}

static void reset(int hard)
{
	int i;

	latch_reset(sync,hard);
	for(i=8;i<16;i++)
		mem_setwritefunc(i,write);
}

MAPPER(B_BMC_65IN1,reset,0,0,latch_state);
