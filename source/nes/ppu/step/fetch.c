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
	if(CONTROL1 & 0x08) {
		nes.ppu.ntbyte = ppu_memread(nes.ppu.busaddr);

		//mapper tile callback
		nes.mapper->tile(nes.ppu.ntbyte,((CONTROL0 & 0x10) >> 2) >> 2);
	}
}

static INLINE void fetch_atbyte()
{
	u32 tmp;

	if(CONTROL1 & 0x08) {
#ifdef CACHE_ATTRIB
		tmp = SCROLL & 0xFFF;
		nes.ppu.attribdata[nes.ppu.fetchpos] = nes.ppu.attribpages[tmp >> 10][tmp & 0x3FF];
#else
		tmp = ((ppu_memread(nes.ppu.busaddr) >> (((SCROLL & 2) | (((SCROLL >> 5) & 2) << 1)))) & 3);
		nes.ppu.attribdata[nes.ppu.fetchpos] = tmp;
#endif
	}
}

static INLINE void fetch_pt0byte()
{
	if(CONTROL1 & 0x08) {
		cache_t *cache,pixels;

		nes.ppu.tiledata[0][nes.ppu.fetchpos] = ppu_memread(nes.ppu.busaddr);

		//tile bank cache pointer
		cache = nes.ppu.cachepages[(nes.ppu.ntbyte >> 6) | ((CONTROL0 & 0x10) >> 2)];

		//index to the tile data start, then the tile half (upper or lower half)
		cache += ((nes.ppu.ntbyte & 0x3F) * 2) + ((SCROLL >> 14) & 1);

		//retreive the tile pixels used
		pixels = *cache >> (((SCROLL >> 12) & 3) << 1);

		nes.ppu.cachedata[nes.ppu.fetchpos] = pixels & CACHE_MASK;
	}
}

static INLINE void fetch_pt1byte()
{
	if(CONTROL1 & 0x08) {
		nes.ppu.tiledata[1][nes.ppu.fetchpos] = ppu_memread(nes.ppu.busaddr);
	}
}

static INLINE void fetch_spt0byte()
{
}

static INLINE void fetch_spt1byte()
{
}
