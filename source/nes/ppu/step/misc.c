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

static INLINE void inc_linecycles()
{
	LINECYCLES++;
	if(LINECYCLES >= 341) {
		LINECYCLES = 0;
		SCANLINE++;
		if(SCANLINE >= 262) {
			SCANLINE = 0;
			FRAMES++;
		}
	}
}

static INLINE void skip_cycle()
{
	if((FRAMES & 1) && (CONTROL1 & 0x18))
		inc_linecycles();
}

static INLINE void clear_nmi_flag()
{
	STATUS = 0;
}

static INLINE void clear_nmi_line()
{
	cpu_set_nmi(0);
}

static INLINE void set_nmi()
{
	STATUS |= 0x80;
	if(CONTROL0 & 0x80)
		cpu_set_nmi(1);
}

static INLINE void mask_bg()
{
	int i;

	//hide leftmost 8 pixels
	if((CONTROL1 & 2) == 0) {
		for(i=0;i<(8 + nes.ppu.scrollx);i++) {
			nes.ppu.linebuffer[i] = 0;
		}
	}
}
