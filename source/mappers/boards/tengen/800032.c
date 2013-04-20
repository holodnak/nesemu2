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
static u8 irqsource,irqlatch,irqreload,irqenabled,irqcounter,irqcpu;
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
	mem_setprg8(0xE,-1);
	for(i=0;i<8;i++)
		mem_setchr1(i ^ chrxor,chr[i]);
	if((control & 0x80) == 0) {
		mem_setchr2(0 ^ chrxor,chr[0] >> 1);
		mem_setchr2(2 ^ chrxor,chr[2] >> 1);
	}
	mem_setmirroring(mirror);
}

static void write_6000(u32 addr,u8 data)
{
}

static void write_upper(u32 addr,u8 data)
{
	switch(addr) {
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
			mirror = data & 1;
			break;
		case 0xA001:
			break;
		case 0xC000:
			irqlatch = data;
			break;
		case 0xC001:
			irqsource = data & 1;
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

	mem_setwritefunc(6,write_6000);
	for(i=8;i<16;i++)
		mem_setwritefunc(i,write_upper);
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

static void clock_irqcounter()
{
	if(irqreload) {
		irqcounter = irqlatch + 1;
		irqreload = 0;
	}
	else if(irqcounter == 0) {
		irqcounter = irqlatch;
	}
	else {
		irqcounter--;
		if(irqcounter == 0 && irqenabled) {
			cpu_set_irq(1);
//		log_printf("800032.c:  irq at frame %d, scanline %d, cycle %d\n",nes.ppu.frames,nes.ppu.scanline,nes.ppu.linecycles);
		}
	}
}

static void cycle()
{
	if(irqsource == 0) {
		if((nes.ppu.control1 & 0x18) && (nes.ppu.linecycles == 265))
			clock_irqcounter();
	}
	else {
//		log_printf("800032.c:  (cpu mode) cycle() called at frame %d, scanline %d, cycle %d\n",nes.ppu.frames,nes.ppu.scanline,nes.ppu.linecycles);
		irqcpu--;
		if(irqcpu == 0) {
			irqcpu = 3 * 4 / 2;
			clock_irqcounter();
		}
	}
}

static void state(int mode,u8 *data)
{
	STATE_U8(control);
	STATE_ARRAY_U8(prg,3);
	STATE_ARRAY_U8(chr,8);
	STATE_U8(mirror);
	STATE_U8(irqreload);
	STATE_U8(irqlatch);
	STATE_U8(irqsource);
	STATE_U8(irqenabled);
	STATE_U8(irqcounter);
	sync();
}

MAPPER(B_TENGEN_800032,reset,0,cycle,state);
