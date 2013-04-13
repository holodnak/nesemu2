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

#ifndef __nes_h__
#define __nes_h__

#include "types.h"
#include "nes/cpu/cpu.h"
#include "nes/ppu/ppu.h"
#include "nes/cart/cart.h"
#include "mappers/mappers.h"

#define CONTROL0		nes.ppu.control0
#define CONTROL1		nes.ppu.control1
#define STATUS			nes.ppu.status

#define LINECYCLES	nes.ppu.linecycles
#define SCANLINE		nes.ppu.scanline
#define FRAMES			nes.ppu.frames

#define SCROLL			nes.ppu.scroll
#define TMPSCROLL		nes.ppu.tmpscroll
#define STATUS			nes.ppu.status

//'correct' names
#define PPUCONTROL	nes.ppu.control0
#define PPUMASK		nes.ppu.control1
//old names
#define CONTROL0		nes.ppu.control0
#define CONTROL1		nes.ppu.control1

typedef struct nes_s {

	//2a03/2c02 data
	cpu_t		cpu;
	ppu_t		ppu;

	//cartridge inserted
	cart_t	*cart;

	//mapper functions
	mapper_t	*mapper;

} nes_t;

extern nes_t nes;

int nes_init();
void nes_kill();
int nes_load_cart(cart_t *c);
int nes_load(char *filename);
void nes_unload();
void nes_reset(int hard);
void nes_frame();

#endif
