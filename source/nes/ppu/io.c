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
#include "nes/nes.h"

u8 ppu_memread(u32 addr)
{
	if(nes.ppu.readpages[addr >> 10])
		return(nes.ppu.readpages[addr >> 10][addr & 0x3FF]);
//	else if(nes.ppu.readfuncs[addr >> 10])
//		return(nes.ppu.readfuncs[addr >> 10](addr));
	log_printf("ppu_memread: read from unmapped memory at $%04X\n",addr);
	return(0);
}

void ppu_memwrite(u32 addr,u8 data)
{
	u8 page = (addr >> 10) & 0xF;

	//check if mapped to memory pointer
	if(nes.ppu.writepages[page])
		nes.ppu.writepages[page][addr & 0x3FF] = data;

	//see if write function is mapped
//	else if(nes->ppu.writefuncs[page])
//		nes->ppu.writefuncs[page](addr,data);

	//not mapped, report error
	else
		log_printf("ppu_memwrite: write to unmapped memory at $%04X = $%02X\n",addr,data);

	//if address is in chr ram, update the cache
	if(addr < 0x2000) {
		if((addr & 0xF) == 0xF) {
			cache_t *cache = nes.ppu.cachepages[page];
			cache_t *cache_hflip = nes.ppu.cachepages_hflip[page];
			u8 *chr = nes.ppu.readpages[page];
			u32 a = addr & 0x3F0;

			convert_tile(chr + a,cache + (a / 8));
			convert_tile_hflip(chr + a,cache_hflip + (a / 8));
		}
	}
}

u8 pal_read(u32 addr)
{
	return(nes.ppu.palette[addr]);
}

void pal_write(u32 addr,u8 data)
{
	nes.ppu.palette[addr] = data;
//	video_setpalentry(addr,palette[data].r,palette[data].g,palette[data].b);
}

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
//			log_message("2007 read: ppuscroll = $%04X, scanline = %d\n",nes->ppu.scroll,nes->scanline);
			nes.ppu.buf = nes.ppu.latch;
			SCROLL &= 0x3FFF;
			nes.ppu.latch = ppu_memread(SCROLL);
			if((SCROLL & 0x3F00) == 0x3F00)
				nes.ppu.buf = pal_read(SCROLL & 0x1F);
			if(CONTROL0 & 4)
				SCROLL += 32;
			else
				SCROLL += 1;
			return(nes.ppu.buf);
	}
	return(nes.ppu.buf);
}

void ppu_write(u32 addr,u8 data)
{
	int i;

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
		case 7:				//vram data
			if(nes.ppu.scroll < 0x3F00) {
				ppu_memwrite(nes.ppu.scroll,data);
			}
			else {
				if((nes.ppu.scroll & 0x0F) == 0) {
					for(i=0;i<8;i++)
						pal_write(i * 4,data);
				}
				else if(nes.ppu.scroll & 3)
					pal_write(nes.ppu.scroll & 0x1F,data);
			}
			if(nes.ppu.control0 & 4)
				nes.ppu.scroll += 32;
			else
				nes.ppu.scroll += 1;
			nes.ppu.scroll &= 0x3FFF;
			return;
	}
}
