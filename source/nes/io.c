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

#include "log/log.h"
#include "nes/io.h"
#include "nes/nes.h"
#include "system/input.h"

//read from apu/joypads
u8 nes_read_4000(u32 addr)
{
	//try input port 0 read
	if(addr == 0x4016)
		return(nes.inputdev[0]->read());

	//try input port 1 read
	if(addr == 0x4017)
		return(nes.inputdev[1]->read() | nes.expdev->read());

	if(addr == 0x4015) {
		u8 ret = /*apu_read(addr) |*/ nes.frame_irq;
		cpu_set_irq(0);
//		nes_frameirq = 0;
//		log_printf("nes_read_4000: $%04X\n",addr);
		return(ret);
	}
/*
	//else return an apu read
	return(apu_read(addr));*/
	return(0);
}

u8 nes_frame_irqmode = 0;

//write to sprite dma, nes joypad strobe, and apu registers
void nes_write_4000(u32 addr,u8 data)
{
/*	u32 temp,temp2;

	//write to apu
	if(addr <= 0x4013 || addr == 0x4015) {
		nes.apuregs[addr & 0x1F] = data;
		apu_write(addr,data);
	}

	//sprite dma write
	else if(addr == 0x4014) {
		temp2 = data << 8;
		for(temp=0;temp<256;temp++,temp2++)
			nes.sprmem[temp] = dead6502_read(temp2);
		nes_burn(2 * 256);
	}*/

	//strobe joypads
	if(addr == 0x4016) {
		nes.inputdev[0]->write(data);
		nes.inputdev[1]->write(data);
		nes.expdev->write(data);
		if(((data & 1) == 0) && (nes.strobe & 1)) {
			nes.inputdev[0]->strobe();
			nes.inputdev[1]->strobe();
			nes.expdev->strobe();
		}
		nes.strobe = data;
	}

	//frame irq control
	else if(addr == 0x4017) {
//		log_printf("nes_write_4000: $%04X = $%02X\n",addr,data);
		nes.frame_irqmode = data;
		nes.frame_irq = 0;
	}
}

u8 nes_read_mem(u32 addr)
{
	if(nes.cpu.readpages[addr >> 12])
		return(nes.cpu.readpages[addr >> 12][addr & 0xFFF]);
	return(0);
}

void nes_write_mem(u32 addr,u8 data)
{
	if(nes.cpu.writepages[addr >> 12])
		nes.cpu.writepages[addr >> 12][addr & 0xFFF] = data;
}

//read nes rom memory area ($8000-FFFF)
u8 nes_read_rom(u32 addr)
{
	return(nes.cpu.readpages[addr >> 12][addr & 0xFFF]);
}
