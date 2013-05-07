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
#include "mappers/chips/taito-tc0190fmc.h"

static u8 irqlatch,irqcounter;
static u8 irqreload,irqenabled;
static u8 irqwait,needirq;

static void write(u32 addr,u8 data)
{
	switch(addr & 0xE003) {
		case 0xC000:
			irqlatch = data ^ 0xFF;
			break;
		case 0xC001:
			irqcounter = 0;
			irqreload = 1;
			break;
		case 0xC002:
			irqenabled = 0;
			cpu_clear_irq(IRQ_MAPPER);
			break;
		case 0xC003:
			irqenabled = 1;
			break;
	}
}

static void reset(int hard)
{
	tc0190fmc_reset(hard);
	mem_setwritefunc(0xC,write);
	mem_setwritefunc(0xD,write);
	irqcounter = irqlatch = 0;
	irqenabled = irqreload = 0;
	irqwait = needirq = 0;
}

static void ppucycle()
{
	u8 tmp;

	if(needirq && (--needirq) == 0)
		cpu_set_irq(IRQ_MAPPER);
	if(irqwait)
		irqwait--;
	if((irqwait == 0) && (nes.ppu.busaddr & 0x1000)) {
		tmp = irqcounter;
		if((irqcounter == 0) || irqreload)
			irqcounter = irqlatch;
		else
			irqcounter--;
		if((tmp || irqreload) && (irqcounter == 0) && irqenabled)
			needirq = 12;
		irqreload = 0;
	}
	if(nes.ppu.busaddr & 0x1000) {
		irqwait = 8;
	}
}

static void state(int mode,u8 *data)
{
	tc0190fmc_state(mode,data);
	STATE_U8(irqlatch);
	STATE_U8(irqcounter);
	STATE_U8(irqreload);
	STATE_U8(irqenabled);
	STATE_U8(irqwait);
	STATE_U8(needirq);
}

MAPPER(B_TAITO_TC0190FMC_PAL16R4,reset,ppucycle,0,state);
