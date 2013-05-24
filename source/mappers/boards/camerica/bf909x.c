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

#include "mappers/mapperinc.h"

static u8 prg,outerprg;
static u8 prgmask,mirroring;
static int type;

static void sync()
{
	mem_setprg16(0x8,outerprg | prg);
	mem_setprg16(0xC,outerprg | (0xFF & prgmask));
	mem_setvram8(0,0);
	mem_setmirroring(mirroring);
}

static void write_mirroring(u32 addr,u8 data)
{
	mirroring = MIRROR_1L + ((data >> 4) & 1);
	sync();
}

static void write_prgselect(u32 addr,u8 data)
{
	prg = data & prgmask;
	sync();
}

static void write_outerprgselect(u32 addr,u8 data)
{
	outerprg = data;
	sync();
}

static void reset(int t,int hard)
{
	type = t;
	if(type == B_CAMERICA_BF9093) {
		prgmask = 0xF;
	}
	else if(type == B_CAMERICA_BF9096) {
		prgmask = 3;
		mem_setwritefunc(0x8,write_outerprgselect);
		mem_setwritefunc(0x9,write_outerprgselect);
		mem_setwritefunc(0xA,write_outerprgselect);
		mem_setwritefunc(0xB,write_outerprgselect);
	}
	else if(type == B_CAMERICA_BF9097) {
		prgmask = 7;
		mem_setwritefunc(0x8,write_mirroring);
		mem_setwritefunc(0x9,write_mirroring);
	}
	mem_setwritefunc(0xC,write_prgselect);
	mem_setwritefunc(0xD,write_prgselect);
	mem_setwritefunc(0xE,write_prgselect);
	mem_setwritefunc(0xF,write_prgselect);
	mem_setvramsize(8);
	prg = outerprg = 0;
	mirroring = nes->cart->mirroring;
	sync();
}

static void state(int mode,u8 *data)
{
	STATE_U8(prg);
	STATE_U8(outerprg);
	STATE_U8(mirroring);
	sync();
}

static void reset_bf9093(int hard)
{
	reset(B_CAMERICA_BF9093,hard);
}

static void reset_bf9096(int hard)
{
	reset(B_CAMERICA_BF9096,hard);
}

static void reset_bf9097(int hard)
{
	reset(B_CAMERICA_BF9097,hard);
}

MAPPER(B_CAMERICA_BF9093,reset_bf9093,0,0,state);
MAPPER(B_CAMERICA_BF9096,reset_bf9096,0,0,state);
MAPPER(B_CAMERICA_BF9097,reset_bf9097,0,0,state);
