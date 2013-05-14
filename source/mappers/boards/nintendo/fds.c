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
static u8 diskread,writeskip;
static u8 diskirq,timerirq;
static u8 irqenable,ioenable,control,status;
static u16 irqcounter,irqlatch;
static int diskaddr;
static int diskflip;
static int hlefds;
static char hleident[6] = "HLEFDS";

//global (for hle fds bios)
int diskside;

void hlefds_write(u32 addr,u8 data);

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
//			log_printf("fds.c:  irq ack.\n");
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

		//hlefds register
		case 0x4222:
			if(hlefds == 0)
				break;
			hlefds_write(addr,data);
			break;
	}
}

static void reset(int hard)
{
	u8 *bios;

	//setup for fds
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

	//get pointer to bios rom
	bios = mem_getreadptr(0xE);

	//see if we have the hlefds bios loaded
	if(memcmp(bios,hleident,6) == 0) {
		log_printf("fds.c:  reset:  hlefds bios found, version %d.%d\n",bios[6],bios[7]);
		hlefds = 1;
	}
	else
		hlefds = 0;

	sync();
}

#define TAKEOVER(addr,hle) \
	if(nes.cpu.opaddr == addr) { \
		nes.cpu.readpages[addr >> 12][addr & 0xFFF] = 0x60; \
		hlefds_write(0x4222,hle); \
	}

static void cpucycle()
{
	//debugging the bios calls!
	static u16 lastopaddr = 0;
	static struct {int type;u8 hle;u16 addr;char *name;} funcaddrs[] = {

		//vectors
		{0,	0x38,		0xe18b,	"NMI"},
		{0,	0x39,		0xe1c7,	"IRQ"},
		{0,	0x3A,		0xee24,	"RESET"},

		//disk
		{1,	0x00,		0xe1f8,	"LoadFiles"},
		{1,	0xFF,		0xe237,	"AppendFile"},
		{1,	0xFF,		0xe239,	"WriteFile"},
		{1,	0xFF,		0xe2b7,	"CheckFileCount"},
		{1,	0xFF,		0xe2bb,	"AdjustFileCount"},
		{1,	0xFF,		0xe301,	"SetFileCount1"},
		{1,	0xFF,		0xe305,	"SetFileCount"},
		{1,	0xFF,		0xe32a,	"GetDiskInfo"},

		//util
		{2,	0xFF,		0xe149,	"Delay132"},
		{2,	0xFF,		0xe153,	"Delayms"},
		{2,	0xFF,		0xe161,	"DisPFObj"},
		{2,	0xFF,		0xe16b,	"EnPFObj"},
		{2,	0xFF,		0xe170,	"DisObj"},
		{2,	0xFF,		0xe178,	"EnObj"},
		{2,	0xFF,		0xe17e,	"DisPF"},
		{2,	0xFF,		0xe185,	"EnPF"},
		{2,	0xFF,		0xe1b2,	"VINTWait"},
		{2,	0xFF,		0xe7bb,	"VRAMStructWrite"},
		{2,	0xFF,		0xe844,	"FetchDirectPtr"},
		{2,	0xFF,		0xe86a,	"WriteVRAMBuffer"},
		{2,	0xFF,		0xe8b3,	"ReadVRAMBuffer"},
		{2,	0xFF,		0xe8d2,	"PrepareVRAMString"},
		{2,	0xFF,		0xe8e1,	"PrepareVRAMStrings"},
		{2,	0xFF,		0xe94f,	"GetVRAMBufferByte"},
		{2,	0xFF,		0xe97d,	"Pixel2NamConv"},
		{2,	0xFF,		0xe997,	"Nam2PixelConv"},
		{2,	0xFF,		0xe9b1,	"Random"},
		{2,	0xFF,		0xe9c8,	"SpriteDMA"},
		{2,	0xFF,		0xe9d3,	"CounterLogic"},
		{2,	0xFF,		0xe9eb,	"ReadPads"},
		{2,	0xFF,		0xea0d,	"ReadOrPads"},
		{2,	0xFF,		0xea1a,	"ReadDownPads"},
		{2,	0xFF,		0xea1f,	"ReadOrDownPads"},
		{2,	0xFF,		0xea36,	"ReadDownVerifyPads"},
		{6,	0xFF,		0xea4c,	"ReadOrDownVerifyPads"},
		{2,	0xFF,		0xea68,	"ReadDownExpPads"},
		{2,	0xFF,		0xea84,	"VRAMFill"},
		{2,	0xFF,		0xead2,	"MemFill"},
		{2,	0xFF,		0xeaea,	"SetScroll"},
		{2,	0xFF,		0xeafd,	"JumpEngine"},
		{2,	0xFF,		0xeb13,	"ReadKeyboard"},
		{2,	0x1C,		0xeb66,	"LoadTileset"},
		{0,0}
	};
	if(nes.cpu.opaddr >= 0xE000 && nes.cpu.opaddr != lastopaddr) {
		int i,found = 0;

		for(i=0;funcaddrs[i].addr;i++) {
			if(funcaddrs[i].addr == nes.cpu.opaddr) {
				int t = funcaddrs[i].type;
				char *types[4] = {"VECTOR","DISK","UTIL","MISC"};

				if(t < 4)
					log_printf("fds.c:  bios %s:  calling $%04X ('%s')\n",types[t],funcaddrs[i].addr,funcaddrs[i].name);
				found = 1;
			}
		}
	}

//force hle!  takeover!
	//if the opaddr changes we are reading an opcode
	if(nes.cpu.opaddr >= 0xE000 && nes.cpu.opaddr != lastopaddr) {
		TAKEOVER(0xea84,0x18);		//vramfill
		TAKEOVER(0xe7bb,0x19);		//vramstructwrite
//		TAKEOVER(0xeb66,0x1C);		//loadtileset
		TAKEOVER(0xead2,0x20);		//memfill
	}
//end force

	lastopaddr = nes.cpu.opaddr;

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
