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
#include "mappers/chips/mmc1.h"

static u8 prglock,irqenabled,dip;
static u32 irqmax,irqcounter;

static void sync()
{
	static u8 oldreg1 = 0x10;
	u8 reg1 = mmc1_getlowchr();

	mmc1_syncmirror();
	mmc1_syncsram();
	mem_setvram8(0,0);
	if(reg1 & 0x10) {
		irqenabled = 0;
		irqcounter = 0;
		cpu_clear_irq(IRQ_MAPPER);
	}
	else
		irqenabled = 1;
	if(prglock) {
		mem_setprg32(8,0);
		if((oldreg1 & 0x10) == 0 && (reg1 & 0x10) == 0x10)
			prglock = 0;
	}
	else {
		if(reg1 & 8)
			mmc1_syncprg(7,8);
		else
			mem_setprg32(8,(reg1 >> 1) & 3);
	}
	oldreg1 = reg1;
}

static void reset(int hard)
{
	mem_setsramsize(2);
	mem_setvramsize(8);
	prglock = 1;
	irqcounter = 0;
	irqenabled = 0;
	dip = 4;
	irqmax = 0x20000000 | (dip << 25);
	mmc1_init(sync);
}

static void cpucycle()
{
	if(irqenabled == 0)
		return;
	irqcounter++;
	if(irqcounter >= irqmax)
		cpu_set_irq(IRQ_MAPPER);
}

static void state(int mode,u8 *data)
{
	if(mode >= 2)log_printf("mapper:  B_NINTENDO_EVENT:  mode = %d, dip = %X\n",mode,*data);
	CFG_U8(dip);
	STATE_U8(dip);
	STATE_U8(prglock);
	STATE_U8(irqenabled);
	STATE_U32(irqcounter);
	mmc1_state(mode,data);
}

MAPPER(B_NINTENDO_EVENT,reset,0,cpucycle,state);
