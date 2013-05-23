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
#include "mappers/fds/calls.h"
#include "nes/nes.h"

typedef void (*hle_call_t)();

typedef struct hle_call_info_s {

	//group the call belongs to (negative = disable)
	s8 group;

	//hle call index ($FF = dont use)
	u8 hle;

	//entry point in the bios
	u16 addr;

	//name of call
	char *name;

	//block to code for function (terminate with 0 or 2)
	//if first byte is 0, use default method of calling (call in this code, insert rts into 6502 code)
	//if last byte is 0, terminate code replacing
	//if last byte is 2, call the hle function them terminate
	u8 code[8];

} hle_call_info_t;

static hle_call_t hle_calls[64] = {

	//$00 - disk related calls
	hle_loadfiles,
	hle_writefile,
	hle_appendfile,
	hle_getdiskinfo,
	hle_checkfilecount,
	hle_adjustfilecount,
	hle_setfilecount1,
	hle_setfilecount,

	//$08 - disk helper calls
	hle_xferdone,
	hle_checkdiskheader,
	hle_getnumfiles,
	hle_checkblocktype,
	hle_filematchtest,
	hle_loaddata,
	0,
	0,

	//$10 - pad calls
	hle_readpads,
	hle_orpads,
	hle_downpads,
	hle_readordownpads,
	hle_readdownverifypads,
	hle_readordownverifypads,
	hle_readdownexppads,
	hle_readkeyboard,

	//$18 - ppu vram related calls
	hle_vramfill,
	hle_vramstructwrite,
	hle_writevrambuffer,
	hle_readvrambuffer,
	hle_preparevramstring,
	hle_preparevramstrings,
	hle_getvrambufferbyte,
	hle_loadtileset,

	//$20 - ???
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,

	//$28 - ppu related calls
	hle_enpf,
	hle_dispf,
	hle_enobj,
	hle_disobj,
	hle_enpfobj,
	hle_dispfobj,
	hle_setscroll,
	hle_spritedma,

	//$30 - utility calls
	hle_counterlogic,
	hle_random,
	hle_fetchdirectptr,
	hle_jumpengine,
	hle_memfill,
	hle_pixel2nam,
	hle_nam2pixel,
	hle_gethcparam,

	//$38 - misc calls
	hle_delay132,
	hle_delayms,
	hle_vintwait,
	hle_unk_EC22,

	//$3C - vectors
	hle_nmi,
	hle_irq,
	hle_reset,
	0,
};

