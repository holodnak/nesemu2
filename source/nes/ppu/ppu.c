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
#include "nes/memory.h"
#include "nes/state/state.h"

int ppu_init()
{
	state_register(B_PPU,ppu_state);
	return(0);
}

void ppu_kill()
{
}

void ppu_reset(int hard)
{
	nes.ppu.scanline = 0;
	nes.ppu.linecycles = 0;
	nes.ppu.frames = 0;
	nes.ppu.fetchpos = 0;
	nes.ppu.cursprite = 0;
}

void ppu_sync()
{
	int i;

	//cache palette data
	for(i=0;i<0x20;i++)
		ppu_pal_write(i,ppu_pal_read(i));

}

void ppu_state(int mode,u8 *data)
{
	STATE_U8(CONTROL0);
	STATE_U8(CONTROL1);
	STATE_U8(STATUS);
	STATE_U16(TMPSCROLL);
	STATE_U16(SCROLL);
	STATE_U8(SCROLLX);
	STATE_U8(TOGGLE);
	STATE_U8(nes.ppu.buf);
	STATE_U8(nes.ppu.latch);
	STATE_U8(nes.ppu.oamaddr);
	STATE_ARRAY_U8(nes.ppu.oam,256);
	STATE_U32(LINECYCLES);
	STATE_U32(SCANLINE);
	STATE_U32(FRAMES);
	STATE_ARRAY_U8(nes.ppu.nametables,0x1000);
	STATE_ARRAY_U8(nes.ppu.palette,32);
	ppu_sync();
}
