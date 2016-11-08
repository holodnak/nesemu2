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

#include <string.h>
#include "mappers/mapperinc.h"
#include "mappers/sound/s_FDS.h"
#include "mappers/fds/hle.h"
#include "misc/log.h"
#include "misc/config.h"

#define SHORTIRQ	100
#define LONGIRQ	150

static apu_external_t fdssound = {
	FDSsound_Load,
	FDSsound_Unload,
	FDSsound_Reset,
	FDSsound_Get,
	0
};

static readfunc_t read4;
static writefunc_t write4;
static u8 mirror,newdiskside;
static u8 diskread,writeskip;
static u8 diskirq,timerirq;
static u8 irqenable,ioenable,control,status;
static u16 irqcounter,irqlatch;
static int diskaddr;
static int diskflip;

//hle
static int hlefds = 0;
static char hleident[6] = "HLEFDS";

//global (for hle fds bios)
int diskside;

void hlefds_write(u32 addr,u8 data);

static void setirq(u8 mask)
{
	status |= mask;
	cpu_set_irq(mask);
}

static void clearirq(u8 mask)
{
	status &= ~mask;
	cpu_clear_irq(mask);
}

static void sync()
{
	mem_setmirroring(mirror);
}

static u8 fds_read(u32 addr)
{
	u8 ret = 0;

	//read from nes apu regs
	if(addr < 0x4020)
		return(read4(addr));

	//fds read
	switch(addr) {

		//status register
		case 0x4030:
			ret = status;
//			log_printf("fds.c:  irq ack. (%02X - %02X)\n",nes->cpu.prev_irqstate,nes->cpu.irqstate);
			clearirq(IRQ_DISK | IRQ_TIMER);
			return(ret);

		//read data register
		case 0x4031:

			//if no disk inserted, return 0
			if(diskside == 0xFF)
				return(diskread);

			//get byte read from disk
			diskread = nes->cart->disk.data[(diskside * 65500) + diskaddr];
//			log_printf("fds.c:  $4031 read:  side = %d, diskaddr = %d, diskdata = $%02X\n",diskside,diskaddr,diskread);

			//increment disk data address
			if(diskaddr < 64999)
				diskaddr++;

			//setup disk irq cycles
			diskirq = SHORTIRQ;

			//clear irq status
			clearirq(IRQ_DISK);

			return(diskread);

		//drive status
		case 0x4032:
			ret = 0x40;
			if(diskside == 0xFF) {
				ret |= 5;
			}
			if((diskside == 0xFF) || ((control & 1) == 0) || (control & 2)) {
//				log_printf("fds_read:  disk not inserted.  (diskside = $%02X, control = $%02X)\n",diskside,control);
				ret |= 2;
			}
//			log_printf("fds.c:  read $4032:  status = $%02X\n",ret);
			return(ret);

		//external connector read
		case 0x4033:
			return(0x80);

	}

	//hlefds registers/memory
	if(addr >= 0x4222 && addr < 0x4300) {
		if(hlefds)
			return(hlefds_read(addr));
	}

	return((u8)(addr >> 8));
}

