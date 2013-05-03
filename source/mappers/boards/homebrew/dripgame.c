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

static const u8 mirrormap[4] = {MIRROR_V,MIRROR_H,MIRROR_1L,MIRROR_1H};
static const u8 ntmap[4][4] = {
	{0,1,0,1},
	{0,0,1,1},
	{0,0,0,0},
	{1,1,1,1}
};
static const u8 attribs[4] = {0x00,0x55,0xAA,0xFF};
static u8 control,prg,chr[4];
static u8 irqenable,irqlatch;
static u16 irqcounter;
static readfunc_t read4,ppuread;
static u8 *exram[2];
static u32 prevbusaddr,busaddr;

//extended attributes
static u8 ppu_dripread(u32 addr)
{
	int nt = (addr >> 10) & 3;
	int offset = addr & 0x3FF;

	if(offset >= 0x3C0) {
		return(attribs[exram[ntmap[control & 3][(addr >> 10) & 3]][prevbusaddr & 0x3FF]]);
	}
	return(ppuread(addr));
}

static void sync()
{
	mem_setprg16(0x8,prg);
	mem_setprg16(0xC,0xFF);
	mem_setchr2(0,chr[0]);
	mem_setchr2(2,chr[1]);
	mem_setchr2(4,chr[2]);
	mem_setchr2(6,chr[3]);
	mem_setmirroring(mirrormap[control & 3]);
	if(control & 4) {
		ppu_setreadfunc(ppu_dripread);
	}
	else {
		ppu_setreadfunc(0);
	}
	if(control & 8)
		mem_setsram8(6,0);
	else
		mem_unsetcpu8(6);
}

static u8 read_4000(u32 addr)
{
	if(addr < 0x4800)
		return(read4(addr));
	return('d');
}

static u8 read_5000(u32 addr)
{
	static u8 hack = 0;

	hack += 3;
	if(hack < 20)
		return(0x40);
	if(hack > 200)
		return(0x80);
//	log_printf("reading drip sound:  $%04X\n",addr);
	return(0);
}

static void write_8000(u32 addr,u8 data)
{
//	if(addr >= 0x8010)
//		log_printf("dripgame.c:  write to $%04x = $%02X\n",addr,data);
	addr &= 0xF;
	switch(addr) {
		case 0x8:
			irqlatch = data;
			break;
		case 0x9:
			irqcounter = ((data & 0x7F) << 8) | irqlatch;
			irqenable = data & 0x80;
			cpu_set_irq(0);
			break;
		case 0xA:
			control = data & 0xF;
			break;
		case 0xB:
			prg = data & 0xF;
			break;
		case 0xC:
		case 0xD:
		case 0xE:
		case 0xF:
			chr[addr & 3] = data & 0xF;
			break;
	}
	sync();
}

static void write_C000(u32 addr,u8 data)
{
//	log_printf("dripgame.c:  write to $%04x = $%02X\n",addr,data);
	addr &= 0xFFF;
	if(addr < 0x400)
		exram[0][addr & 0x3FF] = data & 3;
	else if(addr < 0x800)
		exram[1][addr & 0x3FF] = data & 3;
}

static void reset(int hard)
{
	int i;

	mem_setsramsize(2);
	mem_setwramsize(1);
	mem_setwram8(8,0);
	exram[0] = mem_getreadptr(8);
	exram[1] = exram[0] + 0x400;
	mem_unsetcpu8(8);
	read4 = mem_getreadfunc(4);
	mem_setreadfunc(4,read_4000);
	mem_setreadfunc(5,read_5000);
	ppuread = ppu_getreadfunc();
	ppu_setreadfunc(ppu_dripread);
	for(i=0;i<4;i++) {
		mem_setwritefunc(0x8 + i,write_8000);
		mem_setwritefunc(0xC + i,write_C000);
	}
	control = 0;
	prg = 0;
	for(i=0;i<4;i++)
		chr[i] = i;
	sync();
}

static void cpucycle()
{
	if(irqenable) {
		irqcounter--;
		if(irqcounter == 0)
			cpu_set_irq(1);
	}
}

static void ppucycle()
{
	prevbusaddr = busaddr;
	busaddr = nes.ppu.busaddr;
}

static void state(int mode,u8 *data)
{
	STATE_U8(control);
	STATE_U8(prg);
	STATE_ARRAY_U8(chr,4);
	STATE_U8(irqenable);
	STATE_U8(irqlatch);
	STATE_U16(irqcounter);
}

MAPPER(B_DRIPGAME,reset,ppucycle,cpucycle,state);
