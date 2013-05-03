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

#ifndef __memory_h__
#define __memory_h__

#include "types.h"

int mem_getprgsize();
int mem_getchrsize();

//set/get cpu page read/write pointers/functions
void mem_setreadfunc(int page,readfunc_t func);
void mem_setwritefunc(int page,writefunc_t func);
void mem_setreadptr(int page,u8 *ptr);
void mem_setwriteptr(int page,u8 *ptr);
readfunc_t mem_getreadfunc(int page);
writefunc_t mem_getwritefunc(int page);
u8 *mem_getreadptr(int page);
u8 *mem_getwriteptr(int page);

//set/get ppu page read/write pointers/functions
void mem_setppureadfunc(int page,readfunc_t func);
void mem_setppuwritefunc(int page,writefunc_t func);
void mem_setppureadptr(int page,u8 *ptr);
void mem_setppuwriteptr(int page,u8 *ptr);
readfunc_t mem_getppureadfunc(int page);
writefunc_t mem_getppuwritefunc(int page);
u8 *mem_getppureadptr(int page);
u8 *mem_getppuwriteptr(int page);

//unset cpu/ppu memory page
void mem_unsetcpu(int banksize,int page);
void mem_unsetppu(int banksize,int page);

//map to cpu memory
void mem_setprg(int banksize,int page,int rombank);
void mem_setwram(int banksize,int page,int rombank);
void mem_setsram(int banksize,int page,int rombank);

//map to ppu memory
void mem_setnt(int banksize,int page,int rombank);
void mem_setchr(int banksize,int page,int rombank);
void mem_setvram(int banksize,int page,int rombank);
void mem_setsvram(int banksize,int page,int rombank);

//setup wram/sram/vram/svram
void mem_setwramsize(int banks);
void mem_setsramsize(int banks);
void mem_setvramsize(int banks);
void mem_setsvramsize(int banks);

//mirroring
void mem_setmirroring(int t);
void mem_setmirroring2(int n0,int n1,int n2,int n3);

//macros for cpu memory
#define mem_unsetcpu4(p)		mem_unsetcpu(4,p)
#define mem_unsetcpu8(p)		mem_unsetcpu(8,p)
#define mem_unsetcpu16(p)		mem_unsetcpu(16,p)
#define mem_unsetcpu32(p)		mem_unsetcpu(32,p)
#define mem_setprg4(p,b)		mem_setprg(4,p,b)
#define mem_setprg8(p,b)		mem_setprg(8,p,b)
#define mem_setprg16(p,b)		mem_setprg(16,p,b)
#define mem_setprg32(p,b)		mem_setprg(32,p,b)
#define mem_setwram4(p,b)		mem_setwram(4,p,b)
#define mem_setwram8(p,b)		mem_setwram(8,p,b)
#define mem_setwram16(p,b)		mem_setwram(16,p,b)
#define mem_setwram32(p,b)		mem_setwram(32,p,b)
#define mem_setsram4(p,b)		mem_setsram(4,p,b)
#define mem_setsram8(p,b)		mem_setsram(8,p,b)
#define mem_setsram16(p,b)		mem_setsram(16,p,b)
#define mem_setsram32(p,b)		mem_setsram(32,p,b)

//macros for ppu memory
#define mem_unsetppu1(p)		mem_unsetppu(1,p)
#define mem_unsetppu2(p)		mem_unsetppu(2,p)
#define mem_unsetppu4(p)		mem_unsetppu(4,p)
#define mem_unsetppu8(p)		mem_unsetppu(8,p)
#define mem_setnt1(p,b)			mem_setnt(1,p,b)
#define mem_setnt2(p,b)			mem_setnt(2,p,b)
#define mem_setchr1(p,b)		mem_setchr(1,p,b)
#define mem_setchr2(p,b)		mem_setchr(2,p,b)
#define mem_setchr4(p,b)		mem_setchr(4,p,b)
#define mem_setchr8(p,b)		mem_setchr(8,p,b)
#define mem_setvram1(p,b)		mem_setvram(1,p,b)
#define mem_setvram2(p,b)		mem_setvram(2,p,b)
#define mem_setvram4(p,b)		mem_setvram(4,p,b)
#define mem_setvram8(p,b)		mem_setvram(8,p,b)
#define mem_setsvram1(p,b)		mem_setsvram(1,p,b)
#define mem_setsvram2(p,b)		mem_setsvram(2,p,b)
#define mem_setsvram4(p,b)		mem_setsvram(4,p,b)
#define mem_setsvram8(p,b)		mem_setsvram(8,p,b)

#endif
