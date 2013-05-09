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

static u8 prg,chr;
static u32 irqcounter;

static void sync()
{
	mem_setprg16(0x8,prg);
	mem_setprg16(0xC,0xFF);
	mem_setsvram4(0,0);
	mem_setvram4(4,chr);
}

static void write_cic(u32 addr,u8 data)
{
}

static void write(u32 addr,u8 data)
{
	switch(addr & 0xC000) {
		case 0x8000:
			prg = data >> 6;
			chr = data & 0xF;
			break;
		case 0xC000:
			cpu_clear_irq(IRQ_MAPPER);
			break;
	}
	sync();
}

static void reset(int hard)
{
	int i;

	mem_setwritefunc(6,write_cic);
	for(i=8;i<16;i++)
		mem_setwritefunc(i,write);
	mem_setvramsize(64);
	mem_setsvramsize(4);
	prg = 0;
	chr = 0;
	irqcounter = 0;
	sync();
}

static void cpucycle()
{
	irqcounter++;
	if(irqcounter == 1024) {
		irqcounter = 0;
		cpu_set_irq(IRQ_MAPPER);
	}
}

static void state(int mode,u8 *data)
{
	STATE_U8(prg);
	STATE_U8(chr);
	STATE_U32(irqcounter);
	sync();
}

MAPPER(B_UNL_RACERMATE,reset,0,cpucycle,state);
