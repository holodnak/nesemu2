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

static u8 control,prg[4],chr[8],mirror;
static u8 irqenable;
static u16 irqlatch,irqcounter;

static void sync()
{
	int i;

	mem_setwram8(6,0);
	mem_setprg8(0x8,prg[0]);
	mem_setprg8(0xA,prg[1]);
	mem_setprg8(0xC,prg[2]);
	mem_setprg8(0xE,0xFF);
	for(i=0;i<8;i++)
		mem_setchr1(i,chr[i]);
}

static void write(u32 addr,u8 data)
{
	switch(addr & 0xF000) {
		case 0x8000:	irqlatch = (irqlatch & 0xFFF0) | ((data & 0xF) << 0);		break;
		case 0x9000:	irqlatch = (irqlatch & 0xFF0F) | ((data & 0xF) << 4);		break;
		case 0xA000:	irqlatch = (irqlatch & 0xF0FF) | ((data & 0xF) << 8);		break;
		case 0xB000:	irqlatch = (irqlatch & 0x0FFF) | ((data & 0xF) << 12);	break;
		case 0xC000:
			irqenable = data & 7;
			if(irqenable & 2)
				irqcounter = irqlatch;
			cpu_clear_irq(IRQ_MAPPER);
			break;
		case 0xD000:
			if(irqenable & 1)
				irqenable |= 2;
			else
				irqenable &= ~2;
			cpu_clear_irq(IRQ_MAPPER);
			break;
		case 0xE000:
			control = data & 0xF;
			break;
		case 0xF000:
			switch(control) {
				case 1:
				case 2:
				case 3:
				case 4:
					prg[control - 1] = (prg[control - 1] & 0x10) | (data & 0xF);
					break;
			}
			switch(addr & 0xFC00) {
				case 0xF000:
					prg[addr & 3] &= 0xF;
					prg[addr & 3] |= (data & 0x10);
					break;
				case 0xF400:
					break;
				case 0xF800:
					mirror = data & 1;
					break;
				case 0xFC00:
					chr[addr & 7] = data;
					break;
			}
			sync();
			break;
	}
}

static void reset(int hard)
{
	int i;

	mem_setwramsize(2);
	for(i=8;i<16;i++)
		mem_setwritefunc(i,write);
	if(hard) {
		for(i=0;i<8;i++) {
			prg[i & 3] = 0;
			chr[i] = 0;
		}
		mirror = 0;
		irqenable = 0;
		irqlatch = irqcounter = 0;
	}
	sync();
}

static void cpucycle()
{
	if((irqenable & 2) == 0)
		return;
	if(irqcounter == 0xFFFF) {
		irqcounter = irqlatch;
		cpu_set_irq(IRQ_MAPPER);
	}
	else
		irqcounter++;
}

static void state(int mode,u8 *data)
{

}

MAPPER(B_KAISER_KS202,reset,0,cpucycle,state);
