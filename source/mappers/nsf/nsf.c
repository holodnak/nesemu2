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

static readfunc_t read4;
static writefunc_t write4;
static readfunc_t oldread;
static writefunc_t oldwrite;
static u8 *bios,*biosbank;
static u8 ram[0x40];
static u32 playspeed;
static u32 irqcounter,irqlatch;
static u8 irqenable;

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
	switch(addr & 0xF000) {
		case 0x4000:
			if(addr < 0x4020)
				return(read4(addr));
			break;
	}
	log_printf("nsf.c:  sound read $%04X\n",addr);
	return(0);
}

static void sound_write(u32 addr,u8 data)
{
	switch(addr & 0xF000) {
		case 0x4000:
			if(addr < 0x4020) {
				write4(addr,data);
				return;
			}
			break;
		case 0x5000:
			//bankswitch registers
			if(addr >= 0x5FF6) {
				log_printf("nsf.c:  bankswitch page $%X to bank %d\n",addr & 0xF,data);

				//swap rom
				if(data < 0xFE) {
					mem_setprg4(addr & 0xF,data);
				}

				//swap ram
				else {
					mem_setwram4(addr & 0xF,data & 1);
				}

				return;
			}
			break;
	}
	log_printf("nsf.c:  sound write $%04X = $%02X\n",addr,data);
}

static u8 read_bios(u32 addr)
{
	switch(addr & 0xFFC0) {

		//registers
		case 0x3400:
			switch(addr) {
				//do nothing
				case 0x3400:
					return(0);

				//irq ack
				case 0x3401:
					cpu_clear_irq(IRQ_MAPPER);
					return(irqenable);
			}
			break;

		//nsf extra ram
		case 0x3440:
			return(ram[addr & 0x3F]);

		//map the entire nsf header
		case 0x3480:
		case 0x34C0:
			return(nes->cart->data[addr & 0x7F]);

	}
	log_printf("nsf.c:  read $%04X\n",addr);
	return(0);
}

extern int showdisasm;

//write nsf bios registers
static void write_bios(u32 addr,u8 data)
{
	switch(addr & 0xFFC0) {

		//registers
		case 0x3400:
			switch(addr) {

				//nsf bios bankswitch ($3C00)
				case 0x3400:
					biosbank = bios + (data * 0x400);
					break;

				//irq enable
				case 0x3401:
					irqenable = data & 1;
					if(irqenable)
						irqcounter = irqlatch;
					cpu_clear_irq(IRQ_MAPPER);
					break;

				//irq latch low
				case 0x3402:
					irqlatch = (irqlatch & 0xFF00) | data;
					break;

				//irq latch high
				case 0x3403:
					irqlatch = (irqlatch & 0x00FF) | (data << 8);
					break;

				//playspeed low
				case 0x3410:
					playspeed = (playspeed & 0xFF00) | data;
					irqlatch = (u32)(1786840.0f * (double)playspeed / 1000000.0f);
					log_printf("irqlatch = %d (playspeed = %d)\n",irqlatch,playspeed);
					break;

				//playspeed high
				case 0x3411:
					playspeed = (playspeed & 0x00FF) | (data << 8);
					irqlatch = (u32)(1786840.0f * (double)playspeed / 1000000.0f);
					log_printf("irqlatch = %d (playspeed = %d)\n",irqlatch,playspeed);
					break;

				//show disassembly
				case 0x3420:
					showdisasm = data ? 1 : 0;
					break;

				//output data to screen
				case 0x3421:
					log_printf("nsf.c:  write:  $%04X = $%02X\n",addr,data);
					break;
			}
			break;

		//nsf extra ram
		case 0x3440:
			ram[addr & 0x3F] = data;
			break;
	}
}

static u8 read(u32 addr)
{
	switch(addr & 0xFC00) {

		//handling of the special bios stuff
		case 0x3400:	return(read_bios(addr));
		case 0x3800:	return(bios[addr & 0x3FF]);
		case 0x3C00:	return(biosbank[addr & 0x3FF]);

		//vector replacement
		case 0xFC00:
			if(addr >= 0xFFFA)
				return(bios[addr & 0xF]);
			break;

	}
	return(oldread(addr));
}

static void write(u32 addr,u8 data)
{
	switch(addr & 0xFC00) {

		//handling of the special bios stuff
		case 0x3400:
			write_bios(addr,data);
			return;
	}
	oldwrite(addr,data);
}

static void reset(int hard)
{
	int i;

	//save pointer to bios
	bios = nes->cart->wram.data + 8192;

	//init sound chips
	sound_init();

	//save original read/write funcs
	oldread = cpu_getreadfunc();
	oldwrite = cpu_getwritefunc();
	cpu_setreadfunc(read);
	cpu_setwritefunc(write);

	//save original $4000 read/write funcs
	read4 = mem_getreadfunc(4);
	write4 = mem_getwritefunc(4);

	//for reading/writing fds/namco/mmc5
	mem_setreadfunc(4,sound_read);
	mem_setreadfunc(5,sound_read);
	mem_setwritefunc(4,sound_write);
	mem_setwritefunc(5,sound_write);

	//set all upper sound write handlers
	for(i=8;i<16;i++) {
		mem_setwritefunc(i,sound_write);
	}

	//setup vram
	mem_setvramsize(8);
	mem_setvram8(0,0);

	//setup wram
	mem_setwramsize(8);

	//set mirroring
	mem_setmirroring(MIRROR_H);

	//clear nsf ram
	for(i=0;i<0x40;i++) {
		ram[i] = 0;
	}

	log_printf("nsf.c:  reset:  loadaddr = $%04X\n",nes->cart->data[8] | (nes->cart->data[9] << 8));
}

static void cpucycle()
{
	if(irqenable) {
		irqcounter--;
		if(irqcounter == 0) {
			irqcounter = irqlatch;
			cpu_set_irq(IRQ_MAPPER);
		}
	}
}
	
static void state(int mode,u8 *data)
{
}

MAPPER(B_NSF,reset,0,cpucycle,state);
