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

#include "mappers/fds/calls.h"
#include "mappers/fds/hle.h"

static void writeppu(u32 addr,u8 data)
{
	if(addr >= 0x3F00)
		ppu_pal_write(addr & 0x1F,data);
	else
		ppu_memwrite(addr,data);
}

/*
vram fill
---------
A is HI VRAM addr (LO VRAM addr is always 0)
X is fill value
Y is iteration count (256 written bytes per iteration). if A is $20 or
  greater (indicating name table VRAM), iteration count is always 4,
  and this data is used for attribute fill data.
*/
HLECALL(vramfill)
{
	int i;
	u8 data,count;

	//bios saves these here
	cpu_write(0,nes->cpu.a);
	cpu_write(1,nes->cpu.x);
	cpu_write(2,nes->cpu.y);

	//reset ppu toggle
	cpu_read(0x2002);

	//write ppu control (increment 2007 writes by 1)
	cpu_write(0xFF,cpu_read(0xFF) & 0xFB);

	//write to ppu ctrl reg
	cpu_write(0x2000,cpu_read(0xFF));

	//set vram address
	cpu_write(0x2006,nes->cpu.a);
	cpu_write(0x2006,0);

	//data to fill with
	data = nes->cpu.x;

	//fill vram
	if(nes->cpu.a < 0x20) {
		count = nes->cpu.y + 1;
	}

	//fill nametable
	else {
		count = 4;
	}

	log_hle("vramfill:  fill at $%04X, %d iterations\n",nes->cpu.a << 8,count);

	while(count--) {
		for(i=0;i<256;i++) {
			writeppu(SCROLL++,data);
			SCROLL &= 0x3FFF;
		}
	}

	//see if we fill attributes now
	if(nes->cpu.a >= 0x20) {
		cpu_write(0x2006,nes->cpu.a + 3);
		cpu_write(0x2006,0xC0);
		for(i=0;i<0x40;i++) {
			writeppu(SCROLL++,nes->cpu.y);
			SCROLL &= 0x3FFF;
		}
	}
}

HLECALL(vramstructwrite)
{
	u8 data,len,tmp,i;
	u32 srcaddr,destaddr,tmp32;

	//setup vram increment to 1
	data = cpu_read(0xFF);
	data &= ~4;
	cpu_write(0xFF,data);
	cpu_write(0x2000,data);

	//get parameter following the calling jsr, it is the address of the ppu data string
	srcaddr = hle_getparam(0);

	//fix the return address
	hle_fixretaddr(1);

	log_hle("vramstructwrite: param = $%04X\n",srcaddr);

	//process!
	for(;;) {

		data = cpu_read(srcaddr++);

		//ternimate!
		if(data >= 0x80) {
			log_hle("vramstructwrite: end opcode @ $%04X = $%02X\n",srcaddr-1,data);
			break;
		}

		//jsr!
		else if(data == 0x4C) {
			//save old address
			tmp32 = srcaddr;

			//get new address
			srcaddr = cpu_read(tmp32 + 0);
			srcaddr |= cpu_read(tmp32 + 1) << 8;
			tmp32 += 2;

			//write return address to stack
			cpu_write(nes->cpu.sp-- | 0x100,(u8)(tmp32 >> 8));
			cpu_write(nes->cpu.sp-- | 0x100,(u8)tmp32);
			log_hle("vramstructwrite:  jsr:  old addr, new addr = $%04X, $%04X\n",tmp32-3,srcaddr);
		}

		//rts!
		else if(data == 0x60) {
			log_hle("vramstructwrite:  rts @ $%04X\n",srcaddr-1);
			//read return address from the stack
			srcaddr = cpu_read(++nes->cpu.sp | 0x100);
			srcaddr |= cpu_read(++nes->cpu.sp | 0x100) << 8;
		}

		//data!
		else {

			//get destination address
			destaddr = (data << 8) | cpu_read(srcaddr++);

			//read size/flags byte
			data = cpu_read(srcaddr++);
			len = (data & 0x3F) == 0 ? 64 : (data & 0x3F);

			log_hle("vramstructwrite: %s data (addr = $%04X, len = $%02X)\n",(data & 0x40) ? "filling" : "copying",destaddr,len);

			//setup increment
			tmp = cpu_read(0xFF) | 4;
			if((data & 0x80) == 0) {
				tmp &= ~4;
			}
			cpu_write(0xFF,tmp);
			cpu_write(0x2000,tmp);	

			//reset the toggle and write the destination address
			cpu_read(0x2002);
			cpu_write(0x2006,(u8)(destaddr >> 8));
			cpu_write(0x2006,(u8)destaddr);

			//fill/copy data
			for(i=0;i<len;i++) {
//				cpu_write(0x2007,cpu_read(srcaddr));
				writeppu(SCROLL,cpu_read(srcaddr));
				if((data & 0x80) == 0)
					SCROLL++;
				else
					SCROLL += 32;
				if((data & 0x40) == 0)		//copy?
					srcaddr++;					//if copy, increment address pointer
			}
			if((data & 0x40) != 0)		//fill?
				srcaddr++;					//increment address pointer
		}
	}
}

