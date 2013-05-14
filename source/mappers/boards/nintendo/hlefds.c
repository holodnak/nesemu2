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

#include <string.h>
#include "nes/nes.h"
#include "misc/log.h"

typedef void (*hle_call_t)();

typedef struct fds_disk_header_s {
	u8 blockid;
	u8 ident[14];
	u8 manufacturer_code;
	u8 name[4];
	u8 version;
	u8 diskside;
	u8 disknum;
	u8 extra[2];
	u8 bootid;
	u8 data[31];
	u8 numfiles;
} fds_disk_header_t;

typedef struct fds_file_header_s {
	u8 unk;
	u8 blockid;
	u8 index;
//start of file header on disk
	u8 fileid;
	u8 name[8];
	u16 loadaddr;
	u16 filesize;
	u8 loadarea;
//end of file header on disk
} fds_file_header_t;

typedef struct fds_file_header2_s {
	u8 fileid;
	u8 name[8];
	u16 loadaddr;
	u16 filesize;
	u8 loadarea;
	u16 srcaddr;
	u8 srcarea;
} fds_file_header2_t;

extern int diskside;

//get parameter from the bytes after the jsr
//n = parameter index (0 is the first, 1 is the second, etc)
static u32 getparam(int n)
{
	u32 tmp,retaddr,addr;

	//current stack address
	tmp = 0x100 + nes.cpu.sp;

	//get the return address off the stack
	retaddr = cpu_read(tmp + 1);
	retaddr |= cpu_read(tmp + 2) << 8;

	//offset to required address
	retaddr += n * 2;

	//read input parameter
	addr = cpu_read(retaddr + 1);
	addr |= cpu_read(retaddr + 2) << 8;
	return(addr);
}

//fixup return address on stack
//n = number of parameters call used
static void fixretaddr(int n)
{
	u32 tmp,retaddr;

	//current stack address
	tmp = 0x100 + nes.cpu.sp;

	//get the return address off the stack
	retaddr = cpu_read(tmp + 1);
	retaddr |= cpu_read(tmp + 2) << 8;

	//offset the return address
	retaddr += n * 2;

	//write new return address to the stack
	cpu_write(tmp + 1,(u8)retaddr);
	cpu_write(tmp + 2,(u8)(retaddr >> 8));
}

