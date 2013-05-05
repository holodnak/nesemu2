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

#ifndef MATTAPU

#include "nes/nes.h"
#include "nes/state/state.h"
#include "misc/log.h"

static apuext_t *ext = 0;
static u8 regs[0x20];

static u8 lengths[32] = {
	0x0A,0xFE,
	0x14,0x02,
	0x28,0x04,
	0x50,0x06,
	0xA0,0x08,
	0x3C,0x0A,
	0x0E,0x0C,
	0x1A,0x0E,
	0x0C,0x10,
	0x18,0x12,
	0x30,0x14,
	0x60,0x16,
	0xC0,0x18,
	0x48,0x1A,
	0x10,0x1C,
	0x20,0x1E
};

int apu_init()
{
	state_register(B_APU,apu_state);
	return(0);
}

void apu_kill()
{
}

void apu_reset(int hard)
{
}

u8 apu_read(u32 addr)
{
//	log_printf("apu_read: $%04X\n",addr);
	return(0);
}

void apu_write(u32 addr,u8 data)
{
//	log_printf("apu_write: $%04X = $%02X\n",addr,data);
}

void apu_frame()
{
}

void apu_state(int mode,u8 *data)
{
	STATE_ARRAY_U8(regs,0x20);
}

void apu_setext(apuext_t *e)
{
	ext = e;
}

#endif
