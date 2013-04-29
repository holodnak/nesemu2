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

#ifndef __nes__cpu_h__
#define __nes__cpu_h__

#include "types.h"

typedef struct cpu_s {

	//program counter
	u16	pc;

	//registers
	u8		a,x,y,sp,p;

	//seperated flags register
	struct {
		u8	c,z,i,d,b,v,n;
	} flags;

	//interrupt flags
	u8		nmistate,prev_nmistate;
	u8		irqstate,prev_irqstate;

	//are we going to do irq
	u8		irqtaken;

	//effective/temporary address
	u16	effaddr;
	u16	tmpaddr;

	//opcode info
	u8		opcode;
	u16	opaddr;

	//for byte read after opcode
	u8		tmpreg;

	//cycle counter
	u64	cycles;

	//internal memory
	u8		ram[0x800];

	//memory access pointers
	u8		*readpages[16];
	u8		*writepages[16];

	//memory access functions
	readfunc_t	readfuncs[16];
	writefunc_t	writefuncs[16];

} cpu_t;

int cpu_init();
void cpu_kill();
void cpu_reset(int hard);
u64 cpu_getcycles();
void cpu_set_nmi(u8 state);
void cpu_set_irq(u8 state);
void cpu_tick();
u8 cpu_read(u32 addr);
void cpu_write(u32 addr,u8 data);
u32 cpu_execute(u32 cycles);
void cpu_execute_frame();
u16 cpu_disassemble(char *buffer,u16 opcodepos);
void cpu_disassemble_init();
readfunc_t cpu_getreadfunc();
writefunc_t cpu_getwritefunc();
void cpu_setreadfunc(readfunc_t readfunc);
void cpu_setwritefunc(writefunc_t writefunc);
void cpu_state(int mode,u8 *data);

#endif
