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
#include <time.h>
#include "mappers/fds/fds.h"
#include "nes/nes.h"
#include "misc/log.h"

#define log_hle if(hlelog) log_printf

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

static u8 hleregs[4];
static int hlelog = 0;

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

static void loadbootfiles()
{
	fds_disk_header_t disk_header;
	fds_file_header_t file_header;
	int i,j,pos;
	u8 loadedfiles = 0;

	//change reset action
	cpu_write(0x103,0x53);

	//leet hax
	log_hle("loadbootfiles:  setting diskside to 0\n");
	diskside = 0;
	pos = 0;

	//load disk header
	pos = diskside * 65500;
	memcpy(&disk_header,nes.cart->disk.data + pos,sizeof(fds_disk_header_t));
	pos += 57;

	//load boot files
	for(i=0;i<disk_header.numfiles;i++) {
		char fn[10] = "        \0";

		memcpy(&file_header,nes.cart->disk.data + pos,sizeof(fds_file_header_t));
		pos += 17;
		if(file_header.fileid <= disk_header.bootid) {
			memcpy(fn,file_header.name,8);
			log_hle("loadfiles: loading boot file '%s', id %d, size $%X, load addr $%04X\n",fn,file_header.fileid,file_header.filesize,file_header.loadaddr);
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

	//put loaded files into y register
	nes.cpu.y = loadedfiles;

	//put error code in accumulator
	nes.cpu.a = 0;
}

static void loadfiles()
{
	fds_disk_header_t disk_header;
	fds_file_header_t file_header;
	u8 diskid,loadedfiles = 0;
	u32 addr1,addr2;
	int pos,i,j,k;
	u8 fileidlist[0x20];

	//bios needs these flags set
	nes.cpu.flags.z = 1;
	nes.cpu.flags.n = 0;
	nes.cpu.flags.v = 0;
	nes.cpu.flags.c = 0;
	nes.cpu.a = 0;
	nes.cpu.y = 0;

	if(diskside == 0xFF) {
		log_hle("loadfiles:  hax:  disk not inserted, setting to 0\n");
		diskside = 0;
//		log_hle("loadfiles:  aborting, disk not inserted\n");
//		return;
	}

	addr1 = getparam(0);
	addr2 = getparam(1);
	fixretaddr(2);

	log_hle("loadfiles: addr1,2 = $%04X, $%04X\n",addr1,addr2);

	//get diskid
	diskid = cpu_read(addr1);

	//load file id list
	for(i=0;i<20;i++) {
		fileidlist[i] = cpu_read(addr2 + i);
		log_hle("loadfiles: id list[%d] = $%02X\n",i,fileidlist[i]);
		if(fileidlist[i] == 0xFF)
			break;
	}

	log_hle("loadfiles: disk id required = %d, disknum = %d\n",diskid,diskside);

	//read in disk header
	pos = diskside * 65500;
	memcpy(&disk_header,nes.cart->disk.data + pos,sizeof(fds_disk_header_t));
	pos += 57;

	//$FF indicates to load system boot files
	if(fileidlist[0] == 0xFF) {
		loadbootfiles();
		return;
	}

	//load files
	for(i=0;i<disk_header.numfiles;i++) {
		char fn[10] = "        \0";

		memcpy(&file_header,nes.cart->disk.data + pos,sizeof(fds_file_header_t));
		pos += 17;                 
		memcpy(fn,file_header.name,8);
		log_hle("loadfiles: checking file '%s', id $%X, size $%X, load addr $%04X\n",fn,file_header.fileid,file_header.filesize,file_header.loadaddr);
		for(k=0;fileidlist[k] != 0xFF && k < 20;k++) {
			if(file_header.fileid == 0xE0) {
				log_hle("loadfiles: file '%s', id $%X, size $%X, load addr $%04X (data @ %d, side %d)\n",fn,file_header.fileid,file_header.filesize,file_header.loadaddr,pos % 65500,diskside);
			}
			if(file_header.fileid == fileidlist[k]) {
				log_hle("loadfiles: loading file '%s', id $%X, size $%X, load addr $%04X\n",fn,file_header.fileid,file_header.filesize,file_header.loadaddr);
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

	log_hle("loadfiles: loaded %d files\n",loadedfiles);

	//put loaded files into y register
	nes.cpu.y = loadedfiles;

	//put error code in accumulator
	nes.cpu.a = 0;
}

static void writefile()
{
	fds_disk_header_t *disk_header;
	fds_file_header_t *file_header;
	fds_file_header2_t file_header2;
	u32 addr1,addr2,pos;
	char fn[10] = "        \0";
	int i;

	addr1 = getparam(0);
	addr2 = getparam(1);
	fixretaddr(2);

	log_hle("hle writefile: addr1 = %04X, addr2 = %04X\n",addr1,addr2);

	//position of disk data start
	pos = diskside * 65500;

	//pointer to disk header
	disk_header = (fds_disk_header_t*)(nes.cart->disk.data + pos);

	log_hle("hle writefile: %d files on disk (A = %d)\n",disk_header->numfiles,nes.cpu.a);

	//seek past the disk header
	pos += 57;

	log_hle("file header: ");
	for(i=0;i<17;i++) {
		log_hle(" $%02X",cpu_read(addr2 + i));
	}
	log_hle("\n");

	//fill the file header2 struct
	file_header2.fileid = cpu_read(addr2 + 0);
	for(i=0;i<8;i++)
		file_header2.name[i] = cpu_read(addr2 + 1);
	file_header2.loadaddr = cpu_read(addr2 + 9) | (cpu_read(addr2 + 10) << 8);
	file_header2.filesize = cpu_read(addr2 + 11) | (cpu_read(addr2 + 12) << 8);
	file_header2.loadarea = cpu_read(addr2 + 13);
	file_header2.srcaddr = cpu_read(addr2 + 14) | (cpu_read(addr2 + 15) << 8);
	file_header2.srcarea = cpu_read(addr2 + 16);

	//show some debug info
	memcpy(fn,&file_header2.name,8);
	log_hle("writing file, header:\n");
	log_hle("  fileid: %02X\n",file_header2.fileid);
	log_hle("  name:  '%8s'\n",fn);
	log_hle("  loadaddr: %04X\n",file_header2.loadaddr);
	log_hle("  filesize: %04X\n",file_header2.filesize);
	log_hle("  loadarea: %02X\n",file_header2.loadarea);
	log_hle("  srcaddr: %04X\n",file_header2.srcaddr);
	log_hle("  srcarea: %02X\n",file_header2.srcarea);

	//loop thru all the files
	for(i=0;i<disk_header->numfiles;i++) {
		file_header = (fds_file_header_t*)(nes.cart->disk.data + pos);
		pos += 17;
		if(file_header->fileid == file_header2.fileid) {
			log_hle("hle writefile: match! '%s', id $%X, size $%X, load addr $%04X\n",fn,file_header->fileid,file_header->filesize,file_header->loadaddr);
			log_hle("hle writefile: fileid match, writing at %d bytes @ %d (side %d)\n",file_header2.filesize,pos % 65500,diskside);

			memcpy(file_header->name,file_header2.name,8);
			file_header->loadaddr = file_header2.loadaddr;
			file_header->filesize = file_header2.filesize;
			file_header->loadarea = file_header2.loadarea;
			for(i=0;i<file_header2.filesize;i++) {
				if(file_header2.srcarea == 0)
					nes.cart->disk.data[pos + i + 1] = cpu_read(file_header2.srcaddr + i);
				else
					nes.cart->disk.data[pos + i + 1] = ppu_memread(file_header2.srcaddr + i);
			}
			nes.cart->disk.data[pos + i + 1] = 0xFF;
			break;
		}
		pos += file_header->filesize;
		memcpy(fn,file_header->name,8);
		log_hle("hle writefile: seeking over '%s', id $%X, size $%X, load addr $%04X\n",fn,file_header->fileid,file_header->filesize,file_header->loadaddr);
	}

	if(nes.cpu.a != 0xFF)
		disk_header->numfiles = nes.cpu.a + 1;
	else
		log_printf("appendfile!\n");

	cpu_write(6,disk_header->numfiles);

	nes.cpu.a = 0;
	nes.cpu.x = 0;
	nes.cpu.flags.n = 0;
	nes.cpu.flags.z = 1;
}

static void appendfile()
{
	log_printf("appendfile!\n");
	nes.cpu.a = 0xFF;
	writefile();
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

	log_hle("vramfill:  fill at $%04X, %d iterations\n",nes.cpu.a << 8,count);

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
	cpu_write(0x2003,0);		//sprite address
	cpu_write(0x4014,2);		//sprite dma
}

static void vramstructwrite()
{
	u8 data,len,tmp,i;
	u32 srcaddr,destaddr,tmp32;

	//setup vram increment
	data = cpu_read(0xFF);
	data &= ~4;
	cpu_write(0xFF,data);
	cpu_write(0x2000,data);

	//get parameter following the calling jsr, it is the address of the ppu data string
	srcaddr = getparam(0);

	//fix the return address
	fixretaddr(1);

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
			cpu_write(nes.cpu.sp-- | 0x100,(u8)(tmp32 >> 8));
			cpu_write(nes.cpu.sp-- | 0x100,(u8)tmp32);
			log_hle("vramstructwrite:  jsr:  old addr, new addr = $%04X, $%04X\n",tmp32-3,srcaddr);
		}

		//rts!
		else if(data == 0x60) {
			log_hle("vramstructwrite:  rts @ $%04X\n",srcaddr-1);
			//read return address from the stack
			srcaddr = cpu_read(++nes.cpu.sp | 0x100);
			srcaddr |= cpu_read(++nes.cpu.sp | 0x100) << 8;
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
				cpu_write(0x2007,cpu_read(srcaddr));
				if((data & 0x40) == 0)		//copy?
					srcaddr++;					//if copy, increment address pointer
			}
			if((data & 0x40) != 0)		//fill?
				srcaddr++;					//increment address pointer
		}
	}
}

static void writevrambuffer()
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
			cpu_write(0x2007,cpu_read(srcaddr++));
		}

	}

	//set buffer to unused for next time
	cpu_write(0x302,data);
	cpu_write(0x301,0);
}

