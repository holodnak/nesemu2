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
	u8 data,count,tmp;

	//bios saves these here
	cpu_write(0,nes.cpu.a);
	cpu_write(1,nes.cpu.x);
	cpu_write(2,nes.cpu.y);

	//reset ppu toggle
	cpu_read(0x2000);

	//write ppu control, increment 2007 writes by 1
	tmp = cpu_read(0xFF);
	tmp &= 0xFB;
	cpu_write(0xFF,tmp);
	cpu_write(0x2000,tmp);

	//set vram address
	cpu_write(0x2006,nes.cpu.a);
	cpu_write(0x2006,0);

	//data to fill with
	data = nes.cpu.x;

	//fill vram
	if(nes.cpu.a < 0x20) {
		count = nes.cpu.y;
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

}

static void memfill()
{
	int i;
	u32 start,stop;

	log_printf("memfill:  fill start $%04X, stop $%04X, data $%02X\n",nes.cpu.x << 8,nes.cpu.y << 8,nes.cpu.a);
	start = nes.cpu.x << 8;
	stop = nes.cpu.y << 8;
	while(start <= stop) {
		for(i=0;i<256;i++) {
			cpu_write(start + i,nes.cpu.a);
		}
		start++;
	}
	
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

//insert disk
static void forceinsert()
{
	log_printf("forceinsert:  disk inserted, side = 0\n");
	diskside = 0;
}


static hle_call_t hle_calls[64] = {

/*	$e1f8 	LoadFiles 	Pointer to Disk ID, Pointer to File List 	A = error #, Y = # of files loaded 	Loads files specified by DiskID into memory from disk. Load addresses are decided by the file's header.
	$e237 	AppendFile 	Pointer to Disk ID, Pointer to File Header 	A = error # 	Appends the file data given by DiskID to the disk. This means that the file is tacked onto the end of the disk, and the disk file count is incremented. The file is then read back to verify the write. If an error occurs during verification, the disk's file count is decremented (logically hiding the written file).
	$e239 	WriteFile 	Pointer to Disk ID, Pointer to File Header, A = file # 	A = error # 	Same as "Append File", but instead of writing the file to the end of the disk, A specifies the sequential position on the disk to write the file (0 is the first). This also has the effect of setting the disk's file count to the A value, therefore logically hiding any other files that may reside after the written one.
	$e2b7 	CheckFileCount 	Pointer to Disk ID, A = # to set file count to 	A = error # 	Reads in disk's file count, compares it to A, then sets the disk's file count to A.
	$e2bb 	AdjustFileCount 	Pointer to Disk ID, A = number to reduce current file count by 	A = error # 	Reads in disk's file count, decrements it by A, then writes the new value back.
	$e301 	SetFileCount1 	Pointer to Disk ID, A = file count minus one = # of the last file 	A = error # 	Set the file count to A + 1
	$e305 	SetFileCount 	Pointer to Disk ID, A = file count 	A = error # 	Set the file count to A
	$e32a 	GetDiskInfo 	Pointer to Disk Info 	A = error # 	Fills DiskInfo up with data read off the current disk. 
*/
	//disk related calls
	//$0
	loadfiles,
	0,//hle_writefile,
	0,//hle_checkfilecount,
	0,//hle_adjfilecount,
	0,//hle_setfilecount1,
	0,//hle_setfilecount,
	0,//hle_getdiskinfo,
	0,//hle_xferbyte,

	//$8
	0,//hle_checkblktype,
	0,//hle_writeblktype,
	0,//hle_endofblkread,
	0,//hle_endofblkwrite,
	0,//hle_ppurestore,
	0,//hle_ppudataproc,
	0,//hle_memfill,
	0,

/*	$e149 	Delay132 				132 clock cycle delay
	$e153 	Delayms 			X, Y 	Delay routine, Y = delay in ms (approximate)
	$e161 	DisPFObj 			A, $fe 	Disable sprites and background
	$e16b 	EnPFObj 			A, $fe 	Enable sprites and background
	$e170 	DisObj 			A, $fe 	Disable sprites
	$e178 	EnObj 			A, $fe 	Enable sprites
	$e17e 	DisPF 			A, $fe 	Disable background
	$e185 	EnPF 			A, $fe 	Enable background
	$e1b2 	VINTWait 			$ff 	Wait until next VBlank NMI fires, and return (for programs that does it the "everything in main" way). NMI vector selection at $100 is preserved, but further VBlanks are disabled.
	$e7bb 	VRAMStructWrite 	Pointer to VRAM buffer to be written 		A, X, Y, $00, $01, $ff 	Set VRAM increment to 1 (clear $2000/$ff bit 2), and write a VRAM buffer to VRAM. Read below for information on the structure.
	$e844 	FetchDirectPtr 		$00, $01 = pointer fetched 	A, X, Y, $05, $06 	Fetch a direct pointer from the stack (the pointer should be placed after the return address of the routine that calls this one (see "important notes" above)), save the pointer at ($00) and fix the return address.
	$e86a 	WriteVRAMBuffer 			A, X, Y, $301, $302 	Write the VRAM Buffer at $302 to VRAM. Read below for information on the structure.
	$e8b3 	ReadVRAMBuffer 	X = start address in buffer, Y = # of bytes to read 		A, X, Y 	Read individual bytes from VRAM to the VRAMBuffer. Apparently this routine has been made so that, when combined to WriteVRAMBuffer, it can easily do read-modify-write operations to individual VRAM bytes.
	$e8d2 	PrepareVRAMString 	A = High VRAM address, X = Low VRAM address, Y = string length, Direct Pointer = data to be written to VRAM 	A = $ff : no error, A = $01 : string didn't fit in buffer 	A, X, Y, $00, $01, $02, $03, $04, $05, $06 	This routine copies pointed data into the VRAM buffer.
	$e8e1 	PrepareVRAMStrings 	A = High VRAM address, X = Low VRAM address, Direct pointer = data to be written to VRAM 	A = $ff : no error, A = $01 : data didn't fit in buffer 	A, X, Y, $00, $01, $02, $03, $04, $05, $06 	This routine copies a 2D string into the VRAM buffer. The first byte of the data determines the width and height of the following string (in tiles): Upper nybble = height, lower nybble = width.
	$e94f 	GetVRAMBufferByte 	X = starting index, Y = # of byte to read in buffer (starting at 1), $00, $01 = address to read from 	carry clear : a byte was read, carry set : no byte was read 	A, X, Y 	This routine is somewhat obscure, and was probably meant to be used in pair with ReadVRAMBuffer. First the index is calculated as x+3*(y-1). Then the address stored at that index is compared with the address in ($00). If match, the third byte is read and the routine exit with c clear. If not match, the address is overwritten by the address in ($00) and the routine exit with c set. Exact details in how this could be any useful is under analysis.
	$e97d 	Pixel2NamConv 	$02 = Pixel X cord, $03 = Pixel Y cord 	$00 = High nametable address, $01 = Low nametable address 	A 	This routine convert pixel screen coordinates to corresponding nametable address (assumes no scrolling, and points to first nametable at $2000-$23ff).
	$e997 	Nam2PixelConv 	$00 = High nametable address, $01 = low nametable address 	$02 = Pixel X cord, $03 = Pixel Y cord 	A 	This routine convert a nametable address to corresponding pixel coordinates (assume no scrolling).
	$e9b1 	Random 	X = Zero Page address where the random bytes are placed, Y = # of shift register bytes (normally $02) 		A, X, Y, $00 	This is a shift-register based random number generator, normally takes 2 bytes (using more won't affect random sequence). On reset you are supposed to write some non-zero values here (BIOS uses writes $d0, $d0), and call this routine several times before the data is actually random. Each call of this routine will shift the bytes right.
	$e9c8 	SpriteDMA 			A 	This routine does sprite DMA from RAM $200-$2ff
	$e9d3 	CounterLogic 	A, Y = end Zeropage address of counters, X = start zeropage address of counters 		A, X, $00 	This decrements several counters in Zeropage. The first counter is a decimal counter 9 -> 8 -> 7 -> ... -> 1 -> 0 -> 9 -> ... Counters 1...A are simply decremented and stays at 0. Counters A+1...Y are decremented when the first counter does a 0 -> 9 transition, and stays at 0.

	$e9eb 	ReadPads 		$f5 = Joypad #1 data, $f6 = Joypad #2 data 	A, X, $00, $01, 	This read hardwired famicom joypads.
	$ea1a 	ReadDownPads 		$f5 = Joypad #1 up->down transitions, $f6 = Joypad #2 up->down transitions $f7 = Joypad #1 data, $f8 = Joypad #2 data 	A, X, $00, $01 	This reads hardwired famicom joypads, and detect up->down button transitions
	$ea1f 	ReadOrDownPads 		$f5 = Joypad #1 up->down transitions, $f6 = Joypad #2 up->down transitions $f7 = Joypad #1 data, $f8 = Joypad #2 data 	A, X, $00, $01 	This read both hardwired famicom and expansion port joypads and detect up->down button transitions.
	$ea36 	ReadDownVerifyPads 		$f5 = Joypad #1 up->down transitions, $f6 = Joypad #2 up->down transitions $f7 = Joypad #1 data, $f8 = Joypad #2 data 	A, X, $00, $01 	This reads hardwired Famicom joypads, and detect up->down button transitions. Data is read until two consecutive read matches to work around the DMC reading glitches.
	$ea4c 	ReadOrDownVerifyPads 		$f5 = Joypad #1 up->down transitions, $f6 = Joypad #2 up->down transitions $f7 = Joypad #1 data, $f8 = Joypad #2 data 	A, X, $00, $01 	This read both hardwired famicom and expansion port joypads and detect up->down button transitions. Data is read until two consecutive read matches to work around the DMC reading glitches.
	$ea68 	ReadDownExpPads 	$f1-$f4 = up->down transitions, $f5-$f8 = Joypad data in the order : Pad1, Pad2, Expansion1, Expansion2 		A, X, $00, $01 	This read both hardwired famicom and expansion port joypad, but stores their data separately instead of ORing them together like the other routines does. This routine is NOT DMC fortified.
	$ea84 	VRAMFill 	A = High VRAM Address (aka tile row #), X = Fill value, Y = # of tile rows OR attribute fill data 		A, X, Y, $00, $01, $02 	This routine does 2 things : If A < $20, it fills pattern table data with the value in X for 16 * Y tiles. If A >= $20, it fills the corresponding nametable with the value in X and attribute table with the value in Y.
	$ead2 	MemFill 	A = fill value, X = first page #, Y = last page # 		A, X, Y, $00, $01 	This routines fills RAM pages with specified value.

	$eaea 	SetScroll 			A 	This routine set scroll registers according to values in $fc, $fd and $ff. Should typically be called in VBlank after VRAM updates
	$eafd 	JumpEngine 	A = Jump table entry 		A, X, Y, $00, $01 	The instruction calling this is supposed to be followed by a jump table (16-bit pointers little endian, up to 128 pointers). A is the entry # to jump to, return address on stack is used to get jump table entries.
	$eb13 	ReadKeyboard 				Read Family Basic Keyboard expansion (detail is under analysis)
	$eb66 	LoadTileset 	A = Low VRAM Address & Flags, Y = Hi VRAM Address, X = # of tiles to transfer to/from VRAM 		A, X, Y, $00, $01, $02, $03, $04 	This routine can read and write 2BP and 1BP tilesets to/from VRAM. See appendix below about the flags. 
*/

//other calls
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
	spritedma,
	vramstructwrite,
	0,
	0,
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
	0,
	0,
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

	//$38 - special calls for hle bios use only!
	forceinsert,
	0,
	0,
	0,
	0,
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
