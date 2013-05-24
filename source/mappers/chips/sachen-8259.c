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
#include "mappers/chips/sachen-8259.h"

static u8 cmd,reg[8];
static writefunc_t write4;
static void (*syncchr)();

void sachen8259_sync()
{
	mem_setprg32(8,reg[5] & 7);
	if(reg[7]&1)
		mem_setmirroring(MIRROR_V);
	else {
		switch(reg[7] & 6) {
			case 0: mem_setmirroring(MIRROR_V); break;
			case 2: mem_setmirroring(MIRROR_H); break;
			case 4: mem_setmirroring2(0,1,1,1); break;
			case 6: mem_setmirroring(MIRROR_1L); break;
		}
	}
	if(nes->cart->chr.size)
		syncchr();
}

void sachen8259a_syncchr()
{
	int i,j;

	for(i=0;i<4;i++) {
		if(reg[7] & 1)
			j = (reg[0] & 7) | ((reg[4] & 7) << 3);
		else
			j = (reg[i] & 7) | ((reg[4] & 7) << 3);
		mem_setchr2(i * 2,(j << 1) | (i & 1));
	}
}

void sachen8259b_syncchr()
{
	int i;

	for(i=0;i<4;i++) {
		if(reg[7] & 1)
			mem_setchr2(i * 2,(reg[0] & 7) | ((reg[4] & 7) << 3));
		else
			mem_setchr2(i * 2,(reg[i] & 7) | ((reg[4] & 7) << 3));
	}
}

void sachen8259c_syncchr()
{
	int i,j;

	for(i=0;i<4;i++) {
		if(reg[7] & 1)
			j = (reg[0] & 7) | ((reg[4] & 7) << 3);
		else
			j = (reg[i] & 7) | ((reg[4] & 7) << 3);
		mem_setchr2(i * 2,(j << 2) | (i & 3));
	}
}

void sachen8259d_syncchr()
{
	int i,j;

	for(i=0;i<4;i++) {
		j = reg[i] & 7;
		switch(i) {
			case 1: j |= (reg[4] & 1) << 4; break;
			case 2: j |= (reg[4] & 2) << 3; break;
			case 3: j |= ((reg[4] & 4) << 2) | ((reg[6] & 1) << 3); break;
		}
		mem_setchr1(i,j);
	}
	mem_setchr4(4,~0);
}

void sachen8259_write(u32 addr,u8 data)
{
	if(addr < 0x4020) {
		write4(addr,data);
		return;
	}
	addr &= 0x101;
	if(addr == 0x100)
		cmd = data & 7;
	else {
		reg[cmd] = data;
		sachen8259_sync();
	}
}

void sachen8259_reset(int revision,int hard)
{
	int i;

	switch(revision) {
		default:
		case B_SACHEN_8259A: syncchr = sachen8259a_syncchr; break;
		case B_SACHEN_8259B: syncchr = sachen8259b_syncchr; break;
		case B_SACHEN_8259C: syncchr = sachen8259c_syncchr; break;
		case B_SACHEN_8259D: syncchr = sachen8259d_syncchr; break;
	}
	write4 = mem_getwritefunc(4);
	for(i=4;i<8;i++)
		mem_setwritefunc(i,sachen8259_write);
	for(i=0;i<8;i++)
		reg[i] = 0;
	cmd = 0;
	sachen8259_sync();
}

void sachen8259_state(int mode,u8 *data)
{
	STATE_ARRAY_U8(reg,8);
	STATE_U8(cmd);
	sachen8259_sync();
}
