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

static INLINE void fetch_ntbyte()
{
	//read byte from ppu memory area
	nes->ppu.ntbyte = ppu_memread(nes->ppu.busaddr);
}

static INLINE void fetch_atbyte(int tilenum)
{
	u32 tmp;

	//get attribute byte
	tmp = ppu_memread(nes->ppu.busaddr);

	//calculate which set of bits to use for attributes here
	tmp = ((tmp >> (((SCROLL & 2) | (((SCROLL >> 5) & 2) << 1)))) & 3);

	//put attributes into the line buffer
	((u64*)nes->ppu.tilebuffer)[tilenum] = tmp * 0x0404040404040404LL;
}

static INLINE void fetch_pt0byte(int tilenum)
{
	cache_t *cache,pixels;

	//perform the read, but throw the data away
	ppu_memread(nes->ppu.busaddr);

	//tile bank cache pointer
	cache = nes->ppu.cachepages[(nes->ppu.ntbyte >> 6) | ((CONTROL0 & 0x10) >> 2)];

	//index to the tile data start, then the tile half (upper or lower half)
	cache += ((nes->ppu.ntbyte & 0x3F) * 2) + ((SCROLL >> 14) & 1);

	//retreive the tile pixels used
	pixels = *cache >> (((SCROLL >> 12) & 3) << 1);

	//mask off the pixels we need
	pixels &= CACHE_MASK;

	//add the pixels to the line buffer
	((u64*)nes->ppu.tilebuffer)[tilenum] += pixels;
}

static INLINE void fetch_pt1byte()
{
	//perform the read, but throw the data away
	ppu_memread(nes->ppu.busaddr);
}

static INLINE void fetch_spt0byte()
{
	cache_t *cache;

	//perform the read, but throw the data away
	ppu_memread(nes->ppu.busaddr);

	//get cache bank used by sprite tile
	if(sprtemp[nes->ppu.cursprite].flags & 0x40)
		cache = nes->ppu.cachepages_hflip[(nes->ppu.busaddr >> 10) & 7];
	else
		cache = nes->ppu.cachepages[(nes->ppu.busaddr >> 10) & 7];

	//offset to the current tile line
	cache += (nes->ppu.busaddr & 0x3FF) / 8;

	//store sprite tile line
	sprtemp[nes->ppu.cursprite].line = *cache >> (nes->ppu.busaddr & 6);
	sprtemp[nes->ppu.cursprite].line &= CACHE_MASK;
}

static INLINE void fetch_spt1byte()
{
	//perform the read, but throw the data away
	ppu_memread(nes->ppu.busaddr);

	//increase our sprite pointer
	nes->ppu.cursprite++;
}
