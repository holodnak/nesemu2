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
		nes.ppu.cachepages[page] =
		nes.ppu.cachepages_hflip[page] = 0;
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
		nes.ppu.cachepages[page + i] = (cache_t*)((u8*)nes.cart->cache + offset + (i * 0x400));
		nes.ppu.cachepages_hflip[page + i] = (cache_t*)((u8*)nes.cart->cache_hflip + offset + (i * 0x400));
	}
}

void mem_setvram(int banksize,int page,int bank)
{
	int i,offset = (bank * banksize * 1024) & nes.cart->vram.mask;

	for(i=0;i<banksize;i++) {
		nes.ppu.readpages[page + i] = 
		nes.ppu.writepages[page + i] = nes.cart->vram.data + offset + (i * 1024);
		nes.ppu.cachepages[page + i] = (cache_t*)((u8*)nes.cart->vcache + offset + (i * 0x400));
		nes.ppu.cachepages_hflip[page + i] = (cache_t*)((u8*)nes.cart->vcache_hflip + offset + (i * 0x400));
	}
}

void mem_setsramsize(int banks)
{
	cart_setsramsize(nes.cart,banks);
}

void mem_setvramsize(int banks)
{
	cart_setvramsize(nes.cart,banks);
}

void mem_setmirroring(int m)
{
	switch(m) {
		default:
		case MIRROR_H: mem_setmirroring2(0,0,1,1); break;
		case MIRROR_V: mem_setmirroring2(0,1,0,1); break;
		case MIRROR_1L:mem_setmirroring2(0,0,0,0); break;
		case MIRROR_1H:mem_setmirroring2(1,1,1,1); break;
		case MIRROR_4:	mem_setmirroring2(0,1,2,3); break;
	}
}

//move this to the memory.c file
void mem_setmirroring2(int n0,int n1,int n2,int n3)
{
	mem_setnt1(8 + 0,n0);
	mem_setnt1(8 + 1,n1);
	mem_setnt1(8 + 2,n2);
	mem_setnt1(8 + 3,n3);
	mem_setnt1(8 + 4,n0);
	mem_setnt1(8 + 5,n1);
	mem_setnt1(8 + 6,n2);
	mem_setnt1(8 + 7,n3);
}
