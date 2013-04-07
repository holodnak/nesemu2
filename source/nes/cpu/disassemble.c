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

#include <stdio.h>
#include <string.h>
#include "nes/cpu/cpu.h"

static char opcodes[256][4] = {
/*  x0    x1    x2    x3    x4    x5    x6    x7    x8    x9    xA    xB    xC    xD    xE    xF           */
 	"BRK","ORA","???","???","NOP","ORA","ASL","???","PHP","ORA","ASL","???","NOP","ORA","ASL","???", /*00-0F*/
	"BPL","ORA","???","???","NOP","ORA","ASL","???","CLC","ORA","NOP","???","NOP","ORA","ASL","???", /*10-1F*/
	"JSR","AND","???","???","BIT","AND","ROL","???","PLP","AND","ROL","???","BIT","AND","ROL","???", /*20-2F*/
	"BMI","AND","???","???","NOP","AND","ROL","???","SEC","AND","NOP","???","NOP","AND","ROL","???", /*30-3F*/
	"RTI","EOR","???","???","NOP","EOR","LSR","???","PHA","EOR","LSR","???","JMP","EOR","LSR","???", /*40-4F*/
	"BVC","EOR","???","???","NOP","EOR","LSR","???","CLI","EOR","NOP","???","NOP","EOR","LSR","???", /*50-5F*/
	"RTS","ADC","???","???","NOP","ADC","ROR","???","PLA","ADC","ROR","???","JMP","ADC","ROR","???", /*60-6F*/
	"BVS","ADC","???","???","NOP","ADC","ROR","???","SEI","ADC","NOP","???","NOP","ADC","ROR","???", /*70-7F*/
	"NOP","STA","NOP","SAX","STY","STA","STX","SAX","DEY","NOP","TXA","???","STY","STA","STX","SAX", /*80-8F*/
	"BCC","STA","???","???","STY","STA","STX","SAX","TYA","STA","TXS","???","???","STA","???","???", /*90-9F*/
	"LDY","LDA","LDX","LAX","LDY","LDA","LDX","LAX","TAY","LDA","TAX","???","LDY","LDA","LDX","LAX", /*A0-AF*/
	"BCS","LDA","???","LAX","LDY","LDA","LDX","LAX","CLV","LDA","TSX","???","LDY","LDA","LDX","LAX", /*B0-BF*/
	"CPY","CMP","NOP","???","CPY","CMP","DEC","???","INY","CMP","DEX","???","CPY","CMP","DEC","???", /*C0-CF*/
	"BNE","CMP","???","???","NOP","CMP","DEC","???","CLD","CMP","NOP","???","NOP","CMP","DEC","???", /*D0-DF*/
	"CPX","SBC","???","???","CPX","SBC","INC","???","INX","SBC","NOP","SBC","CPX","SBC","INC","???", /*E0-EF*/
	"BEQ","SBC","???","???","NOP","SBC","INC","???","SED","SBC","NOP","???","NOP","SBC","INC","???"  /*F0-FF*/
};
enum addrmodes {
	er=0,no,ab,ax,ay,ac,im,ix,iy,in,re,zp,zx,zy
};
static u8 addrtable[256] = {
/* x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xA xB xC xD xE xF         */
   no,ix,er,er,zp,zp,zp,er,no,im,ac,er,ab,ab,ab,er, /*00-0f*/
   re,iy,er,er,zx,zx,zx,er,no,ay,im,er,ax,ax,ax,er, /*10-1f*/
   ab,ix,er,er,zp,zp,zp,er,no,im,ac,er,ab,ab,ab,er, /*20-2f*/
   re,iy,er,er,zx,zx,zx,er,no,ay,im,er,ax,ax,ax,er, /*30-3f*/
   no,ix,er,er,zp,zp,zp,er,no,im,ac,er,ab,ab,ab,er, /*40-4f*/
   re,iy,er,er,zx,zx,zx,er,no,ay,im,er,ax,ax,ax,er, /*50-5f*/
   no,ix,er,er,zp,zp,zp,er,no,im,ac,er,in,ab,ab,er, /*60-6f*/
   re,iy,er,er,zx,zx,zx,er,no,ay,im,er,ax,ax,ax,er, /*70-7f*/
 	im,ix,im,ix,zp,zp,zp,zp,no,im,no,er,ab,ab,ab,ab, /*80-8f*/
   re,iy,er,er,zx,zx,zy,zy,no,ay,no,er,er,ax,er,er, /*90-9f*/
   im,ix,im,ix,zp,zp,zp,zp,no,im,no,er,ab,ab,ab,ab, /*a0-af*/
   re,iy,er,iy,zx,zx,zy,zx,no,ay,no,er,ax,ax,ay,ax, /*b0-bf*/
   im,ix,im,er,zp,zp,zp,er,no,im,no,er,ab,ab,ab,er, /*c0-cf*/
   re,iy,er,er,zx,zx,zx,er,no,ay,im,er,ax,ax,ax,er, /*d0-df*/
   im,ix,er,er,zp,zp,zp,er,no,im,no,im,ab,ab,ab,er, /*e0-ef*/
   re,iy,er,er,zx,zx,zx,er,no,ay,im,er,ax,ax,ax,er  /*f0-ff*/
};
static u8 oplength[256];

