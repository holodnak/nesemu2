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
#include "mappers/sound/s_N106.h"

static apu_external_t sound = {
	N106sound_Load,
	N106sound_Unload,
	N106sound_Reset,
	N106sound_Get,
	0
};

static readfunc_t read4;
static writefunc_t write4;
static void (*sync)();
static u8 prg[4],chr[8],nt[4];
static u8 protect;
static u16 irqcounter;

void namcot163_sync()
{
	int i;

	mem_setwram8(6,0);
	for(i=0;i<4;i++) {
		mem_setprg8(8 + (i * 2),prg[i] & 0x3F);
	}
	for(i=0;i<4;i++) {
		if(chr[i] >= 0xE0 && (prg[1] & 0x40) == 0)
			mem_setvram1(i,chr[i] - 0xE0);
		else
			mem_setchr1(i,chr[i]);
		if(chr[i+4] >= 0xE0 && (prg[1] & 0x80) == 0)
			mem_setvram1(i+4,chr[i+4] - 0xE0);
		else
			mem_setchr1(i+4,chr[i+4]);
	}
	for(i=0;i<4;i++) {
		if(nt[i] >= 0xE0) {
			mem_setnt1(i + 0x8,nt[i] & 1);
			mem_setnt1(i + 0xC,nt[i] & 1);
		}
		else {
			mem_setchr1(i + 0x8,nt[i]);
			mem_setchr1(i + 0xC,nt[i]);
		}
	}
}

u8 namcot163_readsound(u32 addr)
{
	if(addr >= 0x4800)
		return(N106sound_Read(addr));
	return(read4(addr));
}

void namcot163_writesound(u32 addr,u8 data)
{
	if(addr >= 0x4800)
		N106sound_Write(addr,data);
	else
		write4(addr,data);
}

u8 namcot163_read(u32 addr)
{
	switch(addr & 0xF800) {
		case 0x5000:
			cpu_clear_irq(IRQ_MAPPER);
			return((u8)irqcounter);
		case 0x5800:
			cpu_clear_irq(IRQ_MAPPER);
			return((u8)(irqcounter >> 8));
	}
	return((u8)(addr >> 8));
}

void namcot163_write(u32 addr,u8 data)
{
	switch(addr & 0xF800) {
		case 0x5000:
			irqcounter = (irqcounter & 0xFF00) | data;
			cpu_clear_irq(IRQ_MAPPER);
			break;
		case 0x5800:
			irqcounter = (irqcounter & 0x00FF) | (data << 8);
			cpu_clear_irq(IRQ_MAPPER);
			break;
		case 0x8000:
		case 0x8800:
		case 0x9000:
		case 0x9800:
		case 0xA000:
		case 0xA800:
		case 0xB000:
		case 0xB800:
			chr[(addr >> 11) & 7] = data;
			sync();
			break;
		case 0xC000:
		case 0xC800:
		case 0xD000:
		case 0xD800:
			nt[(addr >> 11) & 3] = data;
			log_printf("namcot-163.c:  write:  nt %d = $%02X\n",(addr >> 11) & 3,data);
			sync();
			break;
		case 0xE000:
			prg[0] = data;
			sync();
			break;
		case 0xE800:
			prg[1] = data;
			sync();
			break;
		case 0xF000:
			prg[2] = data & 0x3F;
			sync();
			break;
		case 0xF800:
			protect = data;
			sync();
			break;
	}
}

void namcot163_reset(void (*syncfunc)(),int hard)
{
	int i;

	mem_setvramsize(8);
	mem_setwramsize(2);
	read4 = mem_getreadfunc(4);
	write4 = mem_getwritefunc(4);
	mem_setreadfunc(4,namcot163_readsound);
	mem_setwritefunc(4,namcot163_writesound);
	mem_setreadfunc(5,namcot163_read);
	mem_setwritefunc(5,namcot163_write);
	for(i=8;i<16;i++)
		mem_setwritefunc(i,namcot163_write);
	if(hard) {
		for(i=0;i<8;i++) {
			prg[i & 3] = 0;
			chr[i] = 0;
			nt[i & 3] = 0;
		}
		prg[3] = 0xFF;
		irqcounter = 0;
	}
	sync = syncfunc;
	apu_setexternal(&sound);
	sync();
}

void namcot163_cpucycle()
{
	if(irqcounter >= 0x8000) {
		irqcounter++;
		if(irqcounter == 0)
			cpu_set_irq(IRQ_MAPPER);
	}
}

void namcot163_state(int mode,u8 *data)
{
	STATE_ARRAY_U8(prg,4);
	STATE_ARRAY_U8(chr,8);
	STATE_ARRAY_U8(nt,4);
	STATE_U8(protect);
	STATE_U16(irqcounter);
	sync();
}