//can be optimized:  use hle call index as index of entry in this table
static hle_call_info_t hle_call_table[] = {

	//vectors
	{1,	0x3C,		0xe18b,	"NMI",						{0x4C,0x80,0x42,2}},	//jmp $4280
	{1,	0x3D,		0xe1c7,	"IRQ",						{0x4C,0x88,0x42,2}},	//jmp $4288
	{1,	0x3E,		0xee24,	"RESET",						{0x6C,0xFC,0xDF,2}},	//jmp ($DFFC)

	//disk
	{2,	0x00,		0xe1f8,	"LoadFiles",				{0}},
	{2,	0x02,		0xe237,	"AppendFile",				{0}},
	{2,	0x01,		0xe239,	"WriteFile",				{0}},
	{2,	0xFF,		0xe2b7,	"CheckFileCount",			{0}},
	{2,	0xFF,		0xe2bb,	"AdjustFileCount",		{0}},
	{2,	0xFF,		0xe301,	"SetFileCount1",			{0}},
	{2,	0xFF,		0xe305,	"SetFileCount",			{0}},
	{2,	0xFF,		0xe32a,	"GetDiskInfo",				{0}},

	//pads
	{3,	0x10,		0xe9eb,	"ReadPads",					{0}},
	{3,	0x11,		0xea0d,	"OrPads",					{0}},
	{3,	0x12,		0xea1a,	"ReadDownPads",			{0}},
	{3,	0x13,		0xea1f,	"ReadOrDownPads",			{0}},
	{3,	0x14,		0xea36,	"ReadDownVerifyPads",	{0}},
	{3,	0x15,		0xea4c,	"ReadOrDownVerifyPads",	{0}},
	{3,	0x16,		0xea68,	"ReadDownExpPads",		{0}},
	{3,	0x17,		0xeb13,	"ReadKeyboard",			{0}},

	//vram
	{4,	0x18,		0xea84,	"VRAMFill",					{0}},
	{4,	0x19,		0xe7bb,	"VRAMStructWrite",		{0}},
	{4,	0x1A,		0xe86a,	"WriteVRAMBuffer",		{0}},
	{4,	0x1B,		0xe8b3,	"ReadVRAMBuffer",			{0}},
	{4,	0x1C,		0xe8d2,	"PrepareVRAMString",		{0}},
	{4,	0x1D,		0xe8e1,	"PrepareVRAMStrings",	{0}},
	{4,	0x1E,		0xe94f,	"GetVRAMBufferByte",		{0}},
	{4,	0x1F,		0xebaf,	"LoadTileset",				{0}},

	//ppu
	{5,	0x28,		0xe185,	"EnPF",						{0}},
	{5,	0x29,		0xe17e,	"DisPF",						{0}},
	{5,	0x2A,		0xe178,	"EnObj",						{0}},
	{5,	0x2B,		0xe171,	"DisObj",					{0}},
	{5,	0x2C,		0xe16b,	"EnPFObj",					{0}},
	{5,	0x2D,		0xe161,	"DisPFObj",					{0}},
	{5,	0x2E,		0xeaea,	"SetScroll",				{0}},
	{5,	0x2F,		0xe9c8,	"SpriteDMA",				{0}},

	//util
	{6,	0x30,		0xe9d3,	"CounterLogic",			{0}},
	{6,	0x31,		0xe9b1,	"Random",					{0}},
	{6,	0x32,		0xe844,	"FetchDirectPtr",			{0}},
	{6,	0x33,		0xeafd,	"JumpEngine",				{0x4C,0x90,0x42,2}},	//jmp $4290
	{6,	0x34,		0xead2,	"MemFill",					{0}},
	{6,	0x35,		0xe97d,	"Pixel2NamConv",			{0}},
	{6,	0x36,		0xe997,	"Nam2PixelConv",			{0}},
	{6,	0xFF,		0xe3e7,	"GetHCPwNWPchk",			{0}},
	{6,	0xFF,		0xe3ea,	"GetHCPwWPchk",			{0}},
	{6,	0x37,		0xe3eb,	"GetHCParam",				{0}},

	//misc
	{7,	0x38,		0xe149,	"Delay132",					{0}},
	{7,	0x39,		0xe153,	"Delayms",					{0}},
	{7,	0x3A,		0xe1b2,	"VINTWait",					{0xD0,0xFE,2}},	//bne forever
	{-7,	0x3B,		0xec22,	"unk_EC22",					{0}},

	//disk helpers
	{8,	0x08,		0xe778,	"XferDone",					{0}},
	{8,	0x09,		0xe445,	"CheckDiskHeader",		{0}},
	{8,	0x0A,		0xe484,	"GetNumFiles",				{0}},
	{8,	0x0B,		0xe68f,	"CheckBlkType",			{0}},
	{8,	0x0C,		0xe4a0,	"FileMatchTest",			{0}},
	{8,	0x0D,		0xe4f9,	"LoadData",					{0}},

	//end
	{-1,	0xFF,		0x0000,	0,								{0}},

};

//four hle registers
u8 hleregs[4];

//128 bytes of temporary memory
u8 hlemem[128];

//log hle calls
int hlelog = 1;

//get parameter from the bytes after the jsr
//n = parameter index (0 is the first, 1 is the second, etc)
u32 hle_getparam(int n)
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
void hle_fixretaddr(int n)
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

