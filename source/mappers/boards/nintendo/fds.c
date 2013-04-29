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

#define IRQ_TIMER	1
#define IRQ_DISK	2

static readfunc_t read4;
static writefunc_t write4;
static u8 diskside,diskread,writeskip;
static u8 diskirq,timerirq;
static u8 irqenable,ioenable,control,status;
static u16 irqcounter,irqlatch;
static int diskaddr;

static void sync()
{
	mem_setmirroring(((control & 8) >> 3) ^ 1);
}

static void setirq(u8 mask)
{
	status |= mask;
	cpu_set_irq(1);
}

static void clearirq(u8 mask)
{
	status &= ~mask;
	if((status & 3) == 0) {
		cpu_set_irq(0);
	}
}

static u8 read(u32 addr)
{
	u8 ret = 0;

	//read from nes apu regs
	if(addr < 0x4020)
		return(read4(addr));

	log_printf("fds.c:  read:  $%04X\n",addr);

	//fds read
	switch(addr) {

		//disk status register
		case 0x4030:
			ret = status;
			clearirq(IRQ_TIMER | IRQ_DISK);
			return(ret);

		//read data register
		case 0x4031:

			//if no disk inserted, return 0
			if(diskside == 0xFF)
				return(diskread);

			//get byte read from disk
			diskread = nes.cart->disk.data[(diskside * 65500) + diskaddr];

			//increment disk data address
			if(diskaddr < 64999)
				diskaddr++;

			//setup disk irq cycles
			diskirq = 150;

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

	log_printf("fds.c:  write:  $%04X = $%02X\n",addr,data);

	switch(addr) {

		//irq latch low
		case 0x4020:
			clearirq(IRQ_TIMER);
			irqlatch = (irqlatch & 0xFF00) | data;
			break;

		//irq latch high
		case 0x4021:
			clearirq(IRQ_TIMER);
			irqlatch = (irqlatch & 0x00FF) | (data << 8);
			break;

		//irq enable
		case 0x4022:
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
			if(diskside != 0xFF && (control & 4) == 0 || (ioenable & 1)) {
				if(diskaddr >= 0 && diskaddr < 65500)
					if(writeskip)
						writeskip--;
					else if(diskaddr >= 2) {
						nes.cart->disk.data[(diskside * 65500) + (diskaddr - 2)] = data;
					}
			}
			break;

		//fds control
		case 0x4025:
			clearirq(IRQ_DISK);
			mem_setmirroring(((data & 8) >> 3) ^ 1);
			if(diskside == 0xFF)
				break;
			if((data & 0x40) == 0) {
				if((control & 0x40) && (data & 0x10) == 0) {
					diskaddr -= 2;
					diskirq = 200;
				}
				if(diskaddr < 0)
					diskaddr = 0;
			}
			if(data & 2) {
				diskaddr = 0;
				diskirq = 200;
			}
			if((data & 4) == 0) {
				writeskip = 2;
			}
			if(data & 0x40)
				diskirq = 200;
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
	mem_setsramsize(8);
	mem_setvramsize(8);
	mem_setsram8(0x6,0);
	mem_setsram8(0x8,1);
	mem_setsram8(0xA,2);
	mem_setsram8(0xC,3);
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
	sync();
}

static void cpucycle()
{
	if((irqenable & 2) && irqcounter) {
		irqcounter--;
		if(irqcounter == 0) {
			if(irqenable & 1)
				irqcounter = irqlatch;
			else
				irqenable &= 1;
			setirq(IRQ_TIMER);
			log_printf("IRQ!  timer!\n");
		}
	}
	if(diskirq) {
		diskirq--;
		if(diskirq == 0) {
			setirq(IRQ_DISK);
			log_printf("IRQ!  disk!\n");
		}
	}
}

static void state(int mode,u8 *data)
{
	CFG_U8(diskside);
	STATE_U16(irqcounter);
	STATE_U8(irqenable);
	sync();
}

MAPPER(B_FDS,reset,0,0,cpucycle,state);
