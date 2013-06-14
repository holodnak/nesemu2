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

static u8 prg[3],chr[8],mirror;
static u8 irqenabled;
static u16 irqcounter,irqlatch;

static void sync()
{
	int i;

	mem_setwram8(6,0);
	mem_setprg8(0x8,prg[0]);
	mem_setprg8(0xA,prg[1]);
	mem_setprg8(0xC,prg[2]);
	mem_setprg8(0xE,0xFF);
	for(i=0;i<8;i++)
		mem_setchr1(i,chr[i]);
	mem_setmirroring(mirror);
}

static void write_reg(u32 addr,u8 data)
{
	switch(addr & 0xF007) {
		case 0x8000:
			prg[0] = data;
			break;
		case 0x9001:
			mirror = (data >> 7) ^ 1;
			break;
		case 0x9003:
			irqenabled = data >> 7;
			cpu_clear_irq(IRQ_MAPPER);
			break;
		case 0x9004:
			irqcounter = irqlatch;
			cpu_clear_irq(IRQ_MAPPER);
			break;
		case 0x9005:
			irqlatch = (data << 8) | (irqlatch & 0xFF);
			break;
		case 0x9006:
			irqlatch = data | (irqlatch & 0xFF00);
			break;
		case 0xA000:
			prg[1] = data;
			break;
		case 0xB000:
		case 0xB001:
		case 0xB002:
		case 0xB003:
		case 0xB004:
		case 0xB005:
		case 0xB006:
		case 0xB007:
			chr[addr & 7] = data;
			break;
		case 0xC000:
			prg[2] = data;
			break;
	}
	sync();
}

static void cpucycle()
{
	if(irqenabled == 0)
	   return;
	irqcounter--;
	if(irqcounter == 0) {
		irqenabled = 0;
		cpu_set_irq(IRQ_MAPPER);
	}
}

static void reset(int hard)
{
	int i;

	mem_setwramsize(8);
	for(i=8;i<0x10;i++)
		mem_setwritefunc(i,write_reg);
	prg[0] = 0;
	prg[1] = 1;
	prg[2] = 0xFE;
	for(i=0;i<8;i++)
		chr[i] = i;
	irqcounter = 0;
	irqenabled = 0;
	irqlatch = 0;
	mirror = 0;
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_ARRAY_U8(prg,3);
	STATE_ARRAY_U8(chr,8);
	STATE_U8(mirror);
	STATE_U8(irqenabled);
	STATE_U16(irqcounter);
	STATE_U16(irqlatch);
	sync();
}

MAPPER(B_IREM_H3001,reset,0,cpucycle,state);
