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
#include "mappers/sound/s_FME7.h"
#include "nes/nes.h"

static apu_external_t fme7 = {
	FME7sound_Load,
	FME7sound_Unload,
	FME7sound_Reset,
	FME7sound_Get,
	0
};

static u8 prg[4],chr[8],mirror;
static u16 irqcounter;
static u8 irqcontrol,command;

static void sync()
{
	int i;

	switch((prg[0] >> 6) & 3) {
		case 0:
		case 2:
			mem_setprg8(6,prg[0] & 0x3F);
			break;
		case 1:
			//open bus
			mem_unsetcpu8(6);
			break;
		case 3:
			mem_setsram8(6,0);
			break;
	}
	mem_setprg8(0x8,prg[1]);
	mem_setprg8(0xA,prg[2]);
	mem_setprg8(0xC,prg[3]);
	mem_setprg8(0xE,0xFF);
	for(i=0;i<8;i++)
		mem_setchr1(i,chr[i]);
	switch(mirror) {
		case 0:	mem_setmirroring(MIRROR_V);	break;
		case 1:	mem_setmirroring(MIRROR_H);	break;
		case 2:	mem_setmirroring(MIRROR_1L);	break;
		case 3:	mem_setmirroring(MIRROR_1H);	break;
	}
}

static void write(u32 addr,u8 data)
{
	switch(addr & 0xE000) {
		case 0x8000:
			command = data & 0xF;
			break;
		case 0xA000:
			switch(command) {
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
					chr[command] = data;
					break;
				case 0x8:
				case 0x9:
				case 0xA:
				case 0xB:
					prg[command & 3] = data;
					break;
				case 0xC:
					mirror = data;
					break;
				case 0xD:
					irqcontrol = data;
					if((data & 1) == 0)
						cpu_clear_irq(IRQ_MAPPER);
					break;
				case 0xE:
					irqcounter = (irqcounter & 0xFF00) | data;
					break;
				case 0xF:
					irqcounter = (irqcounter & 0x00FF) | (data << 8);
					break;
			}
			sync();
			break;
	}
}

static void reset(int hard)
{
	int i;

	mem_setsramsize(2);
	for(i=8;i<16;i++)
		mem_setwritefunc(i,write);
	if(hard) {
		for(i=0;i<8;i++) {
			prg[i & 3] = 0;
			chr[i] = 0;
		}
		mirror = 0;
		irqcounter = 0xFFFF;
		irqcontrol = 0;
		command = 0;
	}
	apu_setexternal(&fme7);
	sync();
}

static void cpucycle()
{
	if(irqcontrol & 0x80) {
		irqcounter--;
	}
	if(irqcounter == 0) {
		if(irqcontrol & 1)
			cpu_set_irq(IRQ_MAPPER);
		irqcontrol = 0;
	}
}

static void state(int mode,u8 *data)
{
	STATE_ARRAY_U8(prg,4);
	STATE_ARRAY_U8(chr,8);
	STATE_U8(mirror);
	STATE_U16(irqcounter);
	STATE_U8(irqcontrol);
	STATE_U8(command);
	sync();
}

MAPPER(B_SUNSOFT_5B,reset,0,cpucycle,state);
