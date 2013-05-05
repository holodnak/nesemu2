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

static u8 prg,chr[4],mirror;
static u16 irqcounter;
static u8 irqenable,irqtoggle;

static void sync()
{
	mem_setprg16(0x8,prg);
	mem_setprg16(0xC,0xFF);
	mem_setchr2(0,chr[0]);
	mem_setchr2(2,chr[1]);
	mem_setchr2(4,chr[2]);
	mem_setchr2(6,chr[3]);
	switch(mirror) {
		case 0:	mem_setmirroring(MIRROR_V);	break;
		case 1:	mem_setmirroring(MIRROR_H);	break;
		case 2:	mem_setmirroring(MIRROR_1L);	break;
		case 3:	mem_setmirroring(MIRROR_1H);	break;
	}
}

static void write(u32 addr,u8 data)
{
	switch(addr & 0xF800) {
		case 0x8800:
		case 0x9800:
		case 0xA800:
		case 0xB800:
			chr[(addr >> 12) & 3] = data;
			break;
		case 0xC800:
			if(irqtoggle) {
				irqcounter = (irqcounter & 0xFF00) | data;
			}
			else {
				irqcounter = (irqcounter & 0x00FF) | (data << 8);
			}
			irqtoggle ^= 1;
			break;
		case 0xD800:
			irqenable = data & 0x10;
			irqtoggle = 0;
			cpu_set_irq(0);
			break;
		case 0xE800:
			mirror = data & 3;
			break;
		case 0xF800:
			prg = data;
			break;
	}
	sync();
}

static void reset(int hard)
{
	int i;

	for(i=8;i<16;i++)
		mem_setwritefunc(i,write);
	if(hard) {
		prg = 0;
		chr[0] = chr[1] = chr[2] = chr[3] = 0;
		mirror = 0;
		irqcounter = 0xFFFF;
		irqenable = irqtoggle = 0;
	}
	sync();
}

static void cpucycle()
{
	if(irqenable == 0)
		return;
	if(irqcounter == 0) {
		irqenable = 0;
		cpu_set_irq(1);
	}
	irqcounter--;
}

static void state(int mode,u8 *data)
{
	STATE_U8(prg);
	STATE_ARRAY_U8(chr,4);
	STATE_U8(mirror);
	STATE_U16(irqcounter);
	STATE_U8(irqenable);
	STATE_U8(irqtoggle);
	sync();
}

MAPPER(B_SUNSOFT_3,reset,0,cpucycle,state);
