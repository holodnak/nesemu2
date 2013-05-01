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
#include "mappers/chips/mmc3.h"

static int type;
static void (*sync)();
static u8 command;
static u8 prg[4],chr[8];
static u8 mirror;
static u8 sramenabled;
static u8 irqlatch,irqcounter,irqenabled,irqreload,irqwait;
static u8 *sram7;

void mmc3_sync()
{
	mmc3_syncprg(0xFF,0);
	if(nes.cart->chr.size)
		mmc3_syncchr(0xFF,0);
	else
		mmc3_syncvram(7,0);
	if(nes.cart->mirroring == MIRROR_4)
	   mem_setmirroring(MIRROR_4);
	else
		mmc3_syncmirror();
	mmc3_syncsram();
}

u8 mmc3_getprgbank(int n)
{
	if(n & 1)
		return(prg[n]);
	return(prg[n ^ ((command & 0x40) >> 5)]);
}

u8 mmc3_getchrbank(int n)
{
	return(chr[n ^ ((command & 0x80) >> 5)]);
}

u8 mmc3_getchrreg(int n)
{
	return(chr[n & 7]);
}

u8 mmc3_getcommand()
{
	return(command);
}

void mmc3_syncprg(int a,int o)
{
	mem_setprg8(0x8,(mmc3_getprgbank(0) & a) | o);
	mem_setprg8(0xA,(mmc3_getprgbank(1) & a) | o);
	mem_setprg8(0xC,(mmc3_getprgbank(2) & a) | o);
	mem_setprg8(0xE,(mmc3_getprgbank(3) & a) | o);
}

void mmc3_syncchr(int a,int o)
{
	int i;

	for(i=0;i<8;i++)
		mem_setchr1(i,(mmc3_getchrbank(i) & a) | o);
}

void mmc3_syncvram(int a,int o)
{
	int i;

	for(i=0;i<8;i++)
		mem_setvram1(i,(mmc3_getchrbank(i) & a) | o);
}

void mmc3_syncsram()
{
/*	if((type & C_MMCNUM) == C_MMC3) {
		if((sramenabled & 0xC0) == 0x80)
			mem_setsram8(6,0);
		else
			mem_unsetcpu8(6);
	}*/
	if((type & C_MMCNUM) == C_MMC3)
		mem_setsram8(6,0);
}

void mmc3_syncmirror()
{
	mem_setmirroring(mirror);
}

u8 mmc6_readsram(u32 addr)
{
	if(command & 0x20) {
		if(addr >= 0x7000 && addr < 0x7200 && sramenabled & 0x20) {
			return(sram7[addr & 0x3FF]);
		}
		if(addr >= 0x7200 && addr < 0x7400 && sramenabled & 0x80) {
			return(sram7[addr & 0x3FF]);
		}
	}
	return(0);
}

void mmc6_writesram(u32 addr,u8 data)
{
	if(command & 0x20) {
		if(addr >= 0x7000 && addr < 0x7200 && sramenabled & 0x10) {
			sram7[addr & 0x3FF] = data;
		}
		if(addr >= 0x7200 && addr < 0x7400 && sramenabled & 0x40) {
			sram7[addr & 0x3FF] = data;
		}
	}
}

void mmc3_reset(int t,void (*s)(),int hard)
{
	int i;

	type = t;
	mem_setsramsize(2);
	for(i=8;i<0x10;i++)
		mem_setwritefunc(i,mmc3_write);
	if((type & C_MMCNUM) == C_MMC6) {
		mem_setsram4(7,0);
		sram7 = mem_getwriteptr(7);
		mem_unsetcpu4(7);
		mem_setreadfunc(6,mmc6_readsram);
		mem_setreadfunc(7,mmc6_readsram);
		mem_setwritefunc(6,mmc6_writesram);
		mem_setwritefunc(7,mmc6_writesram);
		sramenabled = 0;
	}
	else {
//		mem_setsram8(6,0);
		//keep enabled for now
		sramenabled = 0x80;
	}
	sync = s;
	command = 0;
	for(i=0;i<4;i++)
		prg[i] = 0x3C + i;
	for(i=0;i<8;i++)
		chr[i] = i;
	mirror = 0;
	irqcounter = irqlatch = 0;
	irqenabled = irqreload = 0;
	irqwait = 0;
	sync();
}