u16 cpu_disassemble(char *buffer,u16 opcodepos)
{
	u8 opcode,size;
	u16 addr;

	strcpy(buffer,"");
	opcode = cpu_read(opcodepos);
	switch(addrtable[opcode]) {
		case er:size = 1;sprintf(buffer,".u8 $%02x",opcode);break;
		case no:size = 1;sprintf(buffer,"%02X       %s",opcode,opcodes[opcode]);break;
		case ac:size = 1;sprintf(buffer,"%02X       %s a",opcode,opcodes[opcode]);break;
		case ab:
			size = 3;
			addr = cpu_read(opcodepos+1) | (cpu_read(opcodepos+2) << 8);
			if(addr < 0x100) { /*should be zero page, so its bad opcode*/
				size = 1;
				sprintf(buffer,".u8 $%02x",opcode);
				break;
			}
			sprintf(buffer,"%02X %02X %02X %s $%04x",opcode,addr & 0xFF,(addr >> 8) & 0xFF,opcodes[opcode],addr);
			break;
		case ax:
			size = 3;
			addr = cpu_read(opcodepos+1) | (cpu_read(opcodepos+2) << 8);
			if(addr < 0x100) { /*should be zero page, so its bad opcode*/
				size = 1;
				sprintf(buffer,".u8 $%02x",opcode);
				break;
			}
			sprintf(buffer,"%s $%04x,x",opcodes[opcode],addr);
			break;
		case ay:
			size = 3;
			addr = cpu_read(opcodepos+1) | (cpu_read(opcodepos+2) << 8);
			if(addr < 0x100) { /*should be zero page, so its bad opcode*/
				size = 1;
				sprintf(buffer,".u8 $%02x",opcode);
				break;
			}
			sprintf(buffer,"%s $%04x,y",opcodes[opcode],addr);
			break;
		case in:size = 3;sprintf(buffer,"%s ($%04X)",opcodes[opcode],cpu_read(opcodepos+1) | (cpu_read(opcodepos+2) << 8));break;
		case im:size = 2;sprintf(buffer,"%02X %02X    %s #$%02X",opcode,cpu_read(opcodepos+1),opcodes[opcode],cpu_read(opcodepos+1));break;
		case ix:size = 2;sprintf(buffer,"%02X %02X    %s ($%02X,x)",opcode,cpu_read(opcodepos+1),opcodes[opcode],cpu_read(opcodepos+1));break;
		case iy:size = 2;sprintf(buffer,"%02X %02X    %s ($%02X),y",opcode,cpu_read(opcodepos+1),opcodes[opcode],cpu_read(opcodepos+1));break;
		case re:size = 2;sprintf(buffer,"%02X %02X    %s $%04X",opcode,cpu_read(opcodepos+1),opcodes[opcode],opcodepos+size+((s8)cpu_read(opcodepos+1)));break;
		case zp:size = 2;sprintf(buffer,"%02X %02X    %s $%02X",opcode,cpu_read(opcodepos+1),opcodes[opcode],cpu_read(opcodepos+1));break;
		case zx:size = 2;sprintf(buffer,"%02X %02X    %s $%02X,x",opcode,cpu_read(opcodepos+1),opcodes[opcode],cpu_read(opcodepos+1));break;
		case zy:size = 2;sprintf(buffer,"%02X %02X    %s $%02X,y",opcode,cpu_read(opcodepos+1),opcodes[opcode],cpu_read(opcodepos+1));break;
		default:size = 1;sprintf(buffer,"disassembler bug");break;
	}
	return(opcodepos + size);
}

void cpu_disassemble_init()
{
int i;

for(i=0;i<256;i++)
	{
	switch(addrtable[i])
		{
		case er:
		case no:
		case ac:oplength[i] = 1;break;
		case im:
		case ix:
		case iy:
		case re:
		case zp:
		case zx:
		case zy:oplength[i] = 2;break;
		case ab:
		case ax:
		case ay:
		case in:oplength[i] = 3;break;
		}
	}
}
