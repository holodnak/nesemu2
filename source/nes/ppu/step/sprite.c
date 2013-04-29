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

#ifndef QUICK_SPRITES

#define OAM2		nes.ppu.oam2
#define OAM2POS	nes.ppu.oam2pos
#define OAM2MODE	nes.ppu.oam2mode
#define OAM2READ	nes.ppu.oam2read

static INLINE void process_sprites()
{
	u32 tmp;

	if(LINECYCLES == 0) {
		OAM2POS = 0;
		OAM2MODE = 0;
	}

	/* Cycles 1-64: Secondary OAM (32-byte buffer for current sprites on scanline) is
	   initialized to $FF - attempting to read $2004 will return $FF */
	else if(LINECYCLES <= 64) {
		if(LINECYCLES & 1)
			OAM2[OAM2POS++] = 0xFF;
	}

	/* Cycles 65-256: Sprite evaluation
	On odd cycles, data is read from (primary) OAM
	On even cycles, data is written to secondary OAM (unless writes are inhibited, in
	   which case it will read the value in secondary OAM instead)
	1. Starting at n = 0, read a sprite's Y-coordinate (OAM[n][0], copying it to the
	   next open slot in secondary OAM (unless 8 sprites have been found, in which case the write is ignored).
	1a. If Y-coordinate is in range, copy remaining bytes of sprite data (OAM[n][1] thru
	    OAM[n][3]) into secondary OAM.
	2. Increment n
	2a. If n has overflowed back to zero (all 64 sprites evaluated), go to 4
	2b. If less than 8 sprites have been found, go to 1
	2c. If exactly 8 sprites have been found, disable writes to secondary OAM. This causes sprites in back to drop out.
	3. Starting at m = 0, evaluate OAM[n][m] as a Y-coordinate.
	3a. If the value is in range, set the sprite overflow flag in $2002 and read the next 3 entries
	    of OAM (incrementing 'm' after each byte and incrementing 'n' when 'm' overflows); if m = 3, increment n
	3b. If the value is not in range, increment n AND m (without carry). If n overflows to 0, go to 4; otherwise go to 3
	4. Attempt (and fail) to copy OAM[n][0] into the next free slot in secondary OAM, and increment n (repeat until HBLANK is reached) */
	else if(LINECYCLES <= 256) {
		//read sprite y coord
		if(OAM2MODE == 0) {
			if(LINECYCLES & 1) {
				//read byte from oam
				OAM2READ = nes.ppu.oam[nes.ppu.oamaddr];
			}
			else {
				//store y coordinate into oam2
				OAM2[OAM2POS] = OAM2READ;

				//calculate sprite height
				tmp = OAM2READ + (8 << ((CONTROL0 & 0x20) >> 5));

				//check if sprite line is visible here
				if(SCANLINE >= OAM2READ && SCANLINE < tmp) {

					//change mode to fetch the read of teh sprite data
					OAM2MODE++;

					//increment oam2 offset
					OAM2POS++;
				}

				//sprite not visible on this line
				else {

				}
			}
		}

		//fetching the rest of the sprite data
		else if(OAM2MODE == 1) {
			if(LINECYCLES & 1) {
				//read byte from oam
				OAM2READ = nes.ppu.oam[nes.ppu.oamaddr++];
			}
			else {
				//store byte into oam2
				OAM2[OAM2POS++] = OAM2READ;

				//see if this is the last fetch for this sprite
				if((OAM2POS & 3) == 0) {

					//if this is the 8th sprite, go into the strange evaluation mode
					if(OAM2POS == 32)
						OAM2MODE++;

					//otherwise fetch next sprite
					else
						OAM2MODE = 0;
				}
			}
		}

		//strange sprite evaluation
		else if(OAM2MODE == 2) {

		}
	}

	/* Cycles 257-320: Sprite fetches (8 sprites total, 8 cycles per sprite)
	1-4: Read the Y-coordinate, tile number, attributes, and X-coordinate
	     of the selected sprite from secondary OAM
	5-8: Read the X-coordinate of the selected sprite from secondary OAM
	     4 times (while the PPU fetches the sprite tile data)
	For the first empty sprite slot, this will consist of sprite #63's Y-coordinate
	followed by 3 $FF bytes; for subsequent empty sprite slots, this will be four $FF bytes */
	else if(LINECYCLES <= 320) {

	}

	/* Cycles 321-340+0: Background render pipeline initialization
    Read the first byte in secondary OAM (while the PPU fetches
	 the first two background tiles for the next scanline) */
	else {

	}
}

#else

//process all sprites that belong to the next scanline
static INLINE void quick_process_sprites()
{
	int i,h,sprinrange;
	u8 *s;

	//clear the sprite temp memory
	for(i=0;i<8;i++) {
		sprtemp[i].line = 0;
		sprtemp[i].attr = sprtemp[i].x = sprtemp[i].flags = 0;
		sprtemp[i].tile = 0xFF;
	}

	//if sprites disabled, return
	if((CONTROL1 & 0x10) == 0)
		return;

	//determine sprite height
	h = 8 + ((CONTROL0 & 0x20) >> 2);

	//loop thru all 64 visible sprites, keeping note of the first eight visible
	for(sprinrange=0,i=0;i<64;i++) {
		int sprline;
		cache_t *cache;
		u8 patternbank;

		//sprite data pointer
		s = &nes.ppu.oam[i * 4];

		//get the sprite tile line to draw
		sprline = SCANLINE - s[0];

		//check for visibility on this line
		if(sprline >= 0 && sprline < h) {

			//if more than 8 sprites are found, set the flag and exit the loop
			if(sprinrange == 8) {
				STATUS |= 0x20;
				break;
			}

			//copy sprite data to temp memory
			sprtemp[sprinrange].attr = (s[2] & 3) | ((s[2] & 0x20) >> 3);
			sprtemp[sprinrange].x = s[3];
			sprtemp[sprinrange].flags = 1;
			sprtemp[sprinrange].tile = s[1];

			//if sprite0 check is needed
			if(i == 0)
				sprtemp[sprinrange].flags = 2;

			//call mapper callback
			nes.mapper->tile((int)s[1] | (CONTROL0 & 8) << 5);

			//determine pattern bank used (upper or lower)
			if(CONTROL0 & 0x20)
				patternbank = (s[1] & 1) << 2;
			else
				patternbank = (CONTROL0 & 8) >> 1;

			//if sprite is to be flipped horizontally
			if(s[2] & 0x40)
				cache = (cache_t*)nes.ppu.cachepages_hflip[(s[1] >> 6) | patternbank];
			else
				cache = (cache_t*)nes.ppu.cachepages[(s[1] >> 6) | patternbank];

			//select tile for 8x16 sprites
			if(CONTROL0 & 0x20) {
				cache += ((s[1] & 0x3E) * 2) + ((s[2] & 0x80) >> 6);
				if(sprline >= 8) {
					if(s[2] & 0x80)
						cache -= 2;
					else
						cache += 2;
					sprline &= 7;
				}
			}

			//select tile for 8x8 sprites
			else
				cache += (s[1] & 0x3F) * 2;

			//if sprite is to be flipped vertically
			if((s[2] & 0x80) != 0)
				sprline = 7 - sprline;

			//select tile line
			cache += (sprline / 4);

			//store sprite tile line
			sprtemp[sprinrange].line = *cache >> ((sprline & 3) * 2);

			//increment sprite in range counter
			sprinrange++;

		}
	}
}

#endif
