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

static u8 prg[3],chr[8],mirror,irqcontrol;
static u16 irqlatch,irqcounter,irqmask;

static void sync()
{
	int i;

	mem_setsram8(6,0);
	mem_setprg8(0x8,prg[0]);
	mem_setprg8(0xA,prg[1]);
	mem_setprg8(0xC,prg[2]);
	mem_setprg8(0xE,0xFF);
	for(i=0;i<8;i++)
		mem_setchr1(i,chr[i]);
	switch(mirror) {
		case 0: mem_setmirroring(MIRROR_H); break;
		case 1: mem_setmirroring(MIRROR_V); break;
		case 2: mem_setmirroring(MIRROR_1L); break;
		case 3: mem_setmirroring(MIRROR_1H); break;
	}
	if(irqcontrol & 8)		irqmask = 0xF;
	else if(irqcontrol & 4)	irqmask = 0xFF;
	else if(irqcontrol & 2)	irqmask = 0xFFF;
	else							irqmask = 0xFFFF;
}

static void write(u32 addr,u8 data)
{
	switch(addr & 0xF003) {
		case 0x8000: prg[0] = (prg[0] & 0xF0) | ((data << 0) & 0x0F); break;
		case 0x8001: prg[0] = (prg[0] & 0x0F) | ((data << 4) & 0xF0); break;
		case 0x8002: prg[1] = (prg[1] & 0xF0) | ((data << 0) & 0x0F); break;
		case 0x8003: prg[1] = (prg[1] & 0x0F) | ((data << 4) & 0xF0); break;
		case 0x9000: prg[2] = (prg[2] & 0xF0) | ((data << 0) & 0x0F); break;
		case 0x9001: prg[2] = (prg[2] & 0x0F) | ((data << 4) & 0xF0); break;
		case 0x9002: break;
		case 0x9003: break;
		case 0xA000: chr[0] = (chr[0] & 0xF0) | ((data << 0) & 0x0F); break;
		case 0xA001: chr[0] = (chr[0] & 0x0F) | ((data << 4) & 0xF0); break;
		case 0xA002: chr[1] = (chr[1] & 0xF0) | ((data << 0) & 0x0F); break;
		case 0xA003: chr[1] = (chr[1] & 0x0F) | ((data << 4) & 0xF0); break;
		case 0xB000: chr[2] = (chr[2] & 0xF0) | ((data << 0) & 0x0F); break;
		case 0xB001: chr[2] = (chr[2] & 0x0F) | ((data << 4) & 0xF0); break;
		case 0xB002: chr[3] = (chr[3] & 0xF0) | ((data << 0) & 0x0F); break;
		case 0xB003: chr[3] = (chr[3] & 0x0F) | ((data << 4) & 0xF0); break;
		case 0xC000: chr[4] = (chr[4] & 0xF0) | ((data << 0) & 0x0F); break;
		case 0xC001: chr[4] = (chr[4] & 0x0F) | ((data << 4) & 0xF0); break;
		case 0xC002: chr[5] = (chr[5] & 0xF0) | ((data << 0) & 0x0F); break;
		case 0xC003: chr[5] = (chr[5] & 0x0F) | ((data << 4) & 0xF0); break;
		case 0xD000: chr[6] = (chr[6] & 0xF0) | ((data << 0) & 0x0F); break;
		case 0xD001: chr[6] = (chr[6] & 0x0F) | ((data << 4) & 0xF0); break;
		case 0xD002: chr[7] = (chr[7] & 0xF0) | ((data << 0) & 0x0F); break;
		case 0xD003: chr[7] = (chr[7] & 0x0F) | ((data << 4) & 0xF0); break;
		case 0xE000: irqlatch = (irqlatch & 0xFFF0) | ((data & 0xF) << 0); break;
		case 0xE001: irqlatch = (irqlatch & 0xFF0F) | ((data & 0xF) << 4); break;
		case 0xE002: irqlatch = (irqlatch & 0xF0FF) | ((data & 0xF) << 8); break;
		case 0xE003: irqlatch = (irqlatch & 0x0FFF) | ((data & 0xF) << 12); break;
		case 0xF000: irqcounter = irqlatch; cpu_set_irq(0); break;
		case 0xF001: irqcontrol = data & 0xF; cpu_set_irq(0); break;
		case 0xF002: mirror = data & 3; break;
		case 0xF003: break;
	}
	sync();
}

static void reset(int hard)
{
	int i;

	mem_setsramsize(2);
	for(i=8;i<16;i++)
		mem_setwritefunc(i,write);
	prg[0] = 0;
	prg[1] = 1;
	prg[2] = 0xFE;
	for(i=0;i<8;i++)
		chr[i] = i;
	mirror = irqcontrol = 0;
	irqmask = 0;
	irqlatch = irqcounter = 0;
	sync();
}

static void cpucycle()
{
	if(irqcontrol & 1) {
		if((irqcounter & irqmask) == 0) {
			cpu_set_irq(1);
			irqcounter = 0;
			irqcounter += irqmask + 1;
		}
		irqcounter--;
	}
}

static void state(int mode,u8 *data)
{
	STATE_ARRAY_U8(prg,3);
	STATE_ARRAY_U8(chr,8);
	STATE_U8(mirror);
	STATE_U8(irqcontrol);
	STATE_U16(irqcounter);
	STATE_U16(irqlatch);
	STATE_U16(irqmask);
	sync();
}

MAPPER(B_JALECO_SS88006,reset,0,0,cpucycle,state);