static void loadfiles()
{
	fds_disk_header_t disk_header;
	fds_file_header_t file_header;
	u8 diskid,loadedfiles = 0;
	u32 addr1,addr2;
	int pos,i,j,k;
	u8 fileidlist[0x20];

	if(diskside == 0xFF) {
		log_printf("loadfiles:  aborting, disk not inserted\n");
		return;
	}

	addr1 = getparam(0);
	addr2 = getparam(1);

	log_printf("loadfiles: addr1,2 = $%04X, $%04X\n",addr1,addr2);

	//get diskid
	diskid = cpu_read(addr1);

	//load file id list
	for(i=0;i<20;i++) {
		fileidlist[i] = cpu_read(addr2 + i);
		log_printf("loadfiles: id list[%d] = $%02X\n",i,fileidlist[i]);
		if(fileidlist[i] == 0xFF)
			break;
	}

	log_printf("loadfiles: disk id required = %d, disknum = %d\n",diskid,diskside);

	//read in disk header
	pos = diskside * 65500;
	memcpy(&disk_header,nes.cart->disk.data + pos,sizeof(fds_disk_header_t));
//	log_message("loading all files with bootid less than %d\n",disk_header.bootid);
	pos += 57;

	//$FF indicates to load system boot files
	if(fileidlist[0] == 0xFF) {

		//leet hax
		log_printf("loadfiles: hack!  fileidlist[0]==$FF!  resetting disknum to 0!!!@!@@121112\n");
		diskside = 0;

		//load boot files
		for(i=0;i<disk_header.numfiles;i++) {
			char fn[9] = "        \0";

			memcpy(&file_header,nes.cart->disk.data + pos,sizeof(fds_file_header_t));
			pos += 17;
			if(file_header.fileid <= disk_header.bootid) {
				memcpy(fn,file_header.name,8);
				log_printf("loadfiles: loading boot file '%s', id %d, size $%X, load addr $%04X\n",fn,file_header.fileid,file_header.filesize,file_header.loadaddr);
				loadedfiles++;

				//load into cpu
				if(file_header.loadarea == 0) {
					for(j=0;j<file_header.filesize;j++)
						cpu_write(j + file_header.loadaddr,nes.cart->disk.data[pos + j + 1]);
				}

				//load into ppu
				else {
					for(j=0;j<file_header.filesize;j++)
						ppu_memwrite(j + file_header.loadaddr,nes.cart->disk.data[pos + j + 1]);
				}
			}
			pos += file_header.filesize;
		}
	}
	else {
		//load files
		for(i=0;i<disk_header.numfiles;i++) {
			char fn[9] = "        \0";

			memcpy(&file_header,nes.cart->disk.data + pos,sizeof(fds_file_header_t));
			pos += 17;                 
			memcpy(fn,file_header.name,8);
			log_printf("loadfiles: checking file '%s', id $%X, size $%X, load addr $%04X\n",fn,file_header.fileid,file_header.filesize,file_header.loadaddr);
			for(k=0;fileidlist[k] != 0xFF && k < 20;k++) {
				if(file_header.fileid == fileidlist[k]) {
					log_printf("loadfiles: loading file '%s', id $%X, size $%X, load addr $%04X\n",fn,file_header.fileid,file_header.filesize,file_header.loadaddr);
					loadedfiles++;       

					//load into cpu
					if(file_header.loadarea == 0) {
						for(j=0;j<file_header.filesize;j++)
							cpu_write(j + file_header.loadaddr,nes.cart->disk.data[pos + j + 1]);
					}

					//load into ppu
					else {
						for(j=0;j<file_header.filesize;j++)
							ppu_memwrite(j + file_header.loadaddr,nes.cart->disk.data[pos + j + 1]);
					}
				}
			}
			pos += file_header.filesize;
		}
	}

	fixretaddr(2);

	log_printf("loadfiles: loaded %d files\n",loadedfiles);

	//put loaded files into y register
	nes.cpu.y = loadedfiles;

	//put error code in accumulator
	nes.cpu.a = 0;
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
static void vramfill()
{
	int i;
	u8 data,count;

	//bios saves these here
	cpu_write(0,nes.cpu.a);
	cpu_write(1,nes.cpu.x);
	cpu_write(2,nes.cpu.y);

	//reset ppu toggle
	cpu_read(0x2002);

	//write ppu control (increment 2007 writes by 1)
	cpu_write(0xFF,cpu_read(0xFF) & 0xFB);

	//write to ppu ctrl reg
	cpu_write(0x2000,cpu_read(0xFF));

	//set vram address
	cpu_write(0x2006,nes.cpu.a);
	cpu_write(0x2006,0);

	//data to fill with
	data = nes.cpu.x;

	//fill vram
	if(nes.cpu.a < 0x20) {
		count = nes.cpu.y + 1;
	}

	//fill nametable
	else {
		count = 4;
	}

	log_printf("vramfill:  fill at $%04X, %d iterations\n",nes.cpu.a << 8,count);

	while(count--) {
		for(i=0;i<256;i++) {
			cpu_write(0x2007,data);
		}
	}

	//see if we fill attributes now
	if(nes.cpu.a >= 0x20) {
		cpu_write(0x2006,nes.cpu.a + 3);
		cpu_write(0x2006,0xC0);
		for(i=0;i<0x40;i++) {
			cpu_write(0x2007,nes.cpu.y);
		}
	}
}

static void spritedma()
{
	log_printf("spritedma:  doing sprite dma\n");
	cpu_write(0x2003,0);		//sprite address
	cpu_write(0x4014,2);		//sprite dma
}

static void vramstructwrite()
{
	u8 data,len,tmp,i;
	u32 srcaddr,destaddr;

	//setup vram increment
	data = cpu_read(0xFF);
	data &= ~4;
	cpu_write(0xFF,data);
	cpu_write(0x2000,data);

	//get parameter following the calling jsr, it is the address of the ppu data string
	srcaddr = getparam(0);

	//fix the return address
	fixretaddr(1);

	log_printf("vramstructwrite: param = $%04X\n",srcaddr);		

	//process!
	for(;;) {

		data = cpu_read(srcaddr++);

		//ternimate!
		if(data >= 0x80) {
			log_printf("vramstructwrite: end opcode @ $%04X = $%02X\n",srcaddr-1,data);		
			break;
		}

		//jsr!
		else if(data == 0x4C) {
			log_printf("vramstructwrite: jsr @ $%04X\n",srcaddr-1);		
		}

		//rts!
		else if(data == 0x60) {
			log_printf("vramstructwrite: rts @ $%04X\n",srcaddr-1);		
		}

		//data!
		else {

			//get destination address
			destaddr = (data << 8) | cpu_read(srcaddr++);

			//read size/flags byte
			data = cpu_read(srcaddr++);
			len = (data & 0x3F) == 0 ? 64 : (data & 0x3F);

			log_printf("vramstructwrite: %s data (addr = $%04X, len = $%02X)\n",(data & 0x40) ? "filling" : "copying",destaddr,len);

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
				cpu_write(0x2007,cpu_read(srcaddr));
				if((data & 0x40) == 0)		//copy?
					srcaddr++;					//if copy, increment address pointer
			}
			if((data & 0x40) != 0)		//fill?
				srcaddr++;					//increment address pointer
		}
	}
}

