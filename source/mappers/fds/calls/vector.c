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
#include <time.h>
#include "mappers/fds/calls.h"
#include "mappers/fds/hle.h"

//nmi vector
HLECALL(nmi)
{
	u8 tmp,action = cpu_read(0x100) >> 6;

//	log_hle("nmi:  action = $%02X\n",action);
	switch(action) {

		//returning from vintwait, return to address that called vintwait
		case 0:

			//disable nmi
			tmp = cpu_read(0xFF) & 0x7F;
			cpu_write(0xFF,tmp);
			cpu_write(0x2000,tmp);
			cpu_read(0x2002);

			//eat nmi return address
			nes->cpu.sp += 3;

			//restore old nmi action
			cpu_write(0x100,cpu_read(++nes->cpu.sp | 0x100));

			//restore a
			nes->cpu.a = cpu_read(++nes->cpu.sp | 0x100);

			//rts
			hlemem[0] = 0x60;
			return;

		//game vectors
		case 1:
		case 2:
		case 3:
			//jmp ($DFF6/8/A)
			hlemem[0] = 0x6C;
			hlemem[1] = 0xF6 + ((action - 1) * 2);
			hlemem[2] = 0xDF;
			break;
	}
}

static void irqdisktransfer()
{
	//eat the irq return address and pushed flags
	nes->cpu.sp += 3;

	//read data from disk into the a register
	nes->cpu.x = cpu_read(0x4031);

	//write data in a reg to disk
	cpu_write(0x4024,nes->cpu.a);

	//txa
	nes->cpu.a = nes->cpu.x;

	//set hle register to rts opcode
	hlemem[8] = 0x60;		//rts
}

//irq vector
HLECALL(irq)
{
	u8 action = cpu_read(0x101);
	u8 tmp = action & 0x3F;

//	log_hle("irq:  action = $%02X\n",action);
	switch(action >> 6) {
		//disk byte skip routine ([$0101] = 00nnnnnn; n is # of bytes to skip)
		//this is mainly used when the CPU has to do some calculations while bytes
		//read off the disk need to be discarded.
		case 0:

			//decrement the number of bytes to skip
			tmp = (action & 0x3F) - 1;

			//if it hasnt expired, throw data away and store new value
			if(tmp != 0xFF) {
				cpu_write(0x101,tmp);
				cpu_read(0x4031);
			}

			//modify return action
			hlemem[8] = 0x40;		//rti
			break;

		case 1:
			irqdisktransfer();
			break;

		//disk IRQ acknowledge routine ([$0101] = 10xxxxxx).
		//don't know what this is used for, or why a delay is put here.
		case 2:
			//ack the interrupt
			cpu_read(0x4030);

			//set hle register to rti opcode
			hlemem[8] = 0x40;		//rti
			break;

		//game irq
		case 3:
			//jmp indirect
			hlemem[0x8] = 0x6C;
			hlemem[0x9] = 0xFE;
			hlemem[0xA] = 0xDF;
			break;
	}
}

/*
($0102):	PC action on reset
[$0101]:	PC action on IRQ. set to $80 on reset
[$0100]:	PC action on NMI. set to $C0 on reset
($DFFE):	disk game IRQ vector    (if [$0101] = 11xxxxxxB)
($DFFC):	disk game reset vector  (if ($0102) = $5335, or $AC35)
($DFFA):	disk game NMI vector #3 (if [$0100] = 11xxxxxxB)
($DFF8):	disk game NMI vector #2 (if [$0100] = 10xxxxxxB)
($DFF6):	disk game NMI vector #1 (if [$0100] = 01xxxxxxB)

[$FF]:  value last written to $2000   $80 on reset.
[$FE]:  value last written to $2001   $06 on reset
[$FD]:  value last written to $2005#1 0'd on reset.
[$FC]:  value last written to $2005#2 0'd on reset.
[$FB]:  value last written to $4016   0'd on reset.
[$FA]:  value last written to $4025   $2E on reset.
[$F9]:  value last written to $4026   $FF on reset.
$F5..$F8 : Used by controller read routines
$00..$0F is used as temporary memory for the BIOS. The main program can use it as temporary memory too.
*/

extern int showdisasm;

//reset vector
HLECALL(reset)
{
	u8 m102,m103;

	srand((u32)time(0));
//	showdisasm = 1;

	//flag setup
	nes->cpu.flags.i = 1;
	nes->cpu.flags.d = 0;

	cpu_write(0x101,0xC0);		//action on irq
	cpu_write(0x100,0xC0);		//action on nmi

	//$2000
	cpu_write(0xFF,0x10);
	cpu_write(0x2000,0x10);

	//$2001
	cpu_write(0xFE,0x06);
	cpu_write(0x2001,0x06);

	cpu_write(0x4022,0x00);		//disable timer irq
	cpu_write(0x4023,0x00);		//disable sound & disk I/O
	cpu_write(0x4023,0x83);		//enable sound & disk I/O

	cpu_write(0xFD,0x00);
	cpu_write(0xFC,0x00);
	cpu_write(0xFB,0x00);
	cpu_write(0x4016,0x00);

	cpu_write(0xFA,0x2E);
	cpu_write(0x4025,0x2E);

	cpu_write(0xF9,0xFF);
	cpu_write(0x4026,0xFF);

	cpu_write(0x4010,0x00);
	cpu_write(0x4015,0x0F);
	cpu_write(0x4080,0x80);
	cpu_write(0x408A,0xE8);
	cpu_write(0x4017,0xC0);

	nes->cpu.sp = 0xFF;
	m102 = cpu_read(0x102);
	m103 = cpu_read(0x103);

/*;if ([$102]=$35)and(([$103]=$53)or([$103]=$AC)) then
;  [$103]:=$53
;  CALL RstPPU05
;  CLI
;  JMP [$DFFC]
$EE84 LDA $0102
$EE87 CMP #$35
$EE89 BNE $EEA2
$EE8B LDA $0103
$EE8E CMP #$53
$EE90 BEQ $EE9B
$EE92 CMP #$ac
$EE94 BNE $EEA2
$EE96 LDA #$53
$EE98 STA $0103
$EE9B JSR RstPPU05
$EE9E CLI; enable interrupts
$EE9F JMP ($DFFC)*/

	memset(nes->ppu.nametables,0,0x1000);
	{//if(m102 == 0x35 && (m103 == 0x53 || m103 == 0xAC)) {
		cpu_write(0x103,0x53);
		hle_setscroll();
		hle_loadbootfiles();
	}
	cpu_write(0x102,0xAC);		//action on reset
	nes->cpu.flags.i = 0;
}
