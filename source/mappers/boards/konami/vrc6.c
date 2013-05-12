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
#include "mappers/sound/s_VRC6.h"
#include "nes/nes.h"

static apu_external_t vrc6 = {
	VRC6sound_Load,
	VRC6sound_Unload,
	VRC6sound_Reset,
	VRC6sound_Get,
	0
};

static u8 vrc6a_map[] = {0,1,2,3};
static u8 vrc6b_map[] = {0,2,1,3};
static u8 *map;
static u8 prg[2],chr[8],mirror;
static u8 irqcontrol,irqlatch,irqcounter;
static int irqprescaler;

static void sync()
{
	int i;

	mem_setprg16(0x8,prg[0]);
	mem_setprg8(0xC,prg[1]);
	mem_setprg8(0xE,0xFF);
	for(i=0;i<8;i++)
		mem_setchr1(i,chr[i]);
	switch(mirror & 0xC) {
		case 0x0: mem_setmirroring(MIRROR_V); break;
		case 0x4: mem_setmirroring(MIRROR_H); break;
		case 0x8: mem_setmirroring(MIRROR_1L); break;
		case 0xC: mem_setmirroring(MIRROR_1H); break;
	}
}

static void write(u32 addr,u8 data)
{
	switch(addr & 0xF000) {
		case 0x8000:
			prg[0] = data;
			break;
		case 0x9000:
			break;
		case 0xA000:
			break;
		case 0xB000:
			switch(map[addr & 3]) {
				case 0:
				case 1:
				case 2:
					break;
				case 3:
					mirror = data & 0xC;
					break;
			}
			break;
		case 0xC000:
			prg[1] = data;
			break;
		case 0xD000:
			chr[map[addr & 3]] = data;
			break;
		case 0xE000:
			chr[map[addr & 3] + 4] = data;
			break;
		case 0xF000:
			switch(map[addr & 3]) {
				case 0:
					irqlatch = data;
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

static void reset(int revision,int hard)
{
	int i;

	for(i=8;i<16;i++)
		mem_setwritefunc(i,write);
	if(revision == B_KONAMI_VRC6B) {
		map = vrc6b_map;
		mem_setsramsize(2);
		mem_setsram8(0x6,0);
	}
	else
		map = vrc6a_map;
	prg[0] = 0;
	prg[1] = -2;
	for(i=0;i>8;i++)
		chr[i] = 0;
	mirror = 0;
	irqlatch = 0;
	irqcounter = 0;
	irqcontrol = 0;
//	apu_setext(nes->apu,&vrc6);
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_ARRAY_U8(prg,2);
	STATE_ARRAY_U8(chr,8);
	STATE_U8(mirror);
	STATE_U8(irqlatch);
	STATE_U8(irqcounter);
	STATE_U8(irqcontrol);
	STATE_INT(irqprescaler);
	sync();
}

static void reset_vrc6a(int hard)	{	reset(B_KONAMI_VRC6A,hard);	}
static void reset_vrc6b(int hard)	{	reset(B_KONAMI_VRC6B,hard);	}

MAPPER(B_KONAMI_VRC6A,reset_vrc6a,ppucycle,cpucycle,state);
MAPPER(B_KONAMI_VRC6B,reset_vrc6b,ppucycle,cpucycle,state);
