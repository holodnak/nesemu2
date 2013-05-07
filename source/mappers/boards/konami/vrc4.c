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

static u8 vrc4a_map[] = {0,2,1,3,1};
static u8 vrc4b_map[] = {0,1,2,3,0};
static u8 vrc4c_map[] = {0,2,1,3,6};
static u8 vrc4d_map[] = {0,1,2,3,2};
static u8 vrc4e_map[] = {0,2,1,3,2};
static u8 *map;

static u8 prgflip;

static u8 prg[2],mirror;
static u8 irqcontrol,irqlatch,irqcounter;
static u16 chr[8];
static int irqprescaler;

static void sync()
{
	int i;

	mem_setsram8(0x6,0);
	if(prgflip) {
		mem_setprg8(0x8,0xFE);
		mem_setprg8(0xC,prg[0]);
	}
	else {
		mem_setprg8(0x8,prg[0]);
		mem_setprg8(0xC,0xFE);
	}
	mem_setprg8(0xA,prg[1]);
	mem_setprg8(0xE,0xFF);
	for(i=0;i<8;i++)
		mem_setchr1(i,chr[i]);
	switch(mirror & 3) {
		case 0: mem_setmirroring(MIRROR_V); break;
		case 1: mem_setmirroring(MIRROR_H); break;
		case 2: mem_setmirroring(MIRROR_1H); break;
		case 3: mem_setmirroring(MIRROR_1L); break;
	}
}

static void write(u32 addr,u8 data)
{
//	log_message("vrc4 write: $%04X = $%02X\n",addr,data);
	data &= 0x1F;
	switch(addr & 0xF000) {
		case 0x8000:
			prg[0] = data;
			break;
		case 0x9000:
			switch(map[(addr >> map[4]) & 3]) {
				case 0:
				case 1: mirror = data & 3; break;
				case 2:
				case 3: prgflip = data & 2; break;
			}
			break;
		case 0xA000:
			prg[1] = data;
			break;
		case 0xB000:
			switch(map[(addr >> map[4]) & 3]) {
				case 0: chr[0] = (chr[0] & 0x1F0) | ((data << 0) & 0x00F); break;
				case 1: chr[1] = (chr[1] & 0x1F0) | ((data << 0) & 0x00F); break;
				case 2: chr[0] = (chr[0] & 0x00F) | ((data << 4) & 0x1F0); break;
				case 3: chr[1] = (chr[1] & 0x00F) | ((data << 4) & 0x1F0); break;
			}
			break;
		case 0xC000:
			switch(map[(addr >> map[4]) & 3]) {
				case 0: chr[2] = (chr[2] & 0x1F0) | ((data << 0) & 0x00F); break;
				case 1: chr[3] = (chr[3] & 0x1F0) | ((data << 0) & 0x00F); break;
				case 2: chr[2] = (chr[2] & 0x00F) | ((data << 4) & 0x1F0); break;
				case 3: chr[3] = (chr[3] & 0x00F) | ((data << 4) & 0x1F0); break;
			}
			break;
		case 0xD000:
			switch(map[(addr >> map[4]) & 3]) {
				case 0: chr[4] = (chr[4] & 0x1F0) | ((data << 0) & 0x00F); break;
				case 1: chr[5] = (chr[5] & 0x1F0) | ((data << 0) & 0x00F); break;
				case 2: chr[4] = (chr[4] & 0x00F) | ((data << 4) & 0x1F0); break;
				case 3: chr[5] = (chr[5] & 0x00F) | ((data << 4) & 0x1F0); break;
			}
			break;
		case 0xE000:
			switch(map[(addr >> map[4]) & 3]) {
				case 0: chr[6] = (chr[6] & 0x1F0) | ((data << 0) & 0x00F); break;
				case 1: chr[7] = (chr[7] & 0x1F0) | ((data << 0) & 0x00F); break;
				case 2: chr[6] = (chr[6] & 0x00F) | ((data << 4) & 0x1F0); break;
				case 3: chr[7] = (chr[7] & 0x00F) | ((data << 4) & 0x1F0); break;
			}
			break;
		case 0xF000:
			switch(map[(addr >> map[4]) & 3]) {
				case 0:
					irqlatch = (irqlatch & 0xF0) | (data & 0xF);
					break;
				case 1:
					irqcontrol = data;
					if(data & 2) {
						irqcounter = irqlatch;
						irqprescaler = 341;
					}
					cpu_clear_irq(IRQ_MAPPER);
					break;
				case 2:
					irqlatch = (irqlatch & 0x0F) | ((data & 0xF) << 4);
					break;
				case 3:
					cpu_clear_irq(IRQ_MAPPER);
					irqcontrol |= (irqcontrol & 1) << 1;
					break;
			}
			break;
	}
	sync();
}

static void clockirq()
{
	if(irqcounter >= 0xFF) {
		irqcounter = irqlatch;
		cpu_set_irq(IRQ_MAPPER);
	}
	else
		irqcounter++;
}

static void ppucycle()
{
	if((irqcontrol & 2) == 0)
		return;

	if((irqcontrol & 4) == 0) {
		irqprescaler -= 2;
		if(irqprescaler <= 0) {
			irqprescaler += 341;
			clockirq();
		}
	}
}

static void cpucycle()
{
	if((irqcontrol & 2) == 0)
		return;

	if(irqcontrol & 4)
		clockirq();
}

static void reset(int revision)
{
	int i;

	switch(revision) {
		default:
		case B_KONAMI_VRC4A: map = vrc4a_map; break;
		case B_KONAMI_VRC4B: map = vrc4b_map; break;
		case B_KONAMI_VRC4C: map = vrc4c_map; break;
		case B_KONAMI_VRC4D: map = vrc4d_map; break;
		case B_KONAMI_VRC4E: map = vrc4e_map; break;
	}
	mem_setsramsize(2);
	for(i=8;i<0x10;i++)
		mem_setwritefunc(i,write);
	for(i=0;i<8;i++) {
		prg[i & 1] = 0;
		chr[i] = 0;
	}
	mirror = 0;
	irqcontrol = 0;
	irqlatch = 0;
	irqcounter = 0;
	irqprescaler = 0;
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_ARRAY_U8(prg,2);
	STATE_ARRAY_U16(chr,8);
	STATE_U8(mirror);
	STATE_U8(irqcontrol);
	STATE_U8(irqlatch);
	STATE_U8(irqcounter);
	STATE_INT(irqprescaler);
	sync();
}

static void reset_vrc4a(int hard)	{	reset(B_KONAMI_VRC4A);	}
static void reset_vrc4b(int hard)	{	reset(B_KONAMI_VRC4B);	}
static void reset_vrc4c(int hard)	{	reset(B_KONAMI_VRC4C);	}
static void reset_vrc4d(int hard)	{	reset(B_KONAMI_VRC4D);	}
static void reset_vrc4e(int hard)	{	reset(B_KONAMI_VRC4E);	}

MAPPER(B_KONAMI_VRC4A,reset_vrc4a,ppucycle,cpucycle,state);
MAPPER(B_KONAMI_VRC4B,reset_vrc4b,ppucycle,cpucycle,state);
MAPPER(B_KONAMI_VRC4C,reset_vrc4c,ppucycle,cpucycle,state);
MAPPER(B_KONAMI_VRC4D,reset_vrc4d,ppucycle,cpucycle,state);
MAPPER(B_KONAMI_VRC4E,reset_vrc4e,ppucycle,cpucycle,state);
