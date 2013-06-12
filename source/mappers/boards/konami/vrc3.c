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

static u8 prg;
static u8 irqcontrol;
static u32 irqreload,irqcounter;

static void sync()
{
	mem_setprg16(0x8,prg);
	mem_setprg16(0xC,0xFF);
	mem_setvram8(0,0);
	mem_setwram8(6,0);
}

static void write(u32 addr,u8 data)
{
	data &= 0xF;
	switch(addr & 0xF000) {
		case 0x8000:
			irqreload = (irqreload & 0xFFF0) | (data << 0);
			break;
		case 0x9000:
			irqreload = (irqreload & 0xFF0F) | (data << 4);
			break;
		case 0xA000:
			irqreload = (irqreload & 0xF0FF) | (data << 8);
			break;
		case 0xB000:
			irqreload = (irqreload & 0x0FFF) | (data << 12);
			break;
		case 0xC000:
			irqcontrol = data & 7;
			if(data & 2)
				irqcounter = irqreload;
		case 0xD000:
			cpu_clear_irq(IRQ_MAPPER);
			irqcontrol |= (irqcontrol & 1) << 1;
			break;
		case 0xF000:
			prg = data;
			sync();
			break;
	}
}

static void reset(int hard)
{
	int i;

	mem_setvramsize(8);
	mem_setwramsize(8);
	for(i=8;i<16;i++)
		mem_setwritefunc(i,write);
	prg = 0;
	irqcontrol = 0;
	irqreload = 0;
	irqcounter = 0;
	sync();
}

static void cpucycle()
{
	//see if it is enabled
	if((irqcontrol & 2) == 0)
		return;

	//16bit mode
	if((irqcontrol & 4) == 0) {
		if(irqcounter == 0xFFFF) {
			irqcounter = irqreload;
			cpu_set_irq(IRQ_MAPPER);
		}
		else
			irqcounter++;
	}

	//8bit mode
	else {
		if((irqcounter & 0xFF) == 0xFF) {
			irqcounter = (irqcounter & 0xFF00) | (irqreload & 0xFF);
			cpu_set_irq(IRQ_MAPPER);
		}
		else
			irqcounter++;
	}
}

static void state(int mode,u8 *data)
{
	STATE_U8(prg);
	STATE_U8(irqcontrol);
	STATE_U32(irqreload);
	STATE_U32(irqcounter);
	sync();
}

MAPPER(B_KONAMI_VRC3,reset,0,cpucycle,state);