HLECALL(writevrambuffer)
{
	u32 srcaddr;
	u8 data,i;

	//increment vram address by 1
	data = cpu_read(0xFF);
	data &= ~4;
	cpu_write(0xFF,data);
	cpu_write(0x2000,data);

	//reset ppu toggle
	cpu_read(0x2002);

	//setup source data address
	srcaddr = 0x302;

	log_hle("writevrambuffer:  len = %d\n",cpu_read(0x301));

	//process!
	for(;;) {

		data = cpu_read(srcaddr++);

		if(data >= 0x80) {
			break;
		}

		//write dest address
		cpu_write(0x2006,data);
		cpu_write(0x2006,cpu_read(srcaddr++));

		//get length
		data = cpu_read(srcaddr++);

		//write data
		for(i=0;i<data;i++) {
//			cpu_write(0x2007,cpu_read(srcaddr++));
			writeppu(SCROLL++,cpu_read(srcaddr++));
			SCROLL &= 0x3FFF;
		}

	}

	//set buffer to unused for next time
	cpu_write(0x302,data);
	cpu_write(0x301,0);
}

HLECALL(readvrambuffer)
{
	log_printf("readvrambuffer not implemented\n");
}

//appends data to end of the buffer
HLECALL(preparevramstring)
{
	u8 i,len = nes->cpu.y;
	u8 bufsize,buflen,bufpos;
	u32 srcaddr;

	srcaddr = hle_getparam(0);
	hle_fixretaddr(1);

	log_hle("preparevramstring:  destaddr = $%04X, srcaddr = $%04X, len = %d\n",nes->cpu.x | (nes->cpu.a << 8),srcaddr,len);

	//get max buffer size
	bufsize = cpu_read(0x300);

	//get size of data already in the buffer
	buflen = cpu_read(0x301);

	log_hle("  -- bufsize = $%02X, buflen = $%02X\n",bufsize,buflen);

	//see if data is too big
	if((int)(buflen + len + 3) > (int)bufsize) {
		nes->cpu.a = 1;
		return;
	}

	//get position to begin writing in
	bufpos = buflen;

	//new size of the buffer
	buflen += len + 3;

	//bios doesnt modify this byte?
//	cpu_write(0x300,len);
	cpu_write(0x301,buflen);
	cpu_write(0x302 + bufpos,nes->cpu.a);
	cpu_write(0x303 + bufpos,nes->cpu.x);
	cpu_write(0x304 + bufpos,nes->cpu.y);
	for(i=0;i<len;i++) {
		cpu_write(0x305 + bufpos + i,cpu_read(srcaddr++));
	}
	cpu_write(0x305 + bufpos + i,0xFF);

	nes->cpu.a = 0xFF;
}

HLECALL(preparevramstrings)
{
	log_printf("preparevramstrings not implemented\n");
}

HLECALL(getvrambufferbyte)
{
	log_printf("preparevramstrings not implemented\n");
}

HLECALL(loadtileset)
{
	int units = nes->cpu.x;
	u16 ppuaddr = (nes->cpu.y << 8) | (nes->cpu.a & 0xF0);
	u8 fill,mode = nes->cpu.a;
	u32 i,addr;

	addr = hle_getparam(0);
	hle_fixretaddr(1);

	log_hle("loadtileset: ppuaddr = $%04X, cpuaddr = $%04X, units = %d, mode = %d\n",ppuaddr,addr,units,(mode & 0xC) >> 2);

//	if(mode & 0xC)
//		addr &= ~0xF;

	//read from ppu
	if(mode & 2) {
		while(units--) {
			log_hle("loadtileset: read:  ppu,cpu addr = $%04X,$%04X, mode = %X\n",ppuaddr,addr,mode & 0xC);
//			fill = (mode & 1) ? 0xFF : 0x00;
			switch(mode & 0xC) {
				case 0x0:
					for(i=0;i<16;i++)	cpu_write(addr++,ppu_memread(ppuaddr++));
					break;
				case 0x4:
					for(i=0;i<8;i++)	cpu_write(addr++,ppu_memread(ppuaddr++));
					for(i=0;i<8;i++)	ppu_memread(ppuaddr++);
					break;
				case 0x8:
					for(i=0;i<8;i++)	ppu_memread(ppuaddr++);
					for(i=0;i<8;i++)	cpu_write(addr++,ppu_memread(ppuaddr++));
					break;
				case 0xC:
					log_hle("loadtileset: reading and mode 3 not supported\n");
					ppuaddr += 8;
					break;
			}
		}
	}

	//write to ppu
	else {
		while(units--) {
			log_hle("loadtileset: write:  ppu,cpu addr = $%04X,$%04X, mode = %X\n",ppuaddr,addr,mode & 0xC);
			fill = (mode & 1) ? 0xFF : 0x00;
			switch(mode & 0xC) {
				case 0x0:
					for(i=0;i<16;i++)	ppu_memwrite(ppuaddr++,cpu_read(addr++));
					break;
				case 0x4:
					for(i=0;i<8;i++)	ppu_memwrite(ppuaddr++,cpu_read(addr++));
					for(i=0;i<8;i++)	ppu_memwrite(ppuaddr++,fill);
					break;
				case 0x8:
					for(i=0;i<8;i++)	ppu_memwrite(ppuaddr++,fill);
					for(i=0;i<8;i++)	ppu_memwrite(ppuaddr++,cpu_read(addr++));
					break;
				case 0xC:
					for(i=0;i<8;i++,ppuaddr++) {
						u8 data = cpu_read(addr++);

						ppu_memwrite(ppuaddr,data ^ fill);
						ppu_memwrite(ppuaddr+8,data);
					}
					ppuaddr += 8;
					break;
			}
		}
	}

}
/*
static void inc00bya()
{
	u16 tmp;

	tmp = cpu_read(0) | (cpu_read(1) << 8);
	tmp += nes->cpu.a;
	cpu_write(0,(u8)tmp);
	cpu_write(1,(u8)(tmp >> 8));
	cpu_write(2,cpu_read(2) - 1);
}

static void inc00by8()
{
	nes->cpu.a = 8;
	inc00bya();
}
*/