static void fds_write(u32 addr,u8 data)
{
	if(addr < 0x4020) {
		write4(addr,data);
		return;
	}

//	log_printf("fds.c:  write:  $%04X = $%02X\n",addr,data);
	switch(addr) {

		//irq latch low
		case 0x4020:
//			log_printf("fds.c:  irq write:  $%04X = $%02X\n",addr,data);
			clearirq(IRQ_TIMER);
			irqlatch = (irqlatch & 0xFF00) | data;
			break;

		//irq latch high
		case 0x4021:
//			log_printf("fds.c:  irq write:  $%04X = $%02X\n",addr,data);
			clearirq(IRQ_TIMER);
			irqlatch = (irqlatch & 0x00FF) | (data << 8);
			break;

		//irq enable
		case 0x4022:
//			log_printf("fds.c:  irq enable:  $%04X = $%02X\n",addr,data);
			clearirq(IRQ_TIMER);
			irqenable = data;
			irqcounter = irqlatch;
			break;

		//i/o enable
		case 0x4023:
			ioenable = data;
			break;

		//write data
		case 0x4024:
			if(diskside != 0xFF && (control & 4) == 0 && (ioenable & 1)) {
//				clearirq(IRQ_DISK);
				if(diskaddr >= 0 && diskaddr < 65500)
					if(writeskip)
						writeskip--;
					else if(diskaddr >= 2) {
						nes->cart->disk.data[(diskside * 65500) + (diskaddr - 2)] = data;
//						log_printf("fds.c:  writing data to disk %d addr (%d - 2) data $%02X\n",diskside,diskaddr,data);
					}
			}
			break;

		//fds control
		case 0x4025:
//			log_printf("fds.c:  $4025 write: %02X (diskaddr == %d)\n", data, diskaddr);
			clearirq(IRQ_DISK);
			mirror = ((data & 8) >> 3) ^ 1;
			mem_setmirroring(mirror);
			if(diskside == 0xFF)
				break;
			if((data & 0x40) == 0) {
				if((control & 0x40) && (data & 0x10) == 0) {
					diskaddr -= 2;
					diskirq = LONGIRQ;
				}
				if(diskaddr < 0)
					diskaddr = 0;
			}
			if(data & 2) {
				diskaddr = 0;
				diskirq = LONGIRQ;
//				log_printf("fds.c:  transfer reset!  $4025.1 set\n");
			}
			if((data & 4) == 0) {
				writeskip = 2;
//				log_printf("fds.c:  write mode!  writeskip = 2\n");
			}
			if(data & 0x40) {
				diskirq = LONGIRQ;
//				log_printf("fds.c:  read/write start!  diskirq = %d\n",diskirq);
			}
			control = data;
			break;

		//external connector
		case 0x4026:
			break;

		//hlefds registers
		case 0x4220:
		case 0x4221:
		case 0x4222:
		case 0x4223:
		case 0x4224:
		case 0x4225:
			if(hlefds)
				hlefds_write(addr,data);
			break;
	}
}

static void fds_reset(int hard)
{
	u8 *bios;

	//setup for fds
	read4 = mem_getreadfunc(4);
	write4 = mem_getwritefunc(4);
	mem_setreadfunc(4, fds_read);
	mem_setwritefunc(4, fds_write);
	mem_setwramsize(32);
	mem_setvramsize(8);
	mem_setwram32(6,0);
	mem_setprg8(0xE,0);
	mem_setvram8(0,0);
	if(hard) {
		diskside = 0xFF;
	}
	ioenable = 0;
	control = 0;
	status = 0x80;
	irqlatch = 0;
	irqcounter = 0;
	irqenable = 0;
	diskirq = 0;
	writeskip = 0;
	diskflip = 0;
	apu_setexternal(&fdssound);

	//get pointer to bios rom
	bios = mem_getreadptr(0xE);

	//turn off
	hlefds = 0;

	//see if we have the hlefds bios loaded
	if(memcmp(bios,hleident,6) == 0) {
		log_printf("fds.c:  reset:  hlefds bios found, version %d.%d\n",bios[6],bios[7]);
		hlefds = 1;
	}
	else if(config_get_bool("nes.fds.hle")) {
		log_printf("fds.c:  reset:  hle supplementing original bios\n");
		hlefds = 2;
	}

	sync();
}

void hlefds_cpucycle2();

static void fds_cpucycle()
{
	if(hlefds == 2)
		hlefds_intercept();
	else if(hlefds == 1)
		hlefds_cpucycle();

	//for disk flipping
	if(diskflip) {
		diskflip--;
		if(diskflip == 0)
			diskside = newdiskside;
	}

	//timer irq
	if((irqenable & 2) && irqcounter) {
		irqcounter--;
		if(irqcounter == 0) {
			if(irqenable & 1)
				irqcounter = irqlatch;
			else {
				irqenable &= 1;
			}
			setirq(IRQ_TIMER);
//			log_printf("fds.c:  IRQ!  timer!  cycle = %d, line = %d, frame %d\n",LINECYCLES,SCANLINE,FRAMES);
		}
	}

	//disk irq
	if(diskirq) {
		diskirq--;
		if(diskirq == 0 && (control & 0x80)) {
			setirq(IRQ_DISK);
//			log_printf("fds.c:  IRQ!  disk!  line = %d, cycle = %d\n",SCANLINE,LINECYCLES);
		}
	}
}

