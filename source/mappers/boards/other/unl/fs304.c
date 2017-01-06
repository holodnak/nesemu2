/***************************************************************************
*   Copyright (C) 2016 by James Holodnak                                   *
*   jamesholodnak@gmail.com                                                *
*                                                                          *
*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published by   *
*   the Free Software Foundation; either version 2 of the License, or      *
*   (at your option) any later version.                                    *
*                                                                          *
*   This program is distributed in the hope that it will be useful,        *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
*   GNU General Public License for more details.                           *
*                                                                          *
*   You should have received a copy of the GNU General Public License      *
*   along with this program; if not, write to the                          *
*   Free Software Foundation, Inc.,                                        *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.              *
***************************************************************************/

#include "mappers/mapperinc.h"
#include "mappers/chips/latch.h"

static u8 regs[4];

static u8 _prgBank;

static void sync()
{
	int bank;

//	log_printf("fs304: sync: data = %02X\n", data);
/*
	switch (regs[3] & 7) {
	case 0:	//000
	case 2: //010
		bank = (regs[0] & 0xc) | (regs[1] & 2) | ((regs[2] & 0xf) << 4);
		break;

	case 1: //001
	case 3: //011
		bank = (regs[0] & 0xc) | (regs[2] & 0xf) << 4;
		break;

	case 4: //100
	case 6: //110
		bank = (regs[0] & 0xe) | ((regs[1] >> 1) & 1) | ((regs[2] & 0xf) << 4);
		break;

	case 5: //101
	case 7: //111
		bank = (regs[0] & 0xf) | ((regs[2] & 0xf) << 4);
		break;
	}

//	bank = ((regs[2] & 0x0f) << 4) | ((regs[1] >> 1) & 1) | (regs[0] & 0x0e);

	bank = regs[0] & 0xF;
	bank |= (regs[1] & 0xF) << 4;
	mem_setprg32(8, bank);
	*/

	bank = (regs[0] & 0xe) | ((regs[1] >> 1) & 1) | ((regs[2] & 0xf) << 4);

	mem_setprg32(8, bank);
}

static void write(u32 addr, u8 data)
{
	if(addr == 0x5300)
	log_printf("fs304: write: %04X = %02X (pc = $%04X)\n", addr, data,nes->cpu.pc);
	regs[(addr >> 8) & 3] = data;

	sync();
}

static void write_high(u32 addr, u8 data)
{
	log_printf("fs304: write: %04X = %02X\n", addr, data);
}

static void reset(int hard)
{
	int i;

	_prgBank = 0xF;

	for (i = 8; i < 16; i++) {
		mem_setwritefunc(i, write_high);
	}
	mem_setwramsize(8);
	mem_setvramsize(8);
	mem_setwram8(6, 0);
	mem_setvram8(0, 0);
	mem_setwritefunc(0x5, write);
//	mem_setwritefunc(0xD, write);
	regs[0] = 0xF;
	regs[1] = 0xF;
	regs[2] = 0xF;
	regs[3] = 0xF;
	sync();
}

MAPPER(B_UNL_FS304, reset, 0, 0, latch_state);
