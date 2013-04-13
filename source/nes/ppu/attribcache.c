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

#include "attribcache.h"
#include "nes/nes.h"

void cache_attribbyte(u32 addr,u8 data)
{
	u32 a = addr & 0x3F;
	u8 nt = (addr >> 10) & 3;
	u32 x = (a & 7) << 2;
	u32 y = (a & 0x38) << 4;
	u8 v[4];
	u32 v32;

	v[0] = data & 3;
	v[1] = (data & 0x0C) >> 2;
	v[2] = (data & 0x30) >> 4;
	v[3] = (data & 0xC0) >> 6;
	v32 = v[0] | (v[0] << 8) | (v[1] << 16) | (v[1] << 24);
	*((u32*)&nes.ppu.cacheattrib[nt][y+x])    = v32;
	*((u32*)&nes.ppu.cacheattrib[nt][y+x+32]) = v32;
	v32 = v[2] | (v[2] << 8) | (v[3] << 16) | (v[3] << 24);
	*((u32*)&nes.ppu.cacheattrib[nt][y+x+64]) = v32;
	*((u32*)&nes.ppu.cacheattrib[nt][y+x+96]) = v32;
}

void cache_attrib(int nt)
{
	int i;
	u32 addr = 0x2000 + (nt * 0x400) + 0x3C0;

	for(i=0;i<0x40;i++) {
		cache_attribbyte(addr + i,ppu_memread(addr + i));
	}
}