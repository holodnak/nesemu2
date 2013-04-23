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

/*static INLINE void calc_ntaddr()
{
	nes.ppu.busaddr = 0x2000 | (SCROLL & 0xFFF);
}

static INLINE void calc_atattr()
{
	nes.ppu.busaddr = SCROLL & 0xC00;
	nes.ppu.busaddr += 0x3C0 + ((SCROLL >> 2) & 7) + (((SCROLL >> (2 + 5)) & 7) << 3);
	nes.ppu.busaddr |= 0x2000;
}

static INLINE void calc_pt0addr()
{
	nes.ppu.busaddr = (CONTROL0 & 0x10) << 8;
	nes.ppu.busaddr += nes.ppu.ntbyte * 16;
	nes.ppu.busaddr += SCROLL >> 12;
}

static INLINE void calc_pt1addr()
{
	nes.ppu.busaddr += 8;
}

static INLINE void calc_spt0addr()
{
	//process 8x16 sprite
	if(CONTROL0 & 0x20) {
		//bank to get tile from
		nes.ppu.busaddr = (sprtemp[nes.ppu.cursprite].tile & 1) << 12;

		//tile offset
		nes.ppu.busaddr += (sprtemp[nes.ppu.cursprite].tile & 0xFE) * 16;
	}

	//process 8x8 sprite
	else {
		//bank to get tile from
		nes.ppu.busaddr = (CONTROL0 & 8) << 9;

		//tile offset
		nes.ppu.busaddr += sprtemp[nes.ppu.cursprite].tile * 16;
	}
}

static INLINE void calc_spt1addr()
{
	nes.ppu.busaddr += 8;
	nes.ppu.cursprite++;
}
*/
//calculate nametable byte address
static INLINE void calc_ntaddr()
{
	nes.ppu.busaddr = 0x2000 | (SCROLL & 0xFFF);
}

//calculate attribute table byte address
static INLINE void calc_ataddr()
{
	//start with current nametable address
	nes.ppu.busaddr = SCROLL & 0xC00;

	//offset to attributes
	nes.ppu.busaddr += 0x3C0;

	//calculate the correct attribute byte
	nes.ppu.busaddr += ((SCROLL >> 2) & 7) + (((SCROLL >> (2 + 5)) & 7) << 3);

	//put us into the nametable region
	nes.ppu.busaddr |= 0x2000;
}

//calculate pattern table low byte address
static INLINE void calc_pt0addr()
{
	//select correct pattern table as determined by ppu control0
	nes.ppu.busaddr = (CONTROL0 & 0x10) << 8;

	//offset to the correct tile
	nes.ppu.busaddr += nes.ppu.ntbyte * 16;

	//account for vertical fine scrolling
	nes.ppu.busaddr += SCROLL >> 12;
}

//calculate pattern table high byte address
static INLINE void calc_pt1addr()
{
	//go to upper bits of tile line
	nes.ppu.busaddr += 8;
}

//calculate sprite tile pattern table low byte address
static INLINE void calc_spt0addr()
{
	//process 8x16 sprite
	if(CONTROL0 & 0x20) {
		//bank to get tile from
		nes.ppu.busaddr = (sprtemp[nes.ppu.cursprite].tile & 1) << 12;

		//tile offset
		nes.ppu.busaddr += (sprtemp[nes.ppu.cursprite].tile & 0xFE) * 16;
	}

	//process 8x8 sprite
	else {
		//bank to get tile from
		nes.ppu.busaddr = (CONTROL0 & 8) << 9;

		//tile offset
		nes.ppu.busaddr += sprtemp[nes.ppu.cursprite].tile * 16;
	}
}

//calculate sprite tile pattern table high byte address
static INLINE void calc_spt1addr()
{
	//go to upper bits of tile line
	nes.ppu.busaddr += 8;

	//increase our sprite pointer
	nes.ppu.cursprite++;
}
