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

#include <stdlib.h>
#include <string.h>
#include "nes/nes.h"
#include "nes/ppu/ppu.h"
#include "log/log.h"

#define PC	nes.cpu.pc
#define A	nes.cpu.a
#define X	nes.cpu.x
#define Y	nes.cpu.y
#define P	nes.cpu.p
#define SP	nes.cpu.sp

#define TMPADDR	nes.cpu.tmpaddr
#define EFFADDR	nes.cpu.effaddr

#define FLAG_C		nes.cpu.flags.c
#define FLAG_Z		nes.cpu.flags.z
#define FLAG_I		nes.cpu.flags.i
#define FLAG_D		nes.cpu.flags.d
#define FLAG_B		nes.cpu.flags.b
#define FLAG_V		nes.cpu.flags.v
#define FLAG_N		nes.cpu.flags.n

#define OPCODE		nes.cpu.opcode
#define OPADDR		nes.cpu.opaddr
#define TMPREG		nes.cpu.tmpreg

#define CYCLES		nes.cpu.cycles

#define NMISTATE			nes.cpu.nmistate
#define IRQSTATE			nes.cpu.irqstate
#define PREV_NMISTATE	nes.cpu.prev_nmistate
#define PREV_IRQSTATE	nes.cpu.prev_irqstate

static u8 memread(u32 addr)
{
	//increment cycle counter, check irq lines
	cpu_tick();

	return(cpu_read(addr));
}

static void memwrite(u32 addr,u8 data)
{
	//increment cycle counter, check irq lines
	cpu_tick();

	cpu_write(addr,data);
}

//push data to stack
static void push(u8 data)
{
	memwrite(SP | 0x100,data);
	SP--;
}

//pop data from stack
static u8 pop()
{
	SP++;
	return(memread(SP | 0x100));
}

//check value for n/z and set flags
static void checknz(u8 n)
{
	FLAG_N = (n >> 7) & 1;
	FLAG_Z = (n == 0) ? 1 : 0;
}

static void expand_flags()
{
	FLAG_C = (P & 0x01) >> 0;
	FLAG_Z = (P & 0x02) >> 1;
	FLAG_I = (P & 0x04) >> 2;
	FLAG_D = (P & 0x08) >> 3;
	FLAG_B = (P & 0x10) >> 4;
	FLAG_V = (P & 0x40) >> 6;
	FLAG_N = (P & 0x80) >> 7;
}

static void compact_flags()
{
#ifdef CPU_DEBUG
	if(FLAG_C & 0xFE || 
		FLAG_Z & 0xFE ||
		FLAG_I & 0xFE ||
		FLAG_D & 0xFE ||
		FLAG_B & 0xFE ||
		FLAG_V & 0xFE ||
		FLAG_N & 0xFE) {
		log_printf("compact_flags:  one or more flags is dirty!\n");
	}
#endif
	P = 0x20;
	P |= (FLAG_C) << 0;
	P |= (FLAG_Z) << 1;
	P |= (FLAG_I) << 2;
	P |= (FLAG_D) << 3;
	P |= (FLAG_B) << 4;
	P |= (FLAG_V) << 6;
	P |= (FLAG_N) << 7;
}

static void execute_nmi()
{
	memread(PC);
	memread(PC);
	push((u8)(PC >> 8));
	push((u8)PC);
	compact_flags();
	push(P);
	FLAG_I = 1;
	PC = memread(0xFFFA);
	PC |= memread(0xFFFB) << 8;
}

static void execute_irq()
{
	memread(PC);
	memread(PC);
	push((u8)(PC >> 8));
	push((u8)PC);
	compact_flags();
	push(P);
	FLAG_I = 1;
/*	if(NMISTATE) {
		NMISTATE = 0;
		PC = memread(0xFFFA);
		PC |= memread(0xFFFB) << 8;
	}
	else*/ {
		PC = memread(0xFFFE);
		PC |= memread(0xFFFF) << 8;
	}
}

int cpu_init()
{
	cpu_disassemble_init();
	return(0);
}

void cpu_kill()
{
}

void cpu_reset(int hard)
{
	if(hard) {
		A = X = Y = 0;
		SP = 0xFD;
		P = 0x34;
		expand_flags();
		memset(nes.cpu.ram,0,0x800);
	}
	else {
		FLAG_I = 1;
		SP -= 3;
	}
	PC = memread(0xFFFC);
	PC |= memread(0xFFFD) << 8;

	log_printf("cpu_reset:  vectors:\n");
	log_printf("  nmi:    $%04X\n",memread(0xFFFA) | (memread(0xFFFB) << 8));
	log_printf("  irq:    $%04X\n",memread(0xFFFE) | (memread(0xFFFF) << 8));
	log_printf("  reset:  $%04X\n",memread(0xFFFC) | (memread(0xFFFD) << 8));
}

u64 cpu_getcycles()
{
	return(nes.cpu.cycles);
}

void cpu_set_nmi(u8 state)
{
	NMISTATE = state;
}

void cpu_set_irq(u8 state)
{
	IRQSTATE = state;
}

void cpu_tick()
{
	//acknowledge interrupts
	PREV_NMISTATE = NMISTATE;
	if(FLAG_I == 0)
		PREV_IRQSTATE = IRQSTATE;
	NMISTATE = 0;
	IRQSTATE = 0;

	//increment cycle counter for every memory access
	CYCLES++;

	ppu_step();
	ppu_step();
	ppu_step();

	nes.mapper->cycle();
}

u8 cpu_read(u32 addr)
{
	u32 page = addr >> 12;

	//see if this page is handled by a memory pointer
	if(nes.cpu.readpages[page] != 0) {
		return(nes.cpu.readpages[page][addr & 0xFFF]);
	}

	//see if this page is handled by a read function
	if(nes.cpu.readfuncs[page] != 0) {
		return(nes.cpu.readfuncs[page](addr));
	}

	//check for ram read
	if(addr < 0x2000) {
		return(nes.cpu.ram[addr & 0x7FF]);
	}

	//not handled
	log_printf("cpu_read:  unhandled read at $%04X\n",addr);
	return(0);
}

void cpu_write(u32 addr,u8 data)
{
	u32 page = addr >> 12;

	//see if this page is handled by a memory pointer
	if(nes.cpu.writepages[page] != 0) {
		nes.cpu.writepages[page][addr & 0xFFF] = data;
		return;
	}

	//see if this page is handled by a read function
	if(nes.cpu.writefuncs[page] != 0) {
		nes.cpu.writefuncs[page](addr,data);
		return;
	}

	//ram write
	if(addr < 0x2000) {
		nes.cpu.ram[addr & 0x7FF] = data;
		return;
	}

	//not handled
	log_printf("cpu_write:  unhandled write at $%04X\n",addr,data);
}

u8 cpu_getflags()
{
	compact_flags();
	return(P);
}

static u8 tmp8;
static int tmpi;

#include "addrmodes.c"
#include "opcodes.c"
#include "opcodes_branch.c"
#include "opcodes_loadstore.c"
#include "opcodes_transfer.c"
#include "opcodes_compare.c"
#include "opcodes_incdec.c"
#include "opcodes_alu.c"
#include "opcodes_flag.c"
#include "opcodes_stack.c"
#ifdef CPU_UNDOC
	#include "opcodes_undocumented.c"
#endif
#include "execute.c"
