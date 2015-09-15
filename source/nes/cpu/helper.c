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

static u8 ztable[256] = {
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static INLINE u8 memread(u32 addr)
{
	static int n;

	//handle dpcm cycle stealing
	if(nes->cpu.pcmcycles) {
		n = nes->cpu.pcmcycles - 1;
		nes->cpu.pcmcycles = 0;
		if(addr == 0x4016 || addr == 0x4017) {
			if(n--)
				memread(addr);
			while(n--)
				cpu_tick();
		}
		else {
			while(n--)
				memread(addr);
		}
		apu_dpcm_fetch();
	}

	//increment cycle counter, check irq lines
	cpu_tick();

	//read data from address
	return(cpu_read(addr));
}

static INLINE void memwrite(u32 addr,u8 data)
{
	//handle dpcm cycle stealing
	if(nes->cpu.pcmcycles)
		nes->cpu.pcmcycles--;

	//increment cycle counter, check irq lines
	cpu_tick();

	//write data to its address
	cpu_write(addr,data);
}

//push data to stack
static INLINE void push(u8 data)
{
	memwrite(SP | 0x100,data);
	SP--;
}

//pop data from stack
static INLINE u8 pop()
{
	SP++;
	return(memread(SP | 0x100));
}

//check value for n/z and set flags
static INLINE void checknz(u8 n)
{
	FLAG_N = (n >> 7) & 1;
	FLAG_Z = ztable[n];
}

static INLINE void expand_flags()
{
	FLAG_C = (P & 0x01) >> 0;
	FLAG_Z = (P & 0x02) >> 1;
	FLAG_I = (P & 0x04) >> 2;
	FLAG_D = (P & 0x08) >> 3;
	FLAG_V = (P & 0x40) >> 6;
	FLAG_N = (P & 0x80) >> 7;
}

static INLINE void compact_flags()
{
#ifdef CPU_DEBUG
	if(FLAG_C & 0xFE || 
		FLAG_Z & 0xFE ||
		FLAG_I & 0xFE ||
		FLAG_D & 0xFE ||
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
	P |= (FLAG_V) << 6;
	P |= (FLAG_N) << 7;
}

static INLINE void execute_nmi()
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

static INLINE void execute_irq()
{
	memread(PC);
	memread(PC);
	push((u8)(PC >> 8));
	push((u8)PC);
	compact_flags();
	push(P);
	FLAG_I = 1;
	if(NMISTATE) {
		NMISTATE = 0;
		PC = memread(0xFFFA);
		PC |= memread(0xFFFB) << 8;
	}
	else {
		PC = memread(0xFFFE);
		PC |= memread(0xFFFF) << 8;
	}
}
