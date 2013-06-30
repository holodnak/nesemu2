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
#include "nes/state/state.h"
#include "misc/log.h"
#include "misc/config.h"

//show disassembly as we execute
#define SHOW_DISASM

//defines to make easier reading
#define PC					nes->cpu.pc
#define A					nes->cpu.a
#define X					nes->cpu.x
#define Y					nes->cpu.y
#define P					nes->cpu.p
#define SP					nes->cpu.sp
#define TMPADDR			nes->cpu.tmpaddr
#define EFFADDR			nes->cpu.effaddr
#define FLAG_C				nes->cpu.flags.c
#define FLAG_Z				nes->cpu.flags.z
#define FLAG_I				nes->cpu.flags.i
#define FLAG_D				nes->cpu.flags.d
#define FLAG_V				nes->cpu.flags.v
#define FLAG_N				nes->cpu.flags.n
#define OPCODE				nes->cpu.opcode
#define OPADDR				nes->cpu.opaddr
#define TMPREG				nes->cpu.tmpreg
#define CYCLES				nes->cpu.cycles
#define NMISTATE			nes->cpu.nmistate
#define IRQSTATE			nes->cpu.irqstate
#define PREV_NMISTATE	nes->cpu.prev_nmistate
#define PREV_IRQSTATE	nes->cpu.prev_irqstate

//cpu memory read/write functions
readfunc_t cpu_read;
writefunc_t cpu_write;

//temp variable for opcode functions
static u8 tmp8;
static int tmpi;

//for stopping execution when invalid opcodes are encountered (kludge)
extern int running;

//include helper functions
#include "helper.c"

//include addressing mode functions
#include "addrmodes.c"

//include opcode functions
#include "opcodes/misc.c"
#include "opcodes/branch.c"
#include "opcodes/loadstore.c"
#include "opcodes/transfer.c"
#include "opcodes/compare.c"
#include "opcodes/incdec.c"
#include "opcodes/alu.c"
#include "opcodes/flag.c"
#include "opcodes/stack.c"
#ifdef CPU_UNDOC
	#include "opcodes/undocumented.c"
#endif

//include functions to execute opcodes
#include "execute.c"

int cpu_init()
{
	cpu_disassemble_init();
	state_register(B_CPU,cpu_state);
	return(0);
}

void cpu_kill()
{
}

void cpu_reset(int hard)
{
	int i;

	for(i=0;i<8;i+=2) {
		nes->cpu.readpages[i + 0] = nes->cpu.writepages[i + 0] = (u8*)nes->cpu.ram;
		nes->cpu.readpages[i + 1] = nes->cpu.writepages[i + 1] = (u8*)nes->cpu.ram + 0x400;
	}

	nes->cpu.pcmcycles = 0;
	if(hard) {
		A = X = Y = 0;
		SP = 0xFD;
		P = 0x24;
		expand_flags();
		memset(nes->cpu.ram,0,0x800);
	}
	else {
		FLAG_I = 1;
		SP -= 3;
	}
	PC = memread(0xFFFC);
	PC |= memread(0xFFFD) << 8;
	log_printf("vectors:\n");
	log_printf("  nmi:    $%04X\n",cpu_read(0xFFFA) | (cpu_read(0xFFFB) << 8));
	log_printf("  irq:    $%04X\n",cpu_read(0xFFFE) | (cpu_read(0xFFFF) << 8));
	log_printf("  reset:  $%04X\n",cpu_read(0xFFFC) | (cpu_read(0xFFFD) << 8));
}

u64 cpu_getcycles()
{
	return(nes->cpu.cycles);
}

void cpu_set_nmi()
{
	NMISTATE = 1;
}

void cpu_clear_nmi()
{
	NMISTATE = 0;
}

void cpu_set_irq(u8 state)
{
	IRQSTATE |= state;
}

void cpu_clear_irq(u8 state)
{
	IRQSTATE &= ~state;
}

void cpu_tick()
{
	//acknowledge interrupts
	PREV_NMISTATE = NMISTATE;
	PREV_IRQSTATE = (FLAG_I == 0) ? IRQSTATE : 0;

	//increment cycle counter for every memory access
	CYCLES++;

	//catch up the ppu
	ppu_step();
	ppu_step();
	ppu_step();

	//catch up the apu
	apu_step();

	//call the mapper callback
	nes->mapper->cpucycle();
}

static u8 read_cpu_memory(u32 addr)
{
	u32 page = addr >> 10;

	//see if this page is handled by a memory pointer
	if(nes->cpu.readpages[page] != 0) {
		return(nes->cpu.readpages[page][addr & 0x3FF]);
	}

	//see if this page is handled by a read function
	if(nes->cpu.readfuncs[page] != 0) {
		return(nes->cpu.readfuncs[page](addr));
	}

	//not handled
	if(config_get_bool("nes.log_unhandled_io"))
		log_printf("cpu_read:  unhandled read at $%04X (page %d)\n",addr,page);
	return(0);
}

static void write_cpu_memory(u32 addr,u8 data)
{
	u32 page = addr >> 10;

	//see if this page is handled by a memory pointer
	if(nes->cpu.writepages[page] != 0) {
		nes->cpu.writepages[page][addr & 0x3FF] = data;
		return;
	}

	//see if this page is handled by a read function
	if(nes->cpu.writefuncs[page] != 0) {
		nes->cpu.writefuncs[page](addr,data);
		return;
	}

	//not handled
	if(config_get_bool("nes.log_unhandled_io"))
		log_printf("cpu_write:  unhandled write at $%04X = $%02X\n",addr,data);
}

u8 cpu_getflags()
{
	compact_flags();
	return(P);
}

readfunc_t cpu_getreadfunc()
{
	return(cpu_read);
}

writefunc_t cpu_getwritefunc()
{
	return(cpu_write);
}

void cpu_setreadfunc(readfunc_t readfunc)
{
	cpu_read = (readfunc == 0) ? read_cpu_memory : readfunc;
}

void cpu_setwritefunc(writefunc_t writefunc)
{
	cpu_write = (writefunc == 0) ? write_cpu_memory : writefunc;
}

void cpu_state(int mode,u8 *data)
{
	STATE_U8(A);
	STATE_U8(X);
	STATE_U8(Y);
	STATE_U8(SP);
	STATE_U8(P);
	STATE_U16(PC);
	STATE_U64(CYCLES);
	STATE_U8(PREV_NMISTATE);
	STATE_U8(PREV_IRQSTATE);
	STATE_ARRAY_U8(nes->cpu.ram,0x800);
}
