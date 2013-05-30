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
#include "mappers/sound/s_FDS.h"
#include "mappers/sound/s_FME7.h"
#include "mappers/sound/s_MMC5.h"
#include "mappers/sound/s_N106.h"
#include "mappers/sound/s_VRC6.h"
#include "mappers/sound/s_VRC7.h"
#include "misc/log.h"

static readfunc_t old_read_vectors;
static u8 *bios;

static void sound_init()
{
	FDSsound_Load();
	FME7sound_Load();
	MMC5sound_Load();
	N106sound_Load();
	VRC6sound_Load();
	VRC7sound_Load();
}

static u8 sound_read(u32 addr)
{

}

static u8 sound_write(u32 addr)
{

}

static u8 read_vectors(u32 addr)
{
	u8 *ptr = nes->cpu.readpages[addr >> 10];

	if(addr >= 0xFFFA)
		return(bios[addr & 0xF]);
	if(ptr)
		return(ptr[addr & 0x3FF]);
	log_printf("nsf.c:  nothing mapped at $%04X\n",addr);
	return(0);
}

static u8 read(u32 addr)
{
	return(0);
}

extern int showdisasm;

//write nsf bios registers
static void write(u32 addr,u8 data)
{
	log_printf("nsf.c:  write $%04X = $%02X\n",addr,data);

	switch(addr) {

		//nsf bios bankswitch ($3C00)
		case 0x3400:
			nes->cpu.readpages[0x3C00 >> 10] = bios + (data * 0x400);
			break;
		case 0x3408:
			showdisasm = data ? 1 : 0;
			break;
	}
}

static void reset(int hard)
{
	bios = nes->cart->sram.data;

	//manually setup the read vector handler
	nes->cpu.readfuncs[0xFFFF >> 10] = read_vectors;

	//clear these functions to pass thru to the memory pointers
	nes->cpu.readfuncs[0x3800 >> 10] = nes->cpu.readfuncs[0x3C00 >> 10] = 0;
	nes->cpu.writefuncs[0x3800 >> 10] = nes->cpu.writefuncs[0x3C00 >> 10] = 0;

	//manually setup the nsf bios register read/write handlers
	nes->cpu.readfuncs[0x3400 >> 10] = read;
	nes->cpu.writefuncs[0x3400 >> 10] = write;

	//setup fixed bank
	nes->cpu.readpages[0x3800 >> 10] = bios;

	//setup vram
	mem_setvramsize(8);
	mem_setvram8(0,0);

	mem_setmirroring(MIRROR_H);
}

static void cpucycle()
{
}
	
static void state(int mode,u8 *data)
{
}

MAPPER(B_NSF,reset,0,cpucycle,state);
