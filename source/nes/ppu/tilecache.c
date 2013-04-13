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

#include "nes/ppu/tilecache.h"

//blargg's wonderful cache scheme, found on nesdev forums

// Expands each of the 8 bits in n into separate bytes of result.
// In: 12345678  Out: 0x87654321
static INLINE u64 expand(u8 n)
{
	u64 ret = 0;

	ret |= (u64)((n >> 7) & 1) << 0;
	ret |= (u64)((n >> 6) & 1) << 8;
	ret |= (u64)((n >> 5) & 1) << 16;
	ret |= (u64)((n >> 4) & 1) << 24;
	ret |= (u64)((n >> 3) & 1) << 32;
	ret |= (u64)((n >> 2) & 1) << 40;
	ret |= (u64)((n >> 1) & 1) << 48;
	ret |= (u64)((n >> 0) & 1) << 56;
	return(ret);
}

// Expands each of the 8 bits in n into separate bytes of result.
// In: 12345678  Out: 0x87654321
static INLINE u64 expand_hflip(u8 n)
{
	u64 ret = 0;

	ret |= (u64)((n >> 0) & 1) << 0;
	ret |= (u64)((n >> 1) & 1) << 8;
	ret |= (u64)((n >> 2) & 1) << 16;
	ret |= (u64)((n >> 3) & 1) << 24;
	ret |= (u64)((n >> 4) & 1) << 32;
	ret |= (u64)((n >> 5) & 1) << 40;
	ret |= (u64)((n >> 6) & 1) << 48;
	ret |= (u64)((n >> 7) & 1) << 56;
	return(ret);
}

// convert one chr tile to a cached tile
void cache_tile(u8 *chr,cache_t *cache)
{
	int n;

	for(n=2;n--;) {
		*cache++ =	(expand(chr[ 0]) << 0) |
						(expand(chr[ 8]) << 1) |
						(expand(chr[ 1]) << 2) |
						(expand(chr[ 9]) << 3) |
						(expand(chr[ 2]) << 4) |
						(expand(chr[10]) << 5) |
						(expand(chr[ 3]) << 6) |
						(expand(chr[11]) << 7);
		chr += 4;
	}
}

// convert one chr tile to a cached tile, horizontally flipped
void cache_tile_hflip(u8 *chr,cache_t *cache)
{
	int n;

	for(n=2;n--;) {
		*cache++ =	(expand_hflip(chr[ 0]) << 0) |
						(expand_hflip(chr[ 8]) << 1) |
						(expand_hflip(chr[ 1]) << 2) |
						(expand_hflip(chr[ 9]) << 3) |
						(expand_hflip(chr[ 2]) << 4) |
						(expand_hflip(chr[10]) << 5) |
						(expand_hflip(chr[ 3]) << 6) |
						(expand_hflip(chr[11]) << 7);
		chr += 4;
	}
}

void cache_tiles(u8 *chr,cache_t *cache,int num,int hflip)
{
	int n;

	for(n=0;n<num;n++) {
		if(hflip != 0)
			cache_tile_hflip(chr,cache);
		else
			cache_tile(chr,cache);
		chr += 16;
		cache += CACHE_TILE_SIZE;
	}
}