static void fds_state(int mode, u8 *data)
{
	int olddiskside = diskside;

	CFG_U8(diskside);
	if(diskside != olddiskside) {
		diskflip = (341 * 262 / 3) * 60;		//60 frames
		newdiskside = diskside;
		diskside = 0xFF;
	}
	STATE_INT(diskside);
	STATE_U8(mirror);
	STATE_U8(diskread);
	STATE_U8(writeskip);
	STATE_U8(diskirq);
	STATE_U8(timerirq);
	STATE_U8(irqenable);
	STATE_U8(ioenable);
	STATE_U8(control);
	STATE_U8(status);
	STATE_U16(irqcounter);
	STATE_U16(irqlatch);
	STATE_INT(diskaddr);
	STATE_INT(diskflip);
	sync();
}

MAPPER(B_FDS, fds_reset, 0, fds_cpucycle, fds_state);

static int prgbase;
static u8 vectors[16] = { 
	0,0,0,0,
	0x4C, 0x64, 0xE1, 0x00, 0x00, 0x00,
	0x6C, 0xFC, 0xDF, 0x00, 0x00, 0x00,
};

static u8 gdram[1024 * 1024];
static u8 gdbank = 0;
static u8 gdrun = 0;

static u8 doctor_read(u32 addr)
{
	u8 ret = 0;

	if (addr < 0x4100) {
		ret = fds_read(addr);
		return(ret);
	}

	switch (addr >> 12) {

	case 0x4:
		switch (addr) {

		case 0x42FF:
			break;
		case 0x43FF:
			break;
		case 0x4410:
			break;
		case 0x4411:
			break;

		case 0x4FF0:
		case 0x4FF1:
		case 0x4FF2:
		case 0x4FF3:
		case 0x4FF4:
		case 0x4FF5:
		case 0x4FF6:
		case 0x4FF7:
		case 0x4FF8:
		case 0x4FF9:
		case 0x4FFA:
		case 0x4FFB:
		case 0x4FFC:
		case 0x4FFD:
		case 0x4FFE:
		case 0x4FFF:
			ret = vectors[addr & 0xF];
			break;
		}

		log_printf("doctor read: %04X = %02X\n", addr, ret);
		break;

	case 0x5:
		log_printf("doctor read: %04X\n", addr);
		break;

	case 0x6:
	case 0x7:
	case 0x8:
	case 0x9:
	case 0xA:
	case 0xB:
	case 0xC:
	case 0xD:
		ret = gdram[gdbank * 0x8000 + (addr & 0x7FFF)];
//		log_printf("doctor read: %04X\n", addr);
		break;

	case 0xE:
	case 0xF:
		if(gdrun == 0)
			ret = nes->cart->prg.data[prgbase + (addr & 0x1FFF)];
		else
			ret = gdram[gdbank * 0x8000 + (addr & 0x7FFF)];
		break;
	}

	return(ret);
}

static void doctor_write(u32 addr, u8 data)
{
	if (addr < 0x4100) {
		fds_write(addr, data);
		return;
	}

	switch (addr >> 12) {

	case 0x4:
		switch (addr) {

		case 0x42FF:
			vectors[5] = gdram[0x7FFA];
			vectors[6] = gdram[0x7FFB];
			vectors[0xB] = gdram[0x7FFC];
			vectors[0xC] = gdram[0x7FFD];
			gdrun = 1;
			break;
		case 0x43FF:
			break;
		case 0x4410:
			break;
		case 0x4411:
			break;
		case 0x4FFF:
			prgbase = 0x2000;
			break;
		}

		log_printf("doctor write: %04X = %02X\n", addr, data);
		break;

	case 0x5:
		log_printf("doctor write: %04X = %02X\n", addr, data);
		break;

	case 0x6:
	case 0x7:
	case 0x8:
	case 0x9:
	case 0xA:
	case 0xB:
	case 0xC:
	case 0xD:
		gdram[gdbank * 0x8000 + (addr & 0x7FFF)] = data;
//		log_printf("doctor write: %04X = %02X\n", addr, data);
		break;

	case 0xE:
	case 0xF:
		log_printf("doctor write: %04X = %02X\n", addr, data);
		break;

	}
}

static void doctor_reset(int hard)
{
	int i;

	fds_reset(hard);
	for (i = 4; i < 16; i++) {
		mem_unsetcpu4(i);
		mem_setreadfunc(i, doctor_read);
		mem_setwritefunc(i, doctor_write);
	}
	prgbase = 0x0000;
	gdbank = 0;
	gdrun = 0;
}

static void doctor_cpucycle()
{
	fds_cpucycle();
}

static void doctor_state(int mode, u8 *data)
{
	fds_state(mode, data);
}

MAPPER(B_DOCTOR, doctor_reset, 0, doctor_cpucycle, doctor_state);
