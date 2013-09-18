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
#include "mappers/chips/mmc1.h"

static void reset(int type,int hard)
{
	if(nes->cart->chr.size == 0)
		mem_setvramsize(8);
	mem_setwramsize(nes->cart->wram.size < 8 ? 8 : nes->cart->wram.size);
	mmc1_reset(type,mmc1_sync,hard);
}

static void reset_a(int hard)	{	reset(C_MMC1A,hard);	}
static void reset_b(int hard)	{	reset(C_MMC1B,hard);	}
static void reset_c(int hard)	{	reset(C_MMC1C,hard);	}

MAPPER(B_NINTENDO_SxROM_MMC1A,reset_a,0,0,mmc1_state);
MAPPER(B_NINTENDO_SxROM_MMC1B,reset_b,0,0,mmc1_state);
MAPPER(B_NINTENDO_SxROM_MMC1C,reset_c,0,0,mmc1_state);
