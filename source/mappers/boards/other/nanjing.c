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

static u8 reg[2],prg,security,trigger,strobe,vram[2];

static void sync()
{
	mem_setprg32(8,prg);
	mem_setvram4(0, vram[0]);
	mem_setvram4(4, vram[1]);
}

static u8 read(u32 addr)
{
	log_printf("nanjing read: $%04X (PC = %04X)\n", addr, nes->cpu.pc);
	switch(addr & 0x7700) {
		case 0x5000:
			return(4);
		case 0x5100:
			return(security);
		case 0x5500:
			return(security & trigger);
	}
	return((u8)(addr >> 8));
}

static void write(u32 addr, u8 data)
{
	log_printf("nanjing write: $%04X = $%02X (PC = %04X)\n", addr, data, nes->cpu.pc);
	if(addr == 0x5101) {
		u8 tmp = strobe;

		strobe = data;
		if(tmp && strobe == 0)
			trigger ^= 0xFF;
		return;
	}
	switch(addr & 0x7300) {
		case 0x5000:
			reg[0] = data;
			prg = (reg[0] & 0xF) | ((reg[1] & 0xF) << 4);
			if((reg[0] & 0x80) == 0) {
				vram[0] = 0;
				vram[1] = 1;
			}
			sync();
			break;
		case 0x5100:
			if(data == 6)
				prg = 3;
			sync();
			break;
		case 0x5200:
			reg[1] = data;
			prg = (reg[0] & 0xF) | ((reg[1] & 0xF) << 4);
			sync();
			break;
		case 0x5300:
			security = data;
			break;
	}
}

static u8 sram[0x2000];

static u8 read_sram(u32 addr)
{
	u8 ret = addr >> 8;

//	log_printf("sram read: $%04X (pc = %04X)\n", addr, nes->cpu.pc);
	if (addr >= 0x6000 && addr < 0x8000)
		ret = sram[addr & 0x1FFF];

	return(ret);
}

static void write_sram(u32 addr, u8 data)
{
//	log_printf("sram write: $%04X = $%02X (pc = %04X)\n", addr, data, nes->cpu.pc);
	if (addr >= 0x6000 && addr < 0x8000)
		sram[addr & 0x1FFF] = data;
}

static void reset(int hard)
{
	mem_setreadfunc(5, read);
	mem_setwritefunc(5, write);
	mem_setreadfunc(6, read_sram);
	mem_setwritefunc(6, write_sram);
	mem_setreadfunc(7, read_sram);
	mem_setwritefunc(7, write_sram);
	mem_setvramsize(8);
	mem_setwramsize(8);
	reg[0] = 0xFF;
	reg[1] = 0x00;
	strobe = 0xFF;
	trigger = 0;
	security = 0;
	vram[0] = 0;
	vram[1] = 1;
	sync();
}

static void ppucycle()
{
	if((reg[0] & 0x80) == 0)
		return;
	if(SCANLINE == 128) {
		vram[0] = vram[1] = 1;
		sync();
	}
	else if(SCANLINE == 240) {
		vram[0] = vram[1] = 0;
		sync();
	}
}

static void state(int mode,u8 *data)
{
	STATE_ARRAY_U8(reg,2);
	STATE_U8(security);
	STATE_U8(trigger);
	STATE_U8(strobe);
	STATE_ARRAY_U8(vram,2);
	sync();
}

MAPPER(B_NANJING,reset,ppucycle,0,state);
