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

int ppu_init()
{
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
}

void ppu_setmirroring2(int n0,int n1,int n2,int n3);

	//move this to the memory.c file
void ppu_setmirroring(int m)
{
	switch(m) {
		default:
		case MIRROR_H: ppu_setmirroring2(0,0,1,1); break;
		case MIRROR_V: ppu_setmirroring2(0,1,0,1); break;
		case MIRROR_1L:ppu_setmirroring2(0,0,0,0); break;
		case MIRROR_1H:ppu_setmirroring2(1,1,1,1); break;
		case MIRROR_4:	ppu_setmirroring2(0,1,2,3); break;
	}
}

//move this to the memory.c file
void ppu_setmirroring2(int n0,int n1,int n2,int n3)
{
	mem_setnt1(0,n0);
	mem_setnt1(1,n1);
	mem_setnt1(2,n2);
	mem_setnt1(3,n3);
}