static void setscroll()
{
	cpu_read(0x2002);
	cpu_write(0x2005,cpu_read(0xFD));
	cpu_write(0x2005,cpu_read(0xFC));
	cpu_write(0x2000,cpu_read(0xFF));
}

static void loadtileset()
{
	u8 tiles = nes.cpu.x;
	u8 flags = nes.cpu.a & 0xF;
	u32 ppuaddr = (nes.cpu.y << 8) | (nes.cpu.a & 0xF0);
	u32 cpuaddr;

	//get parameter following the calling jsr, it is the address of data
	cpuaddr = getparam(0);

	//fix the return address
	fixretaddr(1);

	log_printf("loadtileset:  cpuaddr = $%04X, ppuaddr = $%04X, num tiles = %d, flags = $%X\n",cpuaddr,ppuaddr,tiles,flags);

/*
	u8 units = nes->cpu.x;
	u16 ppuaddr = (nes->cpu.y << 8) | (nes->cpu.a & 0xF0);
	u8 mode = nes->cpu.a;
	u8 fill;

	u32 i,tmp,tmp1,addr;

	//get data pointer addr
	tmp = 0x100 + nes->cpu.s;
	tmp1 = dead6502_read(tmp + 1) << 0;
	tmp1 |= dead6502_read(tmp + 2) << 8;

	log_message("hle cputoppucpy: tmp1 = $%04X\n",tmp1);
	//get data addr
	addr = dead6502_read(tmp1 + 1) << 0;
	addr |= dead6502_read(tmp1 + 2) << 8;

//	addr --; 
	//fixup return address
	tmp1 += 2;
	dead6502_write(tmp + 1,tmp1 & 0xFF);
	dead6502_write(tmp + 2,(tmp1 >> 8) & 0xFF);
	log_message("hle cputoppucpy: cpuaddr = $%04X, units = %d\n",addr,units);

	//read from ppu
	if(mode & 2) {
		log_message("hle cputoppucpy: read from ppu not supported\n");
	}

//	if(mode & 0xC)
//		addr &= ~0xF;
//
//	//write to ppu
//	else {
{		while(units--) {
			log_message("hle cputoppucpy: ppu,cpu addr = $%04X,$%04X, mode = %X\n",ppuaddr,addr,mode & 0xC);
			fill = (mode & 1) ? 0xFF : 0x00;
			switch(mode & 0xC) {
				case 0x0:
					for(i=0;i<16;i++)	ppumem_write(ppuaddr++,dead6502_read(addr++));
					break;
				case 0x4:
					for(i=0;i<8;i++)	ppumem_write(ppuaddr++,dead6502_read(addr++));
					for(i=0;i<8;i++)	ppumem_write(ppuaddr++,fill);
					break;
				case 0x8:
					for(i=0;i<8;i++)	ppumem_write(ppuaddr++,fill);
					for(i=0;i<8;i++)	ppumem_write(ppuaddr++,dead6502_read(addr++));
					break;
				case 0xC:
					for(i=0;i<8;i++,ppuaddr++) {
						u8 data = dead6502_read(addr++);

						ppumem_write(ppuaddr,data ^ fill);
						ppumem_write(ppuaddr+8,data);
					}
					ppuaddr += 8;
					break;
			}
		}
	}
*/
}


