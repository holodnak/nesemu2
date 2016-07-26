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
#include "mappers/sound/s_VRC7.h"
#include "nes/nes.h"

static apu_external_t vrc7 = {
	VRC7sound_Load,
	VRC7sound_Unload,
	VRC7sound_Reset,
	VRC7sound_Get,
	0
};

static u8 prg[3],chr[8],mirror;
static u8 irqlatch,irqcontrol,irqcounter;
static int irqprescaler;

static void sync()
{
	int i;

	mem_setprg8(0x8,prg[0]);
	mem_setprg8(0xA,prg[1]);
	mem_setprg8(0xC,prg[2]);
	mem_setprg8(0xE,0xFF);
	if(nes->cart->chr.size) {
		for(i=0;i<8;i++)
			mem_setchr1(i,chr[i]);
	}
	else {
		for(i=0;i<8;i++)
			mem_setvram1(i,chr[i] & 7);
	}
	switch(mirror) {
		case 0: mem_setmirroring(MIRROR_V); break;
		case 1: mem_setmirroring(MIRROR_H); break;
		case 2: mem_setmirroring(MIRROR_1L); break;
		case 3: mem_setmirroring(MIRROR_1H); break;
	}
}

static void write_8000(u32 addr,u8 data)
{
	if(addr & 0x18)
		prg[1] = data;
	else
		prg[0] = data;
	sync();
}

static void write_9000(u32 addr,u8 data)
{
	if(addr & 0x18) {
		VRC7sound_Write(addr,data);
	}
	else {
		prg[2] = data;
		sync();
	}
}

static void write_A000(u32 addr,u8 data)
{
	if(addr & 0x18)
		chr[1] = data;
	else
		chr[0] = data;
	sync();
}

static void write_B000(u32 addr,u8 data)
{
	if(addr & 0x18)
		chr[3] = data;
	else
		chr[2] = data;
	sync();
}

static void write_C000(u32 addr,u8 data)
{
	if(addr & 0x18)
		chr[5] = data;
	else
		chr[4] = data;
	sync();
}

static void write_D000(u32 addr,u8 data)
{
	if(addr & 0x18)
		chr[7] = data;
	else
		chr[6] = data;
	sync();
}

static void write_E000(u32 addr,u8 data)
{
	if(addr & 0x18)
		irqlatch = data;
	else
		mirror = data & 3;
}

static void write_F000(u32 addr,u8 data)
{
	if(addr & 0x18) {
		irqcontrol &= ~2;
		irqcontrol |= (irqcontrol & 1) << 1;
		cpu_clear_irq(IRQ_MAPPER);
	}
	else {
		irqcontrol = data & 7;
		if(data & 2) {
			irqcounter = irqlatch;
			irqprescaler = 341;
		}
	}
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

	mem_setwritefunc(0x8,write_8000);
	mem_setwritefunc(0x9,write_9000);
	mem_setwritefunc(0xA,write_A000);
	mem_setwritefunc(0xB,write_B000);
	mem_setwritefunc(0xC,write_C000);
	mem_setwritefunc(0xD,write_D000);
	mem_setwritefunc(0xE,write_E000);
	mem_setwritefunc(0xF,write_F000);
	if(revision == B_KONAMI_VRC7A) {		//lagrange point
		mem_setvramsize(8);
		mem_setvram8(0,0);
	}
	mem_setwramsize(8);
	mem_setwram8(6,0);
	prg[0] = 0;
	prg[1] = (u8)-2;
	prg[2] = (u8)-2;
	for(i=0;i<8;i++)
		chr[i] = 0;
	mirror = 0;
	irqlatch = 0;
	irqcontrol = 0;
	irqcounter = 0;
	irqprescaler = 341;
	apu_setexternal(&vrc7);
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_ARRAY_U8(prg,2);
	STATE_ARRAY_U8(chr,8);
	STATE_U8(mirror);
	STATE_U8(irqcontrol);
	STATE_U8(irqlatch);
	STATE_U8(irqcounter);
	STATE_INT(irqprescaler);
	sync();
}

static void reset_vrc7a(int hard)	{	reset(B_KONAMI_VRC7A,hard);	}
static void reset_vrc7b(int hard)	{	reset(B_KONAMI_VRC7B,hard);	}

MAPPER(B_KONAMI_VRC7A,reset_vrc7a,ppucycle,cpucycle,state);
MAPPER(B_KONAMI_VRC7B,reset_vrc7b,ppucycle,cpucycle,state);
