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
#include "mappers/fds/calls.h"
#include "mappers/fds/hle.h"

HLECALL(counterlogic)
{
	u8 tmp;
	int i;

	log_hle("counterlogic:  a,x,y = $%02X, $%02X, $%02X\n",nes->cpu.a,nes->cpu.x,nes->cpu.y);

	//first counter is decimal counter (9 down to 0, repeat)
	tmp = cpu_read(nes->cpu.x);

	//when this one transitions, decrement counters in upper region
	if(tmp == 0) {

		//reset counter to 9
		cpu_write(nes->cpu.x,9);

		//these stay at 0
		for(i=nes->cpu.a+1;i<=nes->cpu.y;i++) {
			tmp = cpu_read(i);
			if(tmp > 0)
				cpu_write(i,tmp - 1);
		}
	}
	else
		cpu_write(nes->cpu.x,tmp - 1);

	//the rest stay at 0
	for(i=nes->cpu.x+1;i<=nes->cpu.a;i++) {
		tmp = cpu_read(i);
		if(tmp > 0)
			cpu_write(i,tmp - 1);
	}
}

HLECALL(random)
{
	u8 i;

	for(i=0;i<nes->cpu.y;i++) {
		cpu_write(nes->cpu.x + i,(u8)rand());
	}
	log_hle("random:  Y = $%02X\n",nes->cpu.y);
}

//doesnt handle stack wrapping (easy fix!  wtf!)
HLECALL(fetchdirectptr)
{
	u32 tmp,retaddr;

	//current stack address
	tmp = 0x100 + nes->cpu.sp + 2;

	//get the return address off the stack
	retaddr = cpu_read(tmp + 1);
	retaddr |= cpu_read(tmp + 2) << 8;

	//read input parameter, write to memory
	cpu_write(0,cpu_read(retaddr + 1));
	cpu_write(1,cpu_read(retaddr + 2));

	log_hle("direct pointer fetched is $%02X%02X (retaddr = $%04X)\n",cpu_read(1),cpu_read(0),retaddr);

	//offset to new return address
	retaddr += 2;

	//write new return address to the stack
	cpu_write(tmp + 1,(u8)retaddr);
	cpu_write(tmp + 2,(u8)(retaddr >> 8));
}

HLECALL(jumpengine)
{
	u32 addr;

	//read address coming after the opcode calling us
	addr = cpu_read(++nes->cpu.sp | 0x100);
	addr |= cpu_read(++nes->cpu.sp | 0x100) << 8;

	//add one (jsr return address is 1 off)
	addr++;

	//offset into the jump table
	addr += nes->cpu.a * 2;

	//jmp indirect
	hlemem[0x10] = 0x6C;

	//destination address
	hlemem[0x11] = (u8)addr;
	hlemem[0x12] = (u8)(addr >> 8);

	log_hle("jumpengine:  entry = %d, address of table = $%04X (dest = $%02X%02X)\n",nes->cpu.a,addr - nes->cpu.a * 2,cpu_read(addr+1),cpu_read(addr));
}

HLECALL(memfill)
{
	int i;
	u32 start,stop;

	log_hle("memfill:  fill start $%04X, stop $%04X, data $%02X\n",nes->cpu.x << 8,nes->cpu.y << 8,nes->cpu.a);
	start = nes->cpu.x << 8;
	stop = nes->cpu.y << 8;
	while(start <= stop) {
		log_hle("memfill:  filling 256 bytes at $%04X with $%02X\n",start,nes->cpu.a);
		for(i=0;i<256;i++) {
			cpu_write(start + i,nes->cpu.a);
		}
		start += 0x100;
	}
}

HLECALL(pixel2nam)
{
	u8 x,y;
	u32 address;

	x = cpu_read(2) / 8;
	y = cpu_read(3) / 8;
	address = 0x2000 + x + (y * 32);
	cpu_write(0,(u8)(address >> 8));
	cpu_write(1,(u8)address);
	log_hle("pixel2nam:  x,y = %d,%d  address = $%04X\n",cpu_read(2),cpu_read(3),address);
}

HLECALL(nam2pixel)
{
	u8 x,y;
	u32 address;

	address = ((cpu_read(0) << 8) | cpu_read(1)) & 0x3FF;
	x = (address & 0x1F) * 8;
	y = ((address >> 5) & 0x1F) * 8;
	cpu_write(2,x);
	cpu_write(3,y);
	log_hle("nam2pixel:  address = $%04X  x,y = %d,%d\n",(cpu_read(0) << 8) | cpu_read(1),x,y);
}

HLECALL(gethcparam)
{
	u8 writeprotectcheck = nes->cpu.flags.c ^ 1;
	u32 addr1,addr2,tmp;

	log_hle("gethcparam:  writeprotectcheck = %d, a = $%02X\n",writeprotectcheck,nes->cpu.a);
	log_hle("stack:  %02X %02X %02X %02X %02X %02X\n",cpu_read((nes->cpu.sp + 1) | 0x100),cpu_read((nes->cpu.sp + 2) | 0x100),cpu_read((nes->cpu.sp + 3) | 0x100),cpu_read((nes->cpu.sp + 4) | 0x100),cpu_read((nes->cpu.sp + 5) | 0x100),cpu_read((nes->cpu.sp + 6) | 0x100));

	tmp = cpu_read((nes->cpu.sp + 3) | 0x100);
	tmp |= cpu_read((nes->cpu.sp + 4) | 0x100) << 8;

	log_hle("tmp = $%04X\n",tmp);

	addr1 = cpu_read(tmp + 0) << 8;
	addr1 |= cpu_read(tmp + 1);

	addr2 = cpu_read(tmp + 2) << 8;
	addr2 |= cpu_read(tmp + 3);

	log_hle("addr1 = $%04X\n",addr1);
	log_hle("addr2 = $%04X\n",addr2);

	cpu_write(0,cpu_read(tmp + 0));
	cpu_write(1,cpu_read(tmp + 1));
	if(nes->cpu.a == 0xFF) {
		cpu_write(2,cpu_read(tmp + 2));
		cpu_write(3,cpu_read(tmp + 3));
	}
}
