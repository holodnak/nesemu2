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
#include "nes/nes.h"
#include "system/video.h"
#include "misc/config.h"

readfunc_t ppu_memread;
writefunc_t ppu_memwrite;

static u8 read_ppu_memory(u32 addr)
{
	//see if a memory page is mapped
	if(nes->ppu.readpages[addr >> 10])
		return(nes->ppu.readpages[addr >> 10][addr & 0x3FF]);

	//check for read function mapped
	else if(nes->ppu.readfuncs[addr >> 10])
		return(nes->ppu.readfuncs[addr >> 10](addr));

	//spit out debug message
	else if(config_get_bool("nes.log_unhandled_io"))
		log_printf("ppu_memread: read from unmapped memory at $%04X\n",addr);

	//return open bus
	return((u8)(addr >> 8));
}

static void write_ppu_memory(u32 addr,u8 data)
{
	u8 page = (addr >> 10) & 0xF;
	cache_t *cache,*cache_hflip;

	//check if mapped to memory pointer
	if(nes->ppu.writepages[page]) {
		nes->ppu.writepages[page][addr & 0x3FF] = data;

		//we have tile cache for this page, update it
		cache = nes->ppu.cachepages[page];
		if(cache) {
			u8 *chr = nes->ppu.readpages[page];
			u32 a = addr & 0x3F0;

			cache_hflip = nes->ppu.cachepages_hflip[page];
			cache_tile(chr + a,cache + (a / 8));
			cache_tile_hflip(chr + a,cache_hflip + (a / 8));
		}
	}

	//see if write function is mapped
	else if(nes->ppu.writefuncs[page])
		nes->ppu.writefuncs[page](addr,data);

	//not mapped, report error
	else if(config_get_bool("nes.log_unhandled_io"))
		log_printf("ppu_memwrite: write to unmapped memory at $%04X = $%02X\n",addr,data);
}

readfunc_t ppu_getreadfunc()
{
	return(ppu_memread);
}

writefunc_t ppu_getwritefunc()
{
	return(ppu_memwrite);
}

void ppu_setreadfunc(readfunc_t readfunc)
{
	ppu_memread = (readfunc == 0) ? read_ppu_memory : readfunc;
}

void ppu_setwritefunc(writefunc_t writefunc)
{
	ppu_memwrite = (writefunc == 0) ? write_ppu_memory : writefunc;
}

u8 ppu_pal_read(u32 addr)
{
	return(nes->ppu.palette[addr]);
}

void ppu_pal_write(u32 addr,u8 data)
{
	nes->ppu.palette[addr] = data;
	video_updatepalette(addr,data);
}

static INLINE void r2007increment()
{
	int i;

	//see if we are rendering
	if((SCANLINE < 240 || SCANLINE == nes->region->end_line) && (CONTROL1 & 0x18)) {
		if((SCROLL >> 12) == 7) {
			SCROLL &= 0xFFF;
			i = SCROLL & 0x3E0;
			if(i == 0x3A0)
				SCROLL ^= 0xBA0;
			else if(i == 0x3E0)
				SCROLL ^= 0x3E0;
			else
				SCROLL += 0x20;
		}
		else
			SCROLL += 0x1000;
	}

	//not rendering, increment normally
	else {
		if(CONTROL0 & 4)
			SCROLL += 32;
		else
			SCROLL += 1;
		SCROLL &= 0x7FFF;
	}
}

u8 ppu_read(u32 addr)
{
	u8 ret = 0;

	switch(addr & 7) {
		case 2:
			//bottom 5 bits come from the $2007 buffer
			ret = (nes->ppu.status & 0xE0) | (nes->ppu.buf & 0x1F);

			//clear vblank flag
			if(ret & 0x80) {
				nes->ppu.status &= 0x60;
			}

			//nmi suppression
			if(SCANLINE == nes->region->vblank_start) {
				if(LINECYCLES == 1) {
					ret &= 0x7F;
					cpu_clear_nmi();
				}
				if(LINECYCLES < 4 && LINECYCLES > 1) {
					cpu_clear_nmi();
				}
			}
			TOGGLE = 0;
			nes->ppu.buf = ret;
			break;
		case 4:
			nes->ppu.buf = nes->ppu.oam[nes->ppu.oamaddr];
			if((nes->ppu.oamaddr & 3) == 2)
				nes->ppu.buf &= 0xE3;
			break;
		case 7:
			//setup io for ppu
			IOADDR = SCROLL & 0x3FFF;
			IOMODE = 5;

			//increment registers
			r2007increment();

			//handle palette reads
			if(IOADDR >= 0x3F00) {
				nes->ppu.buf &= 0xC0;
				nes->ppu.buf |= ppu_pal_read(IOADDR & 0x1F);
			}

			//regular read
			else 
				nes->ppu.buf = nes->ppu.latch;
			break;
	}
	return(nes->ppu.buf);
}

void ppu_write(u32 addr,u8 data)
{
	nes->ppu.buf = data;
	switch(addr & 7) {
		case 0:
			if((STATUS & 0x80) && (data & 0x80) && ((CONTROL0 & 0x80) == 0))
				cpu_set_nmi();
			if(((data & 0x80) == 0) && (SCANLINE == nes->region->vblank_start) && (LINECYCLES < 4))
				cpu_clear_nmi();
			CONTROL0 = data;
			TMPSCROLL = (TMPSCROLL & 0x73FF) | ((data & 3) << 10);
			return;
		case 1:
			if((data & 0x18) && (SCANLINE < 240 || SCANLINE == nes->region->end_line))
				nes->ppu.rendering = 1;
			else
				nes->ppu.rendering = 0;
			CONTROL1 = data;
			return;
		case 3:
			nes->ppu.oamaddr = data;
			return;
		case 4:
			//check if we are rendering
			if(nes->ppu.rendering)
				data = 0xFF;
			nes->ppu.oam[nes->ppu.oamaddr++] = data;
			return;
		case 5:				//scroll
			if(TOGGLE == 0) { //first write
				TMPSCROLL = (TMPSCROLL & 0x7FE0) | (data >> 3);
				SCROLLX = data & 7;
				TOGGLE = 1;
			}
			else { //second write
				TMPSCROLL &= 0x0C1F;
				TMPSCROLL |= ((data & 0xF8) << 2) | ((data & 7) << 12);
				TOGGLE = 0;
			}
			return;
		case 6:				//vram addr
			if(TOGGLE == 0) { //first write
				TMPSCROLL = (TMPSCROLL & 0x00FF) | ((data & 0x7F) << 8);
				TOGGLE = 1;
			}
			else { //second write
				SCROLL = TMPSCROLL = (TMPSCROLL & 0x7F00) | data;
				TOGGLE = 0;
			}
			return;
		case 7:				//vram data
			if((SCROLL & 0x3F00) != 0x3F00) {
				IOADDR = SCROLL & 0x3FFF;
				IODATA = data;
				IOMODE = 6;
			}
			else {
				addr = SCROLL & 0x1F;
				data &= 0x3F;
				ppu_pal_write(addr,data);
				if((addr & 3) == 0)
					ppu_pal_write(addr ^ 0x10,data);
			}
			r2007increment();
			return;
	}
}
