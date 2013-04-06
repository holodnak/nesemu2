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

#ifndef __ppu_h__
#define __ppu_h__

#include "types.h"

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
	u8		nametables[0x800];
	u8		oam[0x100];
	u8		oam2[32];
	u8		palette[32];

	//screen buffer
	u16	screen[256 * 240];

	//read/write pointers
	u8		*readpages[16];
	u8		*writepages[16];

	//line cycle counter, scanline counter and frame counter
	u32	linecycles;
	u32	scanline;
	u32	frames;

} ppu_t;

int ppu_init();
void ppu_kill();
void ppu_reset(int hard);
u8 ppu_read(u32 addr);
void ppu_write(u32 addr,u8 data);
void ppu_step();

#endif
