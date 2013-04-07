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

#include "nes/nes.h"
#include "log/log.h"

#define CONTROL0		nes.ppu.control0
#define CONTROL1		nes.ppu.control1
#define STATUS			nes.ppu.status

#define LINECYCLES	nes.ppu.linecycles
#define SCANLINE		nes.ppu.scanline
#define FRAMES			nes.ppu.frames

void ppu_step()
{
	u32 lastcycle = 341;

	//vblank
	if(SCANLINE < 20) {
	}

	//dummy scanline
	else if(SCANLINE == 20) {
		if(FRAMES & 1) {
			lastcycle = 340;
		}
	}

	//scanlines 21 - 260 (visible lines)
	else if(SCANLINE < 261) {
	}

	//last scanline
	else if(SCANLINE == 261) {

		//last cycle
		if(LINECYCLES == 340) {
//			log_printf("ppu_step:  scanline %d, cycle %d:  setting VBLANK\n",SCANLINE,LINECYCLES);
			STATUS |= 0x80;
			if(CONTROL0 & 0x80)
				cpu_set_nmi(1);
		}
	}

	LINECYCLES++;
	if(LINECYCLES >= lastcycle) {
		LINECYCLES = 0;
		SCANLINE++;
		if(SCANLINE >= 262) {
			SCANLINE = 0;
			FRAMES++;
		}
	}
}
