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

#ifndef __nes__cart_h__
#define __nes__cart_h__

#include "types.h"
#include "nes/ppu/tilecache.h"

//mirroring types
#define MIRROR_H			0
#define MIRROR_V			1
#define MIRROR_1L			2
#define MIRROR_1H			3
#define MIRROR_4			4
#define MIRROR_MAPPER	5

//battery flags
#define BATTERY_SRAM		1
#define BATTERY_VRAM		2

//ines 2.0 ppu
#define RP2C03B		0
#define RP2C03G		1
#define RP2C04_0001	2
#define RP2C04_0002	3
#define RP2C04_0003	4
#define RP2C04_0004	5
#define RC2C03B		6
#define RC2C03C		7
#define RC2C05_01		8
#define RC2C05_02		9
#define RC2C05_03		10
#define RC2C05_04		11
#define RC2C05_05		12

//ines 2.0 vs
#define VS_NORMAL				0
#define VS_RBI_BASEBALL		1
#define VS_TKO_BOXING		2
#define VS_SUPER_XEVIOUS	3

typedef struct data_s {
	u32 size,mask;
	u8 *data;
} data_t;

typedef struct cart_s {

	//rom information
	data_t	prg,chr;					//prg/chr data
	data_t	vram,wram;				//vram/wram data
	data_t	sram,svram;				//battery backed ram/vram
	data_t	trainer;					//trainer data
	data_t	pc10rom;					//pc10 rom data

	//internal mapper id
	int		mapperid;

	//mirroring, battery flag
	u8			mirroring,battery;

	//tv type (ntsc,pal)
	u8			tvmode;

	//name of game
	char		title[512];

	//cached tile data
	cache_t	*cache,*cache_hflip;		//chr cache
	cache_t	*vcache,*vcache_hflip;	//vram cache

} cart_t;

cart_t *cart_load(const char *filename);
void cart_unload(cart_t *r);
void cart_setsramsize(cart_t *r,int banks);
void cart_setwramsize(cart_t *r,int banks);
void cart_setvramsize(cart_t *r,int banks);
void cart_setsvramsize(cart_t *r,int banks);

#endif
