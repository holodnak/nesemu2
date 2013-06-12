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

static INLINE void drawpixel()
{
	int pos = LINECYCLES - 1;
	u8 *dest = (u8*)nes->ppu.linebuffer + pos;
	u8 pixel;

	if((pos >= 8 || (nes->ppu.control1 & 2)) && (CONTROL1 & 8)) {
		pixel = nes->ppu.tilebuffer[pos + nes->ppu.scrollx];
		if((pixel & 3) == 0)
			pixel = 0;
		*dest = pixel | (nes->ppu.control1 & 0xE0);
	}
	else
		*dest = 0;
#ifdef QUICK_SPRITES
	if(CONTROL1 & 0x10) {
		sprite0_hit_check();
		if(pos >= 8 || (nes->ppu.control1 & 4)) {
			pixel = nes->ppu.spritebuffer[pos];
			if(pixel & 3) {
				if((pixel & 0x10) == 0)    //foreground sprite
					*dest = pixel | 0x10;
				else if((*dest & 3) == 0)      //background sprite that is visible
					*dest = pixel | 0x10;
			}
		}
	}
#else
	#error QUICK_SPRITES must be defined.
#endif
}

static INLINE void quick_draw_sprite_line()
{
	sprtemp_t *spr = (sprtemp_t*)sprtemp + 7;
	u64 *spriteline64 = (u64*)nes->ppu.spritebuffer;
	int n;

	//clear sprite line
	for(n=0;n<(256 / 8);n++)
		spriteline64[n] = 0;

	//loop thru all eight possible sprites
	for(n=0;n<8;n++,spr--) {
		cache_t offs;
		u64 *scr64,sp0,sp1,pmask0,pmask1;
		u64 shiftright,shiftleft;

		if(spr->flags == 0)
			continue;

		//calculate shifting amounts for aligned writes
		shiftright = (8 - (spr->x & 7)) * 8;
		shiftleft = (spr->x & 7) * 8;

		//get offset in sprite buffer
		scr64 = ((u64*)nes->ppu.spritebuffer) + (spr->x / 8);

		//setup to draw the pixel
		offs = spr->attr * 0x0404040404040404LL;
		sp0 = sp1 = spr->line;
		pmask0 = pmask1 = (0x8080808080808080LL - sp0) >> 2;

		//render pixel to sprite line buffer
#ifdef __GNUC__
		if(spr->x & 7) {
#endif
			sp0 <<= shiftleft;
			sp1 >>= shiftright;
			pmask0 <<= shiftleft;
			pmask1 >>= shiftright;
			scr64[0] = ((sp0 + offs) & pmask0) | (scr64[0] & ~pmask0);
			scr64[1] = ((sp1 + offs) & pmask1) | (scr64[1] & ~pmask1);
#ifdef __GNUC__
		}
		else
			scr64[0] = ((sp0 + offs) & pmask0) | (scr64[0] & ~pmask0);
#endif
	}
}