u8 hlefds_read(u32 addr)
{
//	log_hle("hlefds_read:  register read!  $%04X\n",addr);

	//return from the registers
	switch(addr) {
		case 0x4222:
		case 0x4223:
		case 0x4224:
		case 0x4225:
			return(hleregs[addr - 0x4222]);
	}

	//read from temporary memory
	if(addr >= 0x4280 && addr < 0x4300)
		return(hlemem[addr & 0x7F]);

	//not handled
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

	//write to temporary memory
	if(addr >= 0x4280 && addr < 0x4300) {
		log_printf("hlefds_write:  temporary memory is read-only!\n");
//		hlemem[addr & 0x7F] = data;
	}
}

#define READPAGE(addr)	nes.cpu.readpages[(addr) >> 10][(addr) & 0x3FF]

extern int showdisasm;

//intercept calls into the real fds bios and replace with the hle calls
void hlefds_intercept()
{
	static u16 lastopaddr = 0;
	int i,j;
	u8 byte;

//	showdisasm = 1;

	//if the opaddr changes, we are reading an opcode
	if(nes.cpu.opaddr >= 0xE000 && nes.cpu.opaddr != lastopaddr) {

		//save op address
		lastopaddr = nes.cpu.opaddr;

		for(i=0;hle_call_table[i].name;i++) {

			//if this call is disabled, skip it
			if(hle_call_table[i].group < 0)
				continue;

			//check for match and if we have matching hle function
			if(nes.cpu.opaddr == hle_call_table[i].addr && hle_call_table[i].hle != 0xFF) {

				log_printf("intercepting at $%04X (%s) with hle call $%02X\n",nes.cpu.opaddr,hle_call_table[i].name,hle_call_table[i].hle);

				//if first byte of 'code' is 0, use default calling method
				if(hle_call_table[i].code[0] == 0) {

					//insert NOP + RTS
					READPAGE(nes.cpu.opaddr+0) = 0xEA;
					READPAGE(nes.cpu.opaddr+1) = 0x60;

					//call the hle function
					hlefds_write(0x4221,hle_call_table[i].hle);
				}

				//insert code for call
				else {

					//NOP
					READPAGE(nes.cpu.opaddr) = 0xEA;

					//copy the code data
					for(j=0;;j++) {
						byte = hle_call_table[i].code[j];
						if(byte == 0)
							break;
						else if(byte == 2) {
							hlefds_write(0x4221,hle_call_table[i].hle);
							break;
						}
						READPAGE(nes.cpu.opaddr + 1 + j) = byte;
					}
				}

				//done
				return;
			}
		}
	}
}

void hlefds_cpucycle()
{
	static u16 lastopaddr = 0;
	int i,found = 0;

//	showdisasm = 1;
	if(nes.cpu.opaddr >= 0xE000 && lastopaddr < 0xE000) {
		for(i=0;hle_call_table[i].name;i++) {
			if(hle_call_table[i].addr == nes.cpu.opaddr) {
				int t = hle_call_table[i].group;
				char *types[9] = {"???","VECTOR","DISK","PAD","PPU","VRAM","UTIL","MISC","DISK2"};

				t = abs(t);
				t = (t > 8) ? 0 : t;
				found = 1;
				log_printf("hle.c:  bios %s:  calling $%04X ('%s') (pixel %d, line %d, frame %d) (A:%02X X:%02X Y:%02X SP:%02X)\n",types[t],hle_call_table[i].addr,hle_call_table[i].name,LINECYCLES,SCANLINE,FRAMES,nes.cpu.a,nes.cpu.x,nes.cpu.y,nes.cpu.sp);
			}
		}
		if(found == 0) {
			log_printf("hle.c:  bios:  calling $%04X (unknown) from $%04X (pixel %d, line %d, frame %d) (A:%02X X:%02X Y:%02X SP:%02X)\n",nes.cpu.opaddr,lastopaddr,LINECYCLES,SCANLINE,FRAMES,nes.cpu.a,nes.cpu.x,nes.cpu.y,nes.cpu.sp);
		}
	}
	lastopaddr = nes.cpu.opaddr;
}
