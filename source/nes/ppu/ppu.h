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

#ifndef __nes__ppu_h__
#define __nes__ppu_h__

#include "types.h"
#include "tilecache.h"
#include "attribcache.h"

typedef struct ppu_s {

	//registers
	u8		control0,control1;
	u8		status;

	//scroll/temp scroll/fine x scroll
	u32	scroll,tmpscroll;
	u8		scrollx;

	//$2005/6 flipflop
	u8		toggle;

	//sprite memory address
	u8		oamaddr;

	//$2007 buffer and latch
	u8		buf,latch;

	//internal memory
	u8		nametables[0x800 * 2];
	u8		oam[0x100];
	u8		palette[32];

	//sprite rendering
	u8		oam2[32];
	u8		oam2pos;
	u8		oam2read;
	u8		oam2mode;

	//rendering data, current address the ppu is accessing
	u32	busaddr;

	//nametable byte and position we are fetching tile data to
	u8		ntbyte,fetchpos;

	//current sprite we are fetching tile data for
	u8		cursprite;

	//line buffer (34 tiles)
	u8		linebuffer[256 + 16];

	//fetchpos gets out of hand and begins to corrupt memory!
	u8		kludge[1024];

	//read/write pointers
	u8		*readpages[16];
	u8		*writepages[16];

	//read/write functions
	readfunc_t readfuncs[16];
	writefunc_t writefuncs[16];

	//cached tile pointers
	cache_t	*cachepages[16];
	cache_t	*cachepages_hflip[16];

#ifdef CACHE_ATTRIB
	//cached attributes
	u8		cacheattrib[4][32*32];	//attribute cache for four nametables
	u8		*attribpages[4];			//attribute cache pointers
#endif

	//line cycle counter, scanline counter and frame counter
	u32	linecycles;
	u32	scanline;
	u32	frames;

} ppu_t;

extern readfunc_t ppu_memread;
extern writefunc_t ppu_memwrite;

int ppu_init();
void ppu_kill();
void ppu_reset(int hard);
u8 ppu_read(u32 addr);
void ppu_write(u32 addr,u8 data);
u8 ppu_pal_read(u32 addr);
void ppu_pal_write(u32 addr,u8 data);
void ppu_step();
void ppu_state(int mode,u8 *data);
readfunc_t ppu_getreadfunc();
writefunc_t ppu_getwritefunc();
void ppu_setreadfunc(readfunc_t readfunc);
void ppu_setwritefunc(writefunc_t writefunc);

#endif