static void memfill()
{
	int i;
	u32 start,stop;

	log_printf("memfill:  fill start $%04X, stop $%04X, data $%02X\n",nes.cpu.x << 8,nes.cpu.y << 8,nes.cpu.a);
	start = nes.cpu.x << 8;
	stop = nes.cpu.y << 8;
	while(start <= stop) {
		log_printf("memfill:  filling 256 bytes at $%04X with $%02X\n",nes.cpu.x << 8,nes.cpu.a);
		for(i=0;i<256;i++) {
			cpu_write(start + i,nes.cpu.a);
		}
		start += 0x100;
	}
	
}

static void counterlogic()
{
	log_printf("counterlogic:  not implemented\n");
}

static void random()
{
	cpu_write(0,(u8)rand());
}

static void readpads()
{
	int i;
	u8 data,tmp1,tmp2,tmp3,tmp4;

	data = cpu_read(0xFB);
	cpu_write(0x4016,data + 1);
	cpu_write(0x4016,data);
	for(tmp1=tmp2=tmp3=tmp4=0,i=0;i<8;i++) {
		tmp1 <<= 1;
		tmp2 <<= 1;
		tmp3 <<= 1;
		tmp4 <<= 1;
		data = cpu_read(0x4016);
		tmp1 |= data & 1;
		tmp2 |= (data >> 1) & 1;
		data = cpu_read(0x4017);
		tmp3 |= data & 1;
		tmp4 |= (data >> 1) & 1;
	}
	cpu_write(0xF5,tmp1);
	cpu_write(0x00,tmp2);
	cpu_write(0xF6,tmp3);
	cpu_write(0x01,tmp4);
	nes.cpu.flags.z = 1;
	nes.cpu.flags.n = 0;
}

static void orpads()
{
	cpu_write(0xF5,cpu_read(0x00) | cpu_read(0xF5));
	cpu_write(0xF6,cpu_read(0x01) | cpu_read(0xF6));
}

static void downpads()
{
	int i;

	for(i = 1; i >= 0; i--) {
		nes.cpu.a = cpu_read(0xF5 + i);
		nes.cpu.y = nes.cpu.a;
		nes.cpu.a ^= cpu_read(0xF7 + i);
		nes.cpu.a &= cpu_read(0xF5 + i);
		cpu_write(0xF5 + i,nes.cpu.a);
		cpu_write(0xF7 + i,nes.cpu.y);
	}
}

static void readordownpads()
{

}

static void readdownverifypads()
{

}

static void readordownverifypads()
{

}

static void readdownexppads()
{

}

//nmi vector
static void nmi()
{

}

//irq vector
static void irq()
{

}

//reset vector
static void reset()
{

}

//insert disk
static void forceinsert()
{
	log_printf("forceinsert:  disk inserted, side = 0\n");
	diskside = 0;
}


static hle_call_t hle_calls[64] = {

	//$0 - disk related calls
	loadfiles,
	0,
	0,
	0,
	0,
	0,
	0,
	0,

	//$8 - ???
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,

	//$10 - pad calls
	readpads,
	orpads,
	downpads,
	0,
	readdownverifypads,
	readordownverifypads,
	readdownexppads,
	0,

	//$18 - ppu related calls
	vramfill,
	vramstructwrite,
	spritedma,
	setscroll,
	loadtileset,
	0,
	0,
	0,

	//$20 - cpu calls
	memfill,
	0,
	0,
	0,
	0,
	0,
	0,
	0,

	//$28 - ???
	counterlogic,
	random,
	0,
	0,
	0,
	0,
	0,
	0,

	//$30 - ???
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,

	//$38 - vectors
	nmi,
	irq,
	reset,
	0,

	//$3C - special calls for hle bios use only
	forceinsert,
	0,
	0,
	0,
};

void hlefds_write(u32 addr,u8 data)
{
	u8 index = data & 0x3F;
	hle_call_t func = hle_calls[index];

	if(addr != 0x4222) {
		return;
	}

//	log_printf("hlefds_write:  register write!  $%02X (function = $%02X)\n",data,index);
	if(func)
		func();
	else
		log_printf("hlefds_write:  hle call $%02X not implemented yet\n",index);
}
