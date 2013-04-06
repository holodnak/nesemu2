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

#include "nes/nes.h"

u8 ppu_read(u32 addr)
{
	u8 ret = 0;

	switch(addr & 7) {
		case 2:
			//TODO: nmi suppression
			//bottom 5 bits come from the $2007 buffer
			ret = (nes.ppu.status & 0xE0) | (nes.ppu.buf & 0x1F);
			//clear vblank flag
			if(ret & 0x80)
				nes.ppu.status &= 0x60;
			nes.ppu.toggle = 0;
			return(ret);
		case 4:
			return(nes.ppu.oam[nes.ppu.oamaddr]);
		case 7:
			break;
	}
	return(nes.ppu.buf);
}

void ppu_write(u32 addr,u8 data)
{
	switch(addr & 7) {
		case 0:
			nes.ppu.control0 = data;
			nes.ppu.tmpscroll = (nes.ppu.tmpscroll & 0x73FF) | ((data & 3) << 10);
			return;
		case 1:
			nes.ppu.control1 = data;
			return;
		case 3:
			nes.ppu.oamaddr = data;
			return;
		case 4:
			nes.ppu.oam[nes.ppu.oamaddr++] = data;
			return;
		case 5:				//scroll
			if(nes.ppu.toggle == 0) { //first write
				nes.ppu.tmpscroll = (nes.ppu.tmpscroll & ~0x001F) | (data >> 3);
				nes.ppu.scrollx = data & 7;
				nes.ppu.toggle = 1;
			}
			else { //second write
				nes.ppu.tmpscroll &= ~0x73E0;
				nes.ppu.tmpscroll |= ((data & 0xF8) << 2) | ((data & 7) << 12);
				nes.ppu.toggle = 0;
			}
			return;
		case 6:				//vram addr
			if(nes.ppu.toggle == 0) { //first write
				nes.ppu.tmpscroll = (nes.ppu.tmpscroll & ~0xFF00) | ((data & 0x3F) << 8);
				nes.ppu.toggle = 1;
			}
			else { //second write
				nes.ppu.scroll = nes.ppu.tmpscroll = (nes.ppu.tmpscroll & ~0x00FF) | data;
				nes.ppu.toggle = 0;
			}
			return;
	}
}