static void readvrambuffer()
{

}

//appends data to end of the buffer
static void preparevramstring()
{
	u8 i,len = nes.cpu.y;
	u8 bufsize,buflen,bufpos;
	u32 srcaddr;

	srcaddr = getparam(0);
	fixretaddr(1);

	log_hle("preparevramstring:  destaddr = $%04X, srcaddr = $%04X, len = %d\n",nes.cpu.x | (nes.cpu.a << 8),srcaddr,len);

	//get max buffer size
	bufsize = cpu_read(0x300);

	//get size of data already in the buffer
	buflen = cpu_read(0x301);

	log_hle("  -- bufsize = $%02X, buflen = $%02X\n",bufsize,buflen);

	//see if data is too big
	if((int)(buflen + len + 3) > (int)bufsize) {
		nes.cpu.a = 1;
		return;
	}

	//get position to begin writing in
	bufpos = buflen;

	//new size of the buffer
	buflen += len + 3;

	//bios doesnt modify this byte?
//	cpu_write(0x300,len);
	cpu_write(0x301,buflen);
	cpu_write(0x302 + bufpos,nes.cpu.a);
	cpu_write(0x303 + bufpos,nes.cpu.x);
	cpu_write(0x304 + bufpos,nes.cpu.y);
	for(i=0;i<len;i++) {
		cpu_write(0x305 + bufpos + i,cpu_read(srcaddr++));
	}
	cpu_write(0x305 + bufpos + i,0xFF);

	nes.cpu.a = 0xFF;
}