void mmc3_write(u32 addr,u8 data)
{
	switch(addr & 0xE001) {
		case 0x8000:
			command = data;
			if((data & 0x20) == 0)
				sramenabled = 0;
			sync();
			break;
		case 0x8001:
			switch(command & 7) {
				case 0:
					data &= 0xFE;
					chr[0] = data | 0;
					chr[1] = data | 1;
					break;
				case 1:
					data &= 0xFE;
					chr[2] = data | 0;
					chr[3] = data | 1;
					break;
				case 2:
					chr[4] = data;
					break;
				case 3:
					chr[5] = data;
					break;
				case 4:
					chr[6] = data;
					break;
				case 5:
					chr[7] = data;
					break;
				case 6:
					prg[0] = data & 0x3F;
					break;
				case 7:
					prg[1] = data & 0x3F;
					break;
			}
			sync();
			break;
		case 0xA000:
			mirror = (data & 1) ^ 1;
			sync();
			break;
		case 0xA001:
			if(type == C_MMC6) {
				if(command & 0x20) {
					sramenabled = data & 0xF0;
					sync();
				}
			}
			else {
				sramenabled = data & 0xC0;
				sync();
			}
			break;
		case 0xC000:
			irqlatch = data;
//			log_printf("mmc3_write:  WRITE!  irq latch = %d (frame %d, line %d, pixel %d)\n",data,FRAMES,SCANLINE,LINECYCLES);
			break;
		case 0xC001:
			irqcounter = 0;
			irqreload = 1;
//			log_printf("mmc3_write:  WRITE!  irq reload request (frame %d, line %d, pixel %d)\n",FRAMES,SCANLINE,LINECYCLES);
			break;
		case 0xE000:
//			log_printf("mmc3_write:  WRITE!  irq disable/ack (frame %d, line %d, pixel %d)\n",FRAMES,SCANLINE,LINECYCLES);
			irqenabled = 0;
			cpu_set_irq(0);
			break;
		case 0xE001:
			irqenabled = 1;
//			log_printf("mmc3_write:  WRITE!  irq enable (frame %d, line %d, pixel %d)\n",FRAMES,SCANLINE,LINECYCLES);
			break;
	}
}

void mmc3_ppucycle()
{
	u8 tmp;

	if(irqwait)
		irqwait--;
	if((irqwait == 0) && (nes.ppu.busaddr & 0x1000)) {
/*		if((irqcounter == 0) || irqreload) {
			irqcounter = irqlatch;
			log_printf("mmc3_cycle:  RELOADED!  irq counter = %d (frame %d, line %d, pixel %d)\n",irqcounter,FRAMES,SCANLINE,LINECYCLES);
		}
		else {
			irqcounter--;
			log_printf("mmc3_cycle:  CLOCKED!  irq counter = %d (frame %d, line %d, pixel %d)\n",irqcounter,FRAMES,SCANLINE,LINECYCLES);
			if(irqcounter == 0 && irqenabled) {
				cpu_set_irq(1);
				log_printf("mmc3_cycle:  IRQ!  (frame %d, line %d, pixel %d)\n",FRAMES,SCANLINE,LINECYCLES);
			}
		}*/
		tmp = irqcounter;
		if((irqcounter == 0) || irqreload) {
			irqcounter = irqlatch;
//			log_printf("mmc3_cycle:  RELOADED!  irq counter = %d (frame %d, line %d, pixel %d)\n",irqcounter,FRAMES,SCANLINE,LINECYCLES);
		}
		else {
			irqcounter--;
//			log_printf("mmc3_cycle:  CLOCKED!  irq counter = %d (frame %d, line %d, pixel %d)\n",irqcounter,FRAMES,SCANLINE,LINECYCLES);
		}
		if((tmp || irqreload) && (irqcounter == 0) && irqenabled) {
			cpu_set_irq(1);
//			log_printf("mmc3_cycle:  IRQ!  (frame %d, line %d, pixel %d)\n",FRAMES,SCANLINE,LINECYCLES);
		}
		irqreload = 0;
	}
	if(nes.ppu.busaddr & 0x1000) {
//		if(FRAMES >= 16)
//		log_printf("mmc3_cycle:  busaddr A12 high ($%04X), line %d, pixel %d\n",nes.ppu.busaddr,nes.ppu.scanline,nes.ppu.linecycles);
		irqwait = 8;
	}
}

void mmc3_state(int mode,u8 *data)
{
	STATE_ARRAY_U8(prg,2);
	STATE_ARRAY_U8(chr,8);
	STATE_U8(command);
	STATE_U8(mirror);
	STATE_U8(sramenabled);
	STATE_U8(irqlatch);
	STATE_U8(irqcounter);
	STATE_U8(irqenabled);
	STATE_U8(irqreload);
	STATE_U8(irqwait);
	sync();
}
