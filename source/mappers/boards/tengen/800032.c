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

static u8 control,mirror;
static u8 irqsource,irqlatch,irqreload,irqenabled,irqcounter,irqcpu,irqwait,needirq;
static u8 prg[3],chr[8];

static void sync()
{
	int i;
	u8 chrxor = (control & 0x80) >> 5;

	if((control & 0x40) == 0) {
		mem_setprg8(0x8,prg[0]);
		mem_setprg8(0xA,prg[1]);
		mem_setprg8(0xC,prg[2]);
	}
	else {
		mem_setprg8(0xA,prg[0]);
		mem_setprg8(0xC,prg[1]);
		mem_setprg8(0x8,prg[2]);
	}
	mem_setprg8(0xE,0xFF);
	for(i=0;i<8;i++)
		mem_setchr1(i ^ chrxor,chr[i]);
	if((control & 0x20) == 0) {
		mem_setchr2(0 ^ chrxor,chr[0] >> 1);
		mem_setchr2(2 ^ chrxor,chr[2] >> 1);
	}
	mem_setmirroring(mirror);
}

static void write(u32 addr,u8 data)
{
	switch(addr & 0xE001) {
		//control register
		case 0x8000:
			control = data;
			break;
		case 0x8001:
			switch(control & 0xF) {
				case 0x0: chr[0] = data; break;
				case 0x1: chr[2] = data; break;
				case 0x2: chr[4] = data; break;
				case 0x3: chr[5] = data; break;
				case 0x4: chr[6] = data; break;
				case 0x5: chr[7] = data; break;
				case 0x6: prg[0] = data; break;
				case 0x7: prg[1] = data; break;
				case 0x8: chr[1] = data; break;
				case 0x9: chr[3] = data; break;
				case 0xF: prg[2] = data; break;
			}
			break;
		case 0xA000:
			mirror = (data & 1) ^ 1;
			break;
		case 0xA001:
			break;
		case 0xC000:
			irqlatch = data;
			break;
		case 0xC001:
			irqsource = data & 1;
			irqcounter = 0;
			irqreload = 1;
			break;
		case 0xE000:
			irqenabled = 0;
			cpu_set_irq(0);
			break;
		case 0xE001:
			irqenabled = 1;
			break;
	}
	sync();
}

static void reset(int hard)
{
	int i;

	for(i=8;i<16;i++)
		mem_setwritefunc(i,write);
	control = 0;
	mirror = 0;
	prg[0] = prg[1] = prg[2] = 0xFF;
	for(i=0;i<8;i++)
		chr[i] = i;
	irqsource = 0;
	irqlatch = 0;
	irqreload = 0;
	irqenabled = 0;
	irqcounter = 0;
	irqcpu = 0;
	sync();
}

static void clockirq()
{
	u8 tmp = irqcounter;
	if((irqcounter == 0) || irqreload) {
		irqcounter = irqlatch;
	}
	else {
		irqcounter--;
	}
	if((tmp || irqreload) && (irqcounter == 0) && irqenabled) {
		needirq = 3;
	}
	irqreload = 0;
}

static void ppucycle()
{
	if(irqsource == 0) {
		if(needirq && (--needirq) == 0) {
			cpu_set_irq(1);
		}
		if(irqwait) {
			irqwait--;
		}
		if((irqwait == 0) && (nes.ppu.busaddr & 0x1000)) {
			clockirq();
		}
		if(nes.ppu.busaddr & 0x1000) {
			irqwait = 8;
		}
	}
}

static void cpucycle()
{
	if(irqsource) {
		irqcpu--;
		if(irqcpu == 0) {
			irqcpu = 4;
			clockirq();
			if(needirq) {
				needirq = 0;
				cpu_set_irq(1);
			}
		}
	}
}

static void state(int mode,u8 *data)
{
	STATE_U8(control);
	STATE_ARRAY_U8(prg,3);
	STATE_ARRAY_U8(chr,8);
	STATE_U8(mirror);
	STATE_U8(irqsource);
	STATE_U8(irqlatch);
	STATE_U8(irqreload);
	STATE_U8(irqenabled);
	STATE_U8(irqcounter);
	STATE_U8(irqcpu);
	STATE_U8(irqwait);
	STATE_U8(needirq);
	sync();
}

MAPPER(B_TENGEN_800032,reset,0,ppucycle,cpucycle,state);
