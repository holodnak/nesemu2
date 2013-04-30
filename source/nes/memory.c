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

#include "nes/memory.h"
#include "nes/nes.h"
#include "misc/log.h"

void mem_setreadfunc(int page,readfunc_t func)		{	nes.cpu.readfuncs[page] = func;		}
void mem_setwritefunc(int page,writefunc_t func)	{	nes.cpu.writefuncs[page] = func;		}
void mem_setreadptr(int page,u8 *ptr)					{	nes.cpu.readpages[page] = ptr;		}
void mem_setwriteptr(int page,u8 *ptr)					{	nes.cpu.writepages[page] = ptr;		}
readfunc_t mem_getreadfunc(int page)					{	return(nes.cpu.readfuncs[page]);		}
writefunc_t mem_getwritefunc(int page)					{	return(nes.cpu.writefuncs[page]);	}
u8 *mem_getreadptr(int page)								{	return(nes.cpu.readpages[page]);		}
u8 *mem_getwriteptr(int page)								{	return(nes.cpu.writepages[page]);	}

void mem_setppureadfunc(int page,readfunc_t func)		{	nes.ppu.readfuncs[page] = func;		}
void mem_setppuwritefunc(int page,writefunc_t func)	{	nes.ppu.writefuncs[page] = func;		}
void mem_setppureadptr(int page,u8 *ptr)					{	nes.ppu.readpages[page] = ptr;		}
void mem_setppuwriteptr(int page,u8 *ptr)					{	nes.ppu.writepages[page] = ptr;		}
readfunc_t mem_getppureadfunc(int page)					{	return(nes.ppu.readfuncs[page]);		}
writefunc_t mem_getppuwritefunc(int page)					{	return(nes.ppu.writefuncs[page]);	}
u8 *mem_getppureadptr(int page)								{	return(nes.ppu.readpages[page]);		}
u8 *mem_getppuwriteptr(int page)								{	return(nes.ppu.writepages[page]);	}

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
	int i,p,offset = (bank * banksize * 1024) & 0xFFF;

	for(i=0;i<banksize;i++) {
		p = page + i;
		nes.ppu.readpages[p] = nes.ppu.nametables + offset + (i * 1024);
		nes.ppu.writepages[p] = nes.ppu.nametables + offset + (i * 1024);
#ifdef CACHE_ATTRIB
		if(p >= 8) {
			nes.ppu.attribpages[p & 3] = nes.ppu.cacheattrib[bank];
			cache_attrib(p & 3);
		}
#endif
	}
}

void mem_setchr(int banksize,int page,int bank)
{
	int i,p,offset = (bank * banksize * 1024) & nes.cart->chr.mask;

	for(i=0;i<banksize;i++) {
		p = page + i;
		nes.ppu.readpages[p] = nes.cart->chr.data + offset + (i * 1024);
		nes.ppu.writepages[p] = 0;
		nes.ppu.cachepages[p] = (cache_t*)((u8*)nes.cart->cache + offset + (i * 0x400));
		nes.ppu.cachepages_hflip[p] = (cache_t*)((u8*)nes.cart->cache_hflip + offset + (i * 0x400));
#ifdef CACHE_ATTRIB
		if(p >= 8) {
			nes.ppu.attribpages[p & 3] = nes.ppu.cacheattrib[bank];
			cache_attrib(p & 3);
		}
#endif
	}
}

void mem_setvram(int banksize,int page,int bank)
{
	int i,p,offset = (bank * banksize * 1024) & nes.cart->vram.mask;

	for(i=0;i<banksize;i++) {
		p = page + i;
		nes.ppu.readpages[p] = 
		nes.ppu.writepages[p] = nes.cart->vram.data + offset + (i * 1024);
		nes.ppu.cachepages[p] = (cache_t*)((u8*)nes.cart->vcache + offset + (i * 0x400));
		nes.ppu.cachepages_hflip[p] = (cache_t*)((u8*)nes.cart->vcache_hflip + offset + (i * 0x400));
#ifdef CACHE_ATTRIB
		if(p >= 8) {
			nes.ppu.attribpages[p & 3] = nes.ppu.cacheattrib[bank];
			cache_attrib(p & 3);
		}
#endif
	}
}

void mem_setsvram(int banksize,int page,int bank)
{
	int i,p,offset = (bank * banksize * 1024) & nes.cart->svram.mask;

	for(i=0;i<banksize;i++) {
		p = page + i;
		nes.ppu.readpages[p] = 
		nes.ppu.writepages[p] = nes.cart->svram.data + offset + (i * 1024);
		nes.ppu.cachepages[p] = (cache_t*)((u8*)nes.cart->svcache + offset + (i * 0x400));
		nes.ppu.cachepages_hflip[p] = (cache_t*)((u8*)nes.cart->svcache_hflip + offset + (i * 0x400));
#ifdef CACHE_ATTRIB
		if(p >= 8) {
			nes.ppu.attribpages[p & 3] = nes.ppu.cacheattrib[bank];
			cache_attrib(p & 3);
		}
#endif
	}
}

void mem_setwramsize(int banks)	{	cart_setwramsize(nes.cart,banks);	}
void mem_setsramsize(int banks)	{	cart_setsramsize(nes.cart,banks);	}
void mem_setvramsize(int banks)	{	cart_setvramsize(nes.cart,banks);	}
void mem_setsvramsize(int banks)	{	cart_setsvramsize(nes.cart,banks);	}

void mem_setmirroring(int m)
{
	switch(m) {
		default:
			log_printf("mem_setmirroring:  bad mirroring type %d\n",m);
		case MIRROR_H: mem_setmirroring2(0,0,1,1); break;
		case MIRROR_V: mem_setmirroring2(0,1,0,1); break;
		case MIRROR_1L:mem_setmirroring2(0,0,0,0); break;
		case MIRROR_1H:mem_setmirroring2(1,1,1,1); break;
		case MIRROR_4:	mem_setmirroring2(0,1,2,3); break;
		case MIRROR_MAPPER: break;
	}
}

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
