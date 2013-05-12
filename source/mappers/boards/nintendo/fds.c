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
static u8 diskside,diskread,writeskip;
static u8 diskirq,timerirq;
static u8 irqenable,ioenable,control,status;
static u16 irqcounter,irqlatch;
static int diskaddr;
static int diskflip;

static void sync()
{
	mem_setmirroring(mirror);
}

static u8 read(u32 addr)
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
			cpu_clear_irq(IRQ_DISK | IRQ_TIMER);
			return(ret);

		//read data register
		case 0x4031:

			//if no disk inserted, return 0
			if(diskside == 0xFF)
				return(diskread);

			//get byte read from disk
			diskread = nes.cart->disk.data[(diskside * 65500) + diskaddr];
//			log_printf("fds.c:  $4031 read:  side = %d, diskaddr = %d, diskdata = $%02X\n",diskside,diskaddr,diskread);

			//increment disk data address
			if(diskaddr < 64999)
				diskaddr++;

			//setup disk irq cycles
			diskirq = SHORTIRQ;

			//clear irq status
			cpu_clear_irq(IRQ_DISK);

			return(diskread);

		//drive status
		case 0x4032:
			ret = 0x40;
			if(diskside == 0xFF) {
				ret |= 5;
			}
			if((diskside == 0xFF) || ((control & 1) == 0) || (control & 2)) {
				log_printf("fds_read:  disk not inserted.  (diskside = $%02X, control = $%02X)\n",diskside,control);
				ret |= 2;
			}
//			log_printf("fds.c:  read:  status = $%02X\n",ret);
			return(ret);

		//external connector read
		case 0x4033:
			return(0x80);

	}
	return((u8)(addr >> 8));
}

static void write(u32 addr,u8 data)
{
	if(addr < 0x4020) {
		write4(addr,data);
		return;
	}

//	log_printf("fds.c:  write:  $%04X = $%02X\n",addr,data);

	switch(addr) {

		//irq latch low
		case 0x4020:
			cpu_clear_irq(IRQ_TIMER);
			irqlatch = (irqlatch & 0xFF00) | data;
			break;

		//irq latch high
		case 0x4021:
			cpu_clear_irq(IRQ_TIMER);
			irqlatch = (irqlatch & 0x00FF) | (data << 8);
			break;

		//irq enable
		case 0x4022:
			cpu_clear_irq(IRQ_TIMER);
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
						nes.cart->disk.data[(diskside * 65500) + (diskaddr - 2)] = data;
//						log_printf("fds.c:  writing data to disk %d addr (%d - 2) data $%02X\n",diskside,diskaddr,data);
					}
			}
			break;

		//fds control
		case 0x4025:
			cpu_clear_irq(IRQ_DISK);
			mem_setmirroring(((data & 8) >> 3) ^ 1);
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
	}
}

static void reset(int hard)
{
	read4 = mem_getreadfunc(4);
	write4 = mem_getwritefunc(4);
	mem_setreadfunc(4,read);
	mem_setwritefunc(4,write);
	mem_setwramsize(8);
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
	sync();
}

static void cpucycle()
{
	//debugging the bios calls!
	static u16 lastopaddr = 0;
	static struct {u16 addr;char *name;} funcaddrs[] = {
		{0xe1f8,"LoadFiles"},
		{0xe237,"AppendFile"},
		{0xe239,"WriteFile"},
		{0xe2b7,"CheckFileCount"},
		{0xe2bb,"AdjustFileCount"},
		{0xe301,"SetFileCount1"},
		{0xe305,"SetFileCount"},
		{0xe32a,"GetDiskInfo"},
		{0xe149,"Delay132"},
		{0xe153,"Delayms"},
		{0xe161,"DisPFObj"},
		{0xe16b,"EnPFObj"},
		{0xe170,"DisObj"},
		{0xe178,"EnObj"},
		{0xe17e,"DisPF"},
		{0xe185,"EnPF"},
		{0xe1b2,"VINTWait"},
		{0xe7bb,"VRAMStructWrite"},
		{0xe844,"FetchDirectPtr"},
		{0xe86a,"WriteVRAMBuffer"},
		{0xe8b3,"ReadVRAMBuffer"},
		{0xe8d2,"PrepareVRAMString"},
		{0xe8e1,"PrepareVRAMStrings"},
		{0xe94f,"GetVRAMBufferByte"},
		{0xe97d,"Pixel2NamConv"},
		{0xe997,"Nam2PixelConv"},
		{0xe9b1,"Random"},
		{0xe9c8,"SpriteDMA"},
		{0xe9d3,"CounterLogic"},
		{0xe9eb,"ReadPads"},
		{0xea1a,"ReadDownPads"},
		{0xea1f,"ReadOrDownPads"},
		{0xea36,"ReadDownVerifyPads"},
		{0xea4c,"ReadOrDownVerifyPads"},
		{0xea68,"ReadDownExpPads"},
		{0xea84,"VRAMFill"},
		{0xead2,"MemFill"},
		{0xeaea,"SetScroll"},
		{0xeafd,"JumpEngine"},
		{0xeb13,"ReadKeyboard"},
		{0xeb66,"LoadTileset"},
		{0,0}
	};
	if(lastopaddr != nes.cpu.opaddr) {
		lastopaddr = nes.cpu.opaddr;
		//in bios?
		if(lastopaddr >= 0xE000) {
			int i;

			for(i=0;funcaddrs[i].addr;i++) {
				if(i >= 8) continue;
				if(funcaddrs[i].addr == lastopaddr) {
					log_printf("fds.c:  bios %s:  calling $%04X ('%s')\n",(i < 8) ? "DISK" : "UTIL",funcaddrs[i].addr,funcaddrs[i].name);
				}
			}
		}
	}
////////////////////////////////////////////////////////////////////
	if(diskflip) {
		diskflip--;
		if(diskflip == 0)
			diskside = newdiskside;
	}
	if((irqenable & 2) && irqcounter) {
		irqcounter--;
		if(irqcounter == 0) {
			if(irqenable & 1)
				irqcounter = irqlatch;
			else {
				irqenable &= ~2;
				irqcounter = 0;
				irqlatch = 0;
			}
			cpu_set_irq(IRQ_TIMER);
//			log_printf("fds.c:  IRQ!  timer!  line = %d, cycle = %d\n",SCANLINE,LINECYCLES);
		}
	}
	if(diskirq) {
		diskirq--;
		if(diskirq == 0 && (control & 0x80)) {
			cpu_set_irq(IRQ_DISK);
//			log_printf("fds.c:  IRQ!  disk!  line = %d, cycle = %d\n",SCANLINE,LINECYCLES);
		}
	}
}

static void state(int mode,u8 *data)
{
	int olddiskside = diskside;

	CFG_U8(diskside);
	if(diskside != olddiskside) {
		diskflip = (341 * 262 / 3) * 15;		//15 frames
		newdiskside = diskside;
		diskside = 0xFF;
	}
	STATE_U16(irqcounter);
	STATE_U8(irqenable);
	sync();
}

MAPPER(B_FDS,reset,0,cpucycle,state);
