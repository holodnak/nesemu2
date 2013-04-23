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

static INLINE void inc_hscroll()
{
	if(CONTROL1 & 0x18) {
		/*	The first one, the horizontal scroll counter, consists of 6 bits, and is
		made up by daisy-chaining the HT counter to the H counter. The HT counter is
		then clocked every 8 pixel dot clocks (or every 8/3 CPU clock cycles). */
		if((SCROLL & 0x1F) == 0x1F)		//see if HT counter creates carry
			SCROLL ^= 0x41F;					//yes, clear lower 5 bits and toggle H counter
		else
			SCROLL++;							//no, increment address
	}
	nes.ppu.fetchpos++;
}

static INLINE void inc_vscroll()
{
	int n;

	if(CONTROL1 & 0x18) {
		//update y coordinate
		if((SCROLL >> 12) == 7) {
			SCROLL &= ~0x7000;
			n = (SCROLL >> 5) & 0x1F;
			if(n == 29) {
				SCROLL &= ~0x03E0;
				SCROLL ^= 0x0800;
			}
			else if(n == 31)
				SCROLL &= ~0x03E0;
			else
				SCROLL += 0x20;
		}
		else
			SCROLL += 0x1000;
	}
}

static INLINE void update_hscroll()
{
	int i;

	if(CONTROL1 & 0x18) {
		SCROLL &= ~0x041F;
		SCROLL |= TMPSCROLL & 0x041F;
	}
	nes.ppu.fetchpos = 0;
	nes.ppu.cursprite = 0;
	for(i=0;i<(32 + 2);i++) {
		nes.ppu.attribdata[i] = 0;
		nes.ppu.cachedata[i] = 0;
		nes.ppu.tiledata[0][i] = 0;
		nes.ppu.tiledata[1][i] = 0;
	}
}

static INLINE void update_scroll()
{
	if(CONTROL1 & 0x18)
		SCROLL = TMPSCROLL;
}
