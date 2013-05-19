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

#include "misc/log.h"
#include "misc/config.h"
#include "nes/io.h"
#include "nes/nes.h"
#include "system/input.h"

//read from apu/joypads
u8 nes_read_4000(u32 addr)
{
	switch(addr) {
		//apu registers
		case 0x4000:	case 0x4001:	case 0x4002:	case 0x4003:
		case 0x4004:	case 0x4005:	case 0x4006:	case 0x4007:
		case 0x4008:	case 0x4009:	case 0x400A:	case 0x400B:
		case 0x400C:	case 0x400D:	case 0x400E:	case 0x400F:
		case 0x4010:	case 0x4011:	case 0x4012:	case 0x4013:
		case 0x4015:
			return(apu_read(addr));

		//sprite memory read
		case 0x4014:
			return(nes.ppu.oam[nes.ppu.oamaddr]);

		//input port 0 read
		case 0x4016:
			return(nes.inputdev[0]->read());

		//input port 1 read
		case 0x4017:
			return(nes.inputdev[1]->read());

		default:
			if(log_unhandled_io)
				log_printf("nes_read_4000:  unhandled read at $%04X\n",addr);
			break;
	}
	return(0);
}

//write to sprite dma, nes joypad strobe, and apu registers
void nes_write_4000(u32 addr,u8 data)
{
	u32 temp,temp2;

	switch(addr) {
		//apu registers
		case 0x4000:	case 0x4001:	case 0x4002:	case 0x4003:
		case 0x4004:	case 0x4005:	case 0x4006:	case 0x4007:
		case 0x4008:	case 0x4009:	case 0x400A:	case 0x400B:
		case 0x400C:	case 0x400D:	case 0x400E:	case 0x400F:
		case 0x4010:	case 0x4011:	case 0x4012:	case 0x4013:
		case 0x4015:	case 0x4017:
			apu_write(addr,data);
			break;

		//sprite dma write
		case 0x4014:
			temp2 = data << 8;
			for(temp=0;temp<256;temp++,temp2++)
				cpu_write(0x2004,cpu_read(temp2));
			temp2 = 513 + ((u32)nes.cpu.cycles & 1);
			for(temp=0;temp<temp2;temp++)
				cpu_tick();
			break;

		//strobe joypads
		case 0x4016:
			nes.inputdev[0]->write(data);
			nes.inputdev[1]->write(data);
			nes.expdev->write(data);
			if(((data & 1) == 0) && (nes.strobe & 1)) {
				nes.inputdev[0]->strobe();
				nes.inputdev[1]->strobe();
				nes.expdev->strobe();
			}
			nes.strobe = data;
			break;

		default:
			if(log_unhandled_io)
				log_printf("nes_write_4000:  unhandled write at $%04X = $%02X\n",addr,data);
			break;
	}
}

u8 nes_read_mem(u32 addr)
{
	if(nes.cpu.readpages[addr >> 10])
		return(nes.cpu.readpages[addr >> 10][addr & 0x3FF]);
	return(0);
}

void nes_write_mem(u32 addr,u8 data)
{
	if(nes.cpu.writepages[addr >> 10])
		nes.cpu.writepages[addr >> 10][addr & 0x3FF] = data;
}

//read nes rom memory area ($8000-FFFF)
u8 nes_read_rom(u32 addr)
{
	return(nes.cpu.readpages[addr >> 10][addr & 0x3FF]);
}
