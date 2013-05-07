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

static u8 prg[2],chr[8],mirror;
static u8 irqenable,irqlatch,irqcounter;

static void sync()
{
	int i;

	mem_setsram8(6,0);
	mem_setprg8(0x8,prg[0]);
	mem_setprg8(0xA,prg[1]);
	mem_setprg8(0xC,0xFE);
	mem_setprg8(0xE,0xFF);
	for(i=0;i<8;i++) {
		if(chr[i] == 6 || chr[i] == 7)
			mem_setvram1(i,chr[i] & 1);
		else
			mem_setchr1(i,chr[i]);
	}
}

static void write(u32 addr,u8 data)
{
	int index;

	log_printf("waixing:  sgz.c:  write $%04X = $%02X\n",addr,data);
	switch(addr & 0xF000) {
		case 0x8000:
			prg[0] = data;
			break;
		case 0xA000:
			prg[1] = data;
			break;
		case 0xB000:
		case 0xC000:
		case 0xD000:
		case 0xE000:
			index = (addr & 8) | (addr >> 8);
			index >>= 3;
			index += 2;
			index &= 7;
			if(addr & 4) {
				chr[index] &= 0x0F;
				chr[index] |= (data & 0x0F) << 4;
			}
			else {
				chr[index] &= 0xF0;
				chr[index] |= data & 0x0F;
			}
			break;
		case 0xF000:
			switch(addr & 0xC) {
				case 0x0:
					cpu_clear_irq(IRQ_MAPPER);
					irqlatch &= 0xF0;
					irqlatch |= data & 0xF;
					break;
				case 0x4:
					cpu_clear_irq(IRQ_MAPPER);
					irqlatch &= 0x0F;
					irqlatch |= (data & 0xF) << 4;
					break;
				case 0x8:
					cpu_clear_irq(IRQ_MAPPER);
					irqcounter = irqlatch;
					irqenable = data & 2;
					break;
				case 0xC:
					break;
			}
			break;
	}
	sync();
}

static void reset(int hard)
{
	int i;

	mem_setsramsize(2);
	mem_setvramsize(8);
	for(i=8;i<16;i++)
		mem_setwritefunc(i,write);
	if(hard) {
		prg[0] = prg[1] = 0;
		for(i=0;i<8;i++)
			chr[i] = i;
		mirror = 0;
		irqlatch = irqcounter = 0;
		irqenable = 0;
	}
	sync();
}

static void ppucycle()
{
	if(irqenable == 0)
		return;
	if(LINECYCLES == 257) {
		if(irqcounter == 0xFF) {
			irqcounter = irqlatch;
			cpu_set_irq(IRQ_MAPPER);
		}
		irqcounter++;
	}
}

static void state(int mode,u8 *data)
{
	STATE_ARRAY_U8(prg,2);
	STATE_ARRAY_U8(chr,8);
	STATE_U8(mirror);
	STATE_U8(irqenable);
	STATE_U8(irqlatch);
	STATE_U8(irqcounter);
	sync();
}

MAPPER(B_WAIXING_SGZ,reset,ppucycle,0,state);
