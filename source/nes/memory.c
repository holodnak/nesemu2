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

#include "memory.h"
#include "nes.h"

//set read page function pointer
void mem_setreadfunc(int page,readfunc_t readfunc)
{
	nes.cpu.readfuncs[page] = readfunc;
}

//set read page data pointer
void mem_setreadpage(int page,u8 *ptr)
{
	nes.cpu.readpages[page] = ptr;
}

//set write page function pointer
void mem_setwritefunc(int page,writefunc_t writefunc)
{
	nes.cpu.writefuncs[page] = writefunc;
}

//set write page data pointer
void mem_setwritepage(int page,u8 *ptr)
{
	nes.cpu.writepages[page] = ptr;
}

//retreive read page function pointer
readfunc_t mem_getreadfunc(int page)
{
	return(nes.cpu.readfuncs[page]);
}

//retreive read page data pointer
u8 *mem_getreadpage(int page)
{
	return(nes.cpu.readpages[page]);
}

//retreive write page function pointer
writefunc_t mem_getwritefunc(int page)
{
	return(nes.cpu.writefuncs[page]);
}

//retreive write page data pointer
u8 *mem_getwritepage(int page)
{
	return(nes.cpu.writepages[page]);
}

void mem_unsetcpu(int banksize,int page)
{
	int i;

	for(i=0;i<(banksize/4);i++) {
		nes.cpu.readpages[page + i] = 
		nes.cpu.writepages[page + i] = 0;
	}
}

void mem_unsetppu(int banksize,int page)
{
	int i;

	for(i=0;i<banksize;i++) {
		nes.ppu.readpages[page + i] = 
		nes.ppu.writepages[page + i] = 0;
	}
}

void mem_setprg(int banksize,int page,int bank)
{
	int i;
	u8 *ptr = nes.cart->prg.data + ((bank * banksize * 1024) & nes.cart->prg.mask);

	for(i=0;i<(banksize/4);i++) {
		nes.cpu.readpages[page + i] = ptr + i * 0x1000;
		nes.cpu.writepages[page + i] = 0;
	}
}

void mem_setsram(int banksize,int page,int bank)
{
	int i;
	u8 *ptr = nes.cart->sram.data + ((bank * banksize * 1024) & nes.cart->sram.mask);

	for(i=0;i<(banksize/4);i++) {
		nes.cpu.readpages[page + i] = 
		nes.cpu.writepages[page + i] = ptr + i * 0x1000;
	}
}

void mem_setwram(int banksize,int page,int bank)
{
	int i;
	u8 *ptr = nes.cart->wram.data + ((bank * banksize * 1024) & nes.cart->wram.mask);

	for(i=0;i<(banksize/4);i++) {
		nes.cpu.readpages[page + i] = 
		nes.cpu.writepages[page + i] = ptr + i * 0x1000;
	}
}

void mem_setnt(int banksize,int page,int bank)
{
	int i,offset = (bank * banksize * 1024) & 0x7FF;

	for(i=0;i<banksize;i++) {
		nes.ppu.readpages[page + i] = nes.ppu.nametables + offset + (i * 1024);
		nes.ppu.writepages[page + i] = nes.ppu.nametables + offset + (i * 1024);
	}
}

void mem_setchr(int banksize,int page,int bank)
{
	int i,offset = (bank * banksize * 1024) & nes.cart->chr.mask;

	for(i=0;i<banksize;i++) {
		nes.ppu.readpages[page + i] = nes.cart->chr.data + offset + (i * 1024);
		nes.ppu.writepages[page + i] = 0;
	}
}

void mem_setvram(int banksize,int page,int bank)
{
	int i,offset = (bank * banksize * 1024) & nes.cart->vram.mask;

	for(i=0;i<banksize;i++) {
		nes.ppu.readpages[page + i] = 
		nes.ppu.writepages[page + i] = nes.cart->vram.data + offset + (i * 1024);
	}
}

void mem_setsramsize(int banks)
{
	cart_setsramsize(nes.cart,banks);
}
