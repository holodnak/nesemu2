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

static u8 prg,irqenable;
static u16 irqcounter;
static writefunc_t write4;

static void sync()
{
	mem_setprg8(0x6,0xF);
	mem_setprg8(0x8,0x8);
	mem_setprg8(0xA,0x9);
	mem_setprg8(0xC,prg);
	mem_setprg8(0xE,0xB);
}

static void write(u32 addr,u8 data)
{
	if(addr < 0x4020) {
		write4(addr,data);
		return;
	}
	switch(addr & 0x4120) {
		case 0x4020:
			prg = (data >> 1) & 3;	//low
			prg |= (data & 1) << 2;	//middle
			prg |= data & 8;			//high
			sync();
			break;
		case 0x4120:
			irqenable = data;
			if((data & 1) == 0) {
				irqcounter = 0;
				cpu_clear_irq(IRQ_MAPPER);
			}
			break;
	}
}

static void reset(int hard)
{
	write4 = mem_getwritefunc(4);
	mem_setwritefunc(4,write);
	mem_setwritefunc(5,write);
	mem_setvramsize(8);
	mem_setvram8(0,0);
	if(hard) {
		prg = 0;
		irqenable = 0;
		irqcounter = 0;
	}
	sync();
}

static void cpucycle()
{
	if(irqenable == 0)
		return;
	irqcounter++;
	if(irqcounter == 0x1000)
		cpu_set_irq(IRQ_MAPPER);
}

static void state(int mode,u8 *data)
{
	STATE_U8(prg);
	STATE_U8(irqenable);
	STATE_U16(irqcounter);
	sync();
}

MAPPER(B_BTL_SMB2B,reset,0,cpucycle,state);