static void preparevramstrings()
{

}

static void getvrambufferbyte()
{

}

static void setscroll()
{
	cpu_read(0x2002);
	cpu_write(0x2005,cpu_read(0xFD));
	cpu_write(0x2005,cpu_read(0xFC));
	cpu_write(0x2000,cpu_read(0xFF));
}

static void vintwait()
{
	u8 tmp;

	//save a
	cpu_write(nes.cpu.sp-- | 0x100,nes.cpu.a);

	//save old nmi action
	cpu_write(nes.cpu.sp-- | 0x100,cpu_read(0x100));

	//write new nmi action
	cpu_write(0x100,0);

	//enable nmi
	tmp = cpu_read(0xFF) | 0x80;
	cpu_write(0xFF,tmp);
	cpu_write(0x2000,tmp);

	//clear zero flag, set negative flag
	nes.cpu.flags.z = 0;
	nes.cpu.flags.n = 1;

	log_hle("vintwait!\n");
}

//todo:  revise this to use $2007 and its internal functions
static void loadtileset()
{
	int units = nes.cpu.x;
	u16 ppuaddr = (nes.cpu.y << 8) | (nes.cpu.a & 0xF0);
	u8 fill,mode = nes.cpu.a;
	u32 i,addr;

	addr = getparam(0);
	fixretaddr(1);

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

static void inc00bya()
{
	u16 tmp;

	tmp = cpu_read(0) | (cpu_read(1) << 8);
	tmp += nes.cpu.a;
	cpu_write(0,(u8)tmp);
	cpu_write(1,(u8)(tmp >> 8));
	cpu_write(2,cpu_read(2) - 1);
}

static void inc00by8()
{
	nes.cpu.a = 8;
	inc00bya();
}

static void memfill()
{
	int i;
	u32 start,stop;

	log_hle("memfill:  fill start $%04X, stop $%04X, data $%02X\n",nes.cpu.x << 8,nes.cpu.y << 8,nes.cpu.a);
	start = nes.cpu.x << 8;
	stop = nes.cpu.y << 8;
	while(start <= stop) {
		log_hle("memfill:  filling 256 bytes at $%04X with $%02X\n",nes.cpu.x << 8,nes.cpu.a);
		for(i=0;i<256;i++) {
			cpu_write(start + i,nes.cpu.a);
		}
		start += 0x100;
	}
}

static void counterlogic()
{
	u8 tmp;
	int i;

	log_hle("counterlogic:  a,x,y = $%02X, $%02X, $%02X\n",nes.cpu.a,nes.cpu.x,nes.cpu.y);

	//first counter is decimal counter (9 down to 0, repeat)
	tmp = cpu_read(nes.cpu.x);

	//when this one transitions, decrement counters in upper region
	if(tmp == 0) {

		//reset counter to 9
		cpu_write(nes.cpu.x,9);

		//these stay at 0
		for(i=nes.cpu.a+1;i<=nes.cpu.y;i++) {
			tmp = cpu_read(i);
			if(tmp > 0)
				cpu_write(i,tmp - 1);
		}
	}
	else
		cpu_write(nes.cpu.x,tmp - 1);

	//the rest stay at 0
	for(i=nes.cpu.x+1;i<=nes.cpu.a;i++) {
		tmp = cpu_read(i);
		if(tmp > 0)
			cpu_write(i,tmp - 1);
	}
}

static void random()
{
	u8 i;

	for(i=0;i<nes.cpu.y;i++) {
		cpu_write(nes.cpu.x + i,(u8)rand());
	}
	log_hle("random:  Y = $%02X\n",nes.cpu.y);
}

//doesnt handle stack wrapping (easy fix!  wtf!)
static void fetchdirectptr()
{
	u32 tmp,retaddr;

	//current stack address
	tmp = 0x100 + nes.cpu.sp + 2;

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

static void jumpengine()
{

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
	readpads();
	orpads();
	downpads();
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
	u8 tmp,action = cpu_read(0x100);

	switch(action >> 6) {

		//returning from vintwait, return to address that called vintwait
		case 0:

			//disable nmi
			tmp = cpu_read(0xFF) & 0x7F;
			cpu_write(0xFF,tmp);
			cpu_write(0x2000,tmp);
			cpu_read(0x2002);

			//eat nmi return address
			nes.cpu.sp += 3;

			//restore old nmi action
			cpu_write(0x100,cpu_read(++nes.cpu.sp | 0x100));

			//restore a
			nes.cpu.a = cpu_read(++nes.cpu.sp | 0x100);
			return;

		//game vectors
		case 1:
			nes.cpu.pc = cpu_read(0xDFF6) | (cpu_read(0xDFF7) << 8);
			break;
		case 2:
			nes.cpu.pc = cpu_read(0xDFF8) | (cpu_read(0xDFF9) << 8);
			break;
		case 3:
			nes.cpu.pc = cpu_read(0xDFFA) | (cpu_read(0xDFFB) << 8);
			break;
	}
}

static void irqdisktransfer()
{
	//eat the irq return address and pushed flags
	nes.cpu.sp += 3;

	//read data from disk into the a register
	nes.cpu.x = cpu_read(0x4031);

	//write data in a reg to disk
	cpu_write(0x4024,nes.cpu.a);

	//txa
	nes.cpu.a = nes.cpu.x;
}

static void irqdiskbyteskip()
{
	u8 tmp = cpu_read(0x101) & 0x3F;

	tmp--;
	if(tmp != 0xFF) {
		cpu_write(0x101,tmp);
		cpu_read(0x4031);
	}
}

static void irqdiskack()
{
	cpu_read(0x4030);
}

extern int showdisasm;

//irq vector
static void irq()
{
	u8 action = cpu_read(0x101);
	u8 tmp = action & 0x3F;

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

			//set hle register to rti opcode
			hleregs[0] = 0x40;		//rti
			nes.cpu.flags.z = 1;
			nes.cpu.flags.n = 0;
			nes.cpu.flags.v = 0;
			break;

		case 1:
			//eat the irq return address and pushed flags
			nes.cpu.sp += 3;

			//read data from disk into the a register
			nes.cpu.x = cpu_read(0x4031);

			//write data in a reg to disk
			cpu_write(0x4024,nes.cpu.a);

			//txa
			nes.cpu.a = nes.cpu.x;

			//set hle register to rts opcode
			hleregs[0] = 0x60;		//rts
			nes.cpu.flags.z = 0;
			nes.cpu.flags.n = 0;
			nes.cpu.flags.v = 1;
			break;

		//disk IRQ acknowledge routine ([$0101] = 10xxxxxx).
		//don't know what this is used for, or why a delay is put here.
		case 2:
			//ack the interrupt
			cpu_read(0x4030);

			//set hle register to rti opcode
			hleregs[0] = 0x40;		//rti
			nes.cpu.flags.z = 0;
			nes.cpu.flags.n = 1;
			nes.cpu.flags.v = 0;
			break;

		//game irq
		case 3:
			nes.cpu.pc = cpu_read(0xDFFE) | (cpu_read(0xDFFF) << 8);
			nes.cpu.flags.z = 0;
			nes.cpu.flags.n = 1;
			nes.cpu.flags.v = 1;
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
//reset vector
static void reset()
{
	u8 m102,m103;

	srand((u32)time(0));
//	showdisasm = 1;

	//flag setup
	nes.cpu.flags.i = 1;
	nes.cpu.flags.d = 0;

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

	nes.cpu.sp = 0xFF;
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

	{//if(m102 == 0x35 && (m103 == 0x53 || m103 == 0xAC)) {
		cpu_write(0x103,0x53);
		setscroll();
		loadbootfiles();
	}
	cpu_write(0x102,0xAC);		//action on reset
	nes.cpu.flags.i = 0;
}

static void enpf()
{
	nes.cpu.a = cpu_read(0xFE) | 0x08;
	cpu_write(0xFE,nes.cpu.a);
	cpu_write(0x2001,nes.cpu.a);
}

static void dispf()
{
	nes.cpu.a = cpu_read(0xFE) & 0xF7;
	cpu_write(0xFE,nes.cpu.a);
	cpu_write(0x2001,nes.cpu.a);
}

static void enobj()
{
	nes.cpu.a = cpu_read(0xFE) | 0x10;
	cpu_write(0xFE,nes.cpu.a);
	cpu_write(0x2001,nes.cpu.a);
}

static void disobj()
{
	nes.cpu.a = cpu_read(0xFE) & 0xEF;
	cpu_write(0xFE,nes.cpu.a);
	cpu_write(0x2001,nes.cpu.a);
}

static void enpfobj()
{
	enpf();
	enobj();
}

static void dispfobj()
{
	dispf();
	disobj();
}

static hle_call_t hle_calls[64] = {

	//$00 - disk related calls
	loadfiles,
	writefile,
	appendfile,
	0,
	0,
	0,
	0,
	0,

	//$08 - disk helper calls
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
	readordownpads,
	readdownverifypads,
	readordownverifypads,
	readdownexppads,
	0,

	//$18 - ppu vram related calls
	vramfill,
	vramstructwrite,
	writevrambuffer,
	readvrambuffer,
	preparevramstring,
	preparevramstrings,
	getvrambufferbyte,
	spritedma,

	//$20 - loadtileset and its internally used functions
	loadtileset,
	inc00by8,
	inc00bya,
	0,
	0,
	0,
	0,
	0,

	//$28 - ppu related calls
	enpf,
	dispf,
	enobj,
	disobj,
	enpfobj,
	dispfobj,
	setscroll,
	vintwait,

	//$30 - utility calls
	counterlogic,
	random,
	fetchdirectptr,
	jumpengine,
	memfill,
	0,
	0,
	0,

	//$38 - vectors
	nmi,
	irq,
	reset,
	0,

	//$3C - special calls for hle bios use only
	loadbootfiles,
	0,
	0,
	0,
};

u8 hlefds_read(u32 addr)
{
//	log_hle("hlefds_read:  register read!  $%04X\n",addr);
	switch(addr) {
		case 0x4222:
		case 0x4223:
		case 0x4224:
		case 0x4225:
			return(hleregs[addr - 0x4222]);
	}
	return(0);
}

void hlefds_write(u32 addr,u8 data)
{
	hle_call_t func = 0;
	int index = -1;

//	log_hle("hlefds_write:  register write!  $%04X = $%02X ($%02X $%02X $%02X $%02X)\n",addr,data,hleregs[0],hleregs[1],hleregs[2],hleregs[3]);
	switch(addr) {
		case 0x4220:
			data = hleregs[0];
		case 0x4221:
			data &= 0x3F;
			if((func = hle_calls[data]) == 0) {
				log_printf("hlefds_write:  hle call $%02X not implemented yet\n",data);
				return;
			}
			func();
			break;
		case 0x4222:
		case 0x4223:
		case 0x4224:
		case 0x4225:
			hleregs[addr - 0x4222] = data;
			return;
	}
}

void hlefds_cpucycle()
{
	//debugging the bios calls!
	static u16 lastopaddr = 0;
	static struct {int type;u8 hle;u16 addr;char *name;} funcaddrs[] = {

		//vectors
		{0,	0x38,		0xe18b,	"NMI"},
		{0,	0x39,		0xe1c7,	"IRQ"},
		{0,	0x3A,		0xee24,	"RESET"},

		//disk
		{1,	0x00,		0xe1f8,	"LoadFiles"},
		{1,	0xFF,		0xe237,	"AppendFile"},
		{1,	0xFF,		0xe239,	"WriteFile"},
		{1,	0xFF,		0xe2b7,	"CheckFileCount"},
		{1,	0xFF,		0xe2bb,	"AdjustFileCount"},
		{1,	0xFF,		0xe301,	"SetFileCount1"},
		{1,	0xFF,		0xe305,	"SetFileCount"},
		{1,	0xFF,		0xe32a,	"GetDiskInfo"},

		//util
		{2,	0xFF,		0xe149,	"Delay132"},
		{2,	0xFF,		0xe153,	"Delayms"},
		{2,	0xFF,		0xe161,	"DisPFObj"},
		{2,	0xFF,		0xe16b,	"EnPFObj"},
		{2,	0xFF,		0xe170,	"DisObj"},
		{2,	0xFF,		0xe178,	"EnObj"},
		{2,	0xFF,		0xe17e,	"DisPF"},
		{2,	0xFF,		0xe185,	"EnPF"},
		{2,	0xFF,		0xe1b2,	"VINTWait"},
		{2,	0xFF,		0xe7bb,	"VRAMStructWrite"},
		{2,	0xFF,		0xe844,	"FetchDirectPtr"},
		{2,	0xFF,		0xe86a,	"WriteVRAMBuffer"},
		{2,	0xFF,		0xe8b3,	"ReadVRAMBuffer"},
		{2,	0xFF,		0xe8d2,	"PrepareVRAMString"},
		{2,	0xFF,		0xe8e1,	"PrepareVRAMStrings"},
		{2,	0xFF,		0xe94f,	"GetVRAMBufferByte"},
		{2,	0xFF,		0xe97d,	"Pixel2NamConv"},
		{2,	0xFF,		0xe997,	"Nam2PixelConv"},
		{2,	0xFF,		0xe9b1,	"Random"},
		{2,	0xFF,		0xe9c8,	"SpriteDMA"},
		{2,	0xFF,		0xe9d3,	"CounterLogic"},
		{2,	0xFF,		0xe9eb,	"ReadPads"},
		{2,	0xFF,		0xea0d,	"OrPads"},
		{2,	0xFF,		0xea1a,	"DownPads"},
		{2,	0xFF,		0xea1f,	"ReadOrDownPads"},
		{2,	0xFF,		0xea36,	"ReadDownVerifyPads"},
		{2,	0xFF,		0xea4c,	"ReadOrDownVerifyPads"},
		{2,	0xFF,		0xea68,	"ReadDownExpPads"},
		{2,	0xFF,		0xea84,	"VRAMFill"},
		{2,	0x30,		0xead2,	"MemFill"},
		{2,	0xFF,		0xeaea,	"SetScroll"},
		{2,	0xFF,		0xeafd,	"JumpEngine"},
		{2,	0xFF,		0xeb13,	"ReadKeyboard"},
		{2,	0x1C,		0xebaf,	"LoadTileset"},
		{2,	0x1D,		0xeb66,	"Inc00by8"},
		{2,	0x1E,		0xeb68,	"Inc00byA"},
		{-1,	0xFF,		0x0000,	0},
	};

#define TAKEOVER(addr,hle) \
	if(nes.cpu.opaddr == addr) { \
		nes.cpu.readpages[addr >> 10][addr & 0x3FF] = 0x60; \
		hlefds_write(0x4221,hle); \
		handled = 1; \
	}

#define RAPE_RESET(addr,hle) \
	if(nes.cpu.opaddr == addr) { \
		nes.cpu.readpages[(addr + 0) >> 10][(addr + 0) & 0x3FF] = 0xA9; /* lda 'hle' */ \
		nes.cpu.readpages[(addr + 1) >> 10][(addr + 1) & 0x3FF] = hle; \
		nes.cpu.readpages[(addr + 2) >> 10][(addr + 2) & 0x3FF] = 0x8D; /* sta $4221 */ \
		nes.cpu.readpages[(addr + 3) >> 10][(addr + 3) & 0x3FF] = 0x21; \
		nes.cpu.readpages[(addr + 4) >> 10][(addr + 4) & 0x3FF] = 0x42; \
		nes.cpu.readpages[(addr + 5) >> 10][(addr + 5) & 0x3FF] = 0x6C; /* jmp ($DFFC) */ \
		nes.cpu.readpages[(addr + 6) >> 10][(addr + 6) & 0x3FF] = 0xFC; \
		nes.cpu.readpages[(addr + 7) >> 10][(addr + 7) & 0x3FF] = 0xDF; \
		handled = 1; \
	}

//pha/pla to work correctly
#define RAPE_IRQ(addr,hle) \
	if(nes.cpu.opaddr == addr) { \
		nes.cpu.readpages[(addr + 0) >> 10][(addr + 0) & 0x3FF] = 0xEA; \
		nes.cpu.readpages[(addr + 1) >> 10][(addr + 1) & 0x3FF] = 0xA9; /* lda 'hle' */ \
		nes.cpu.readpages[(addr + 2) >> 10][(addr + 2) & 0x3FF] = hle; \
		nes.cpu.readpages[(addr + 3) >> 10][(addr + 3) & 0x3FF] = 0x8D; /* sta $4221 */ \
		nes.cpu.readpages[(addr + 4) >> 10][(addr + 4) & 0x3FF] = 0x21; \
		nes.cpu.readpages[(addr + 5) >> 10][(addr + 5) & 0x3FF] = 0x42; \
		nes.cpu.readpages[(addr + 6) >> 10][(addr + 6) & 0x3FF] = 0x4C; /* jmp $4222 */ \
		nes.cpu.readpages[(addr + 7) >> 10][(addr + 7) & 0x3FF] = 0x22; \
		nes.cpu.readpages[(addr + 8) >> 10][(addr + 8) & 0x3FF] = 0x42; \
		handled = 1; \
	}

#define RAPE_NMI(addr,hle) \
	if(nes.cpu.opaddr == addr) { \
		nes.cpu.readpages[(addr + 0) >> 10][(addr + 0) & 0x3FF] = 0xEA; \
		nes.cpu.readpages[(addr + 1) >> 10][(addr + 1) & 0x3FF] = 0xA9; /* lda 'hle' */ \
		nes.cpu.readpages[(addr + 2) >> 10][(addr + 2) & 0x3FF] = hle; \
		nes.cpu.readpages[(addr + 3) >> 10][(addr + 3) & 0x3FF] = 0x8D; /* sta $4221 */ \
		nes.cpu.readpages[(addr + 4) >> 10][(addr + 4) & 0x3FF] = 0x21; \
		nes.cpu.readpages[(addr + 5) >> 10][(addr + 5) & 0x3FF] = 0x42; \
		nes.cpu.readpages[(addr + 6) >> 10][(addr + 6) & 0x3FF] = 0x60; /* rts */ \
		handled = 1; \
	}

	//force hle!  takeover!
	//if the opaddr changes we are reading an opcode
	if(nes.cpu.opaddr >= 0xE000 && nes.cpu.opaddr != lastopaddr) {
		int i;
		int handled = 0;

		lastopaddr = nes.cpu.opaddr;

		RAPE_RESET(0xee24,0x3A);	//reset
//		RAPE_NMI(0xe18b,0x38);		//nmi
//		RAPE_IRQ(0xe1c7,0x39);		//irq

		TAKEOVER(0xe1f8,0x00);		//loadfiles
		TAKEOVER(0xe239,0x01);		//writefile
		TAKEOVER(0xe237,0x02);		//appendfile

		TAKEOVER(0xe9eb,0x10);		//readpads
		TAKEOVER(0xea0d,0x11);		//orpads
		TAKEOVER(0xea1a,0x12);		//downpads
		TAKEOVER(0xea1f,0x13);		//readordownpads

		TAKEOVER(0xea84,0x18);		//vramfill
		TAKEOVER(0xe7bb,0x19);		//vramstructwrite
		TAKEOVER(0xe86a,0x1A);		//writevrambuffer
//		TAKEOVER(0xe8b3,0x1B);		//readvrambuffer
		TAKEOVER(0xe8d2,0x1C);		//preparevramstring
// preparevramstrings
// getvrambufferbyte
		TAKEOVER(0xe9c8,0x1F);		//spritedma

		TAKEOVER(0xebaf,0x20);		//loadtileset
		TAKEOVER(0xeb66,0x21);		//inc00by8
		TAKEOVER(0xeb68,0x22);		//inc00byA

		TAKEOVER(0xe185,0x28);		//enpf
		TAKEOVER(0xe17e,0x29);		//dispf
		TAKEOVER(0xe178,0x2A);		//enobj
		TAKEOVER(0xe170,0x2B);		//disobj
		TAKEOVER(0xe16b,0x2C);		//enpfobj
		TAKEOVER(0xe161,0x2D);		//dispfobj
		TAKEOVER(0xeaea,0x2E);		//setscroll
//		TAKEOVER(0xe1b2,0x2F);		//vintwait
		if(nes.cpu.opaddr == 0xe1b2) {
			nes.cpu.readpages[0xe1b2 >> 10][0xe1b2 & 0x3FF] = 0xD0;  //BNE forever!
			nes.cpu.readpages[0xe1b3 >> 10][0xe1b3 & 0x3FF] = 0xFE;
			hlefds_write(0x4221,0x2F);
			handled = 1;
		}

		TAKEOVER(0xe9d3,0x30);		//counterlogic
		TAKEOVER(0xe9b1,0x31);		//random
		TAKEOVER(0xe844,0x32);		//fetchdirectptr
// jumpengine
		TAKEOVER(0xead2,0x34);		//memfill

//		handled = 0;

		for(i=0;funcaddrs[i].type >= 0 && handled == 0;i++) {
			if(funcaddrs[i].addr == nes.cpu.opaddr) {
				int t = funcaddrs[i].type;
				char *types[4] = {"VECTOR","DISK","UTIL","MISC"};

				if(t > 0) //(t < 4)
					log_hle("fds.c:  bios %s:  calling $%04X ('%s') (pixel %d, line %d, frame %d) (A:%02X X:%02X Y:%02X)\n",types[t],funcaddrs[i].addr,funcaddrs[i].name,LINECYCLES,SCANLINE,FRAMES,nes.cpu.a,nes.cpu.x,nes.cpu.y);
			}
		}
	}
//end force
}
