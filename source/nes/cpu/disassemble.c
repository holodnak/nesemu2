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

enum addrmodes {
	er=0,no,ab,ax,ay,ac,im,ix,iy,in,re,zp,zx,zy
};

#ifdef CPU_UNDOC
static char opcodes[256][4] = {
/*  x0    x1    x2    x3    x4    x5    x6    x7    x8    x9    xA    xB    xC    xD    xE    xF           */
 	"BRK","ORA","???","SLO","NOP","ORA","ASL","SLO","PHP","ORA","ASL","AAC","NOP","ORA","ASL","SLO", /*00-0F*/
	"BPL","ORA","???","SLO","NOP","ORA","ASL","SLO","CLC","ORA","NOP","SLO","NOP","ORA","ASL","SLO", /*10-1F*/
	"JSR","AND","???","RLA","BIT","AND","ROL","RLA","PLP","AND","ROL","AAC","BIT","AND","ROL","RLA", /*20-2F*/
	"BMI","AND","???","RLA","NOP","AND","ROL","RLA","SEC","AND","NOP","RLA","NOP","AND","ROL","RLA", /*30-3F*/
	"RTI","EOR","???","SRE","NOP","EOR","LSR","SRE","PHA","EOR","LSR","ASR","JMP","EOR","LSR","SRE", /*40-4F*/
	"BVC","EOR","???","SRE","NOP","EOR","LSR","SRE","CLI","EOR","NOP","SRE","NOP","EOR","LSR","SRE", /*50-5F*/
	"RTS","ADC","???","RRA","NOP","ADC","ROR","RRA","PLA","ADC","ROR","???","JMP","ADC","ROR","RRA", /*60-6F*/
	"BVS","ADC","???","RRA","NOP","ADC","ROR","RRA","SEI","ADC","NOP","RRA","NOP","ADC","ROR","RRA", /*70-7F*/
	"NOP","STA","NOP","SAX","STY","STA","STX","SAX","DEY","NOP","TXA","???","STY","STA","STX","SAX", /*80-8F*/
	"BCC","STA","???","???","STY","STA","STX","SAX","TYA","STA","TXS","???","SYA","STA","SXA","???", /*90-9F*/
	"LDY","LDA","LDX","LAX","LDY","LDA","LDX","LAX","TAY","LDA","TAX","ATX","LDY","LDA","LDX","LAX", /*A0-AF*/
	"BCS","LDA","???","LAX","LDY","LDA","LDX","LAX","CLV","LDA","TSX","???","LDY","LDA","LDX","LAX", /*B0-BF*/
	"CPY","CMP","NOP","DCP","CPY","CMP","DEC","DCP","INY","CMP","DEX","AXS","CPY","CMP","DEC","DCP", /*C0-CF*/
	"BNE","CMP","???","DCP","NOP","CMP","DEC","DCP","CLD","CMP","NOP","DCP","NOP","CMP","DEC","DCP", /*D0-DF*/
	"CPX","SBC","NOP","ISB","CPX","SBC","INC","ISB","INX","SBC","NOP","SBC","CPX","SBC","INC","ISB", /*E0-EF*/
	"BEQ","SBC","???","ISB","NOP","SBC","INC","ISB","SED","SBC","NOP","ISB","NOP","SBC","INC","ISB"  /*F0-FF*/
};
static u8 addrtable[256] = {
/* x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xA xB xC xD xE xF         */
   no,ix,er,ix,zp,zp,zp,zp,no,im,ac,im,ab,ab,ab,ab, /*00-0f*/
   re,iy,er,iy,zx,zx,zx,zx,no,ay,im,ay,ax,ax,ax,ax, /*10-1f*/
   ab,ix,er,ix,zp,zp,zp,zp,no,im,ac,im,ab,ab,ab,ab, /*20-2f*/
   re,iy,er,iy,zx,zx,zx,zx,no,ay,im,ay,ax,ax,ax,ax, /*30-3f*/
   no,ix,er,ix,zp,zp,zp,zp,no,im,ac,im,ab,ab,ab,ab, /*40-4f*/
   re,iy,er,iy,zx,zx,zx,zx,no,ay,im,ay,ax,ax,ax,ax, /*50-5f*/
   no,ix,er,ix,zp,zp,zp,zp,no,im,ac,er,in,ab,ab,ab, /*60-6f*/
   re,iy,er,iy,zx,zx,zx,zx,no,ay,im,ay,ax,ax,ax,ax, /*70-7f*/
 	im,ix,im,ix,zp,zp,zp,zp,no,im,no,er,ab,ab,ab,ab, /*80-8f*/
   re,iy,er,er,zx,zx,zy,zy,no,ay,no,er,ax,ax,ay,er, /*90-9f*/
   im,ix,im,ix,zp,zp,zp,zp,no,im,no,im,ab,ab,ab,ab, /*a0-af*/
   re,iy,er,iy,zx,zx,zy,zy,no,ay,no,er,ax,ax,ay,ay, /*b0-bf*/
   im,ix,im,ix,zp,zp,zp,zp,no,im,no,im,ab,ab,ab,ab, /*c0-cf*/
   re,iy,iy,er,zx,zx,zx,zx,no,ay,im,ay,ax,ax,ax,ax, /*d0-df*/
   im,ix,im,ix,zp,zp,zp,zp,no,im,no,im,ab,ab,ab,ab, /*e0-ef*/
   re,iy,er,iy,zx,zx,zx,zx,no,ay,im,ay,ax,ax,ax,ax  /*f0-ff*/
};
#else
static char opcodes[256][4] = {
 	"BRK","ORA","???","???","???","ORA","ASL","???","PHP","ORA","ASL","???","???","ORA","ASL","???", /*00-0F*/
	"BPL","ORA","???","???","???","ORA","ASL","???","CLC","ORA","???","???","???","ORA","ASL","???", /*10-1F*/
	"JSR","AND","???","???","BIT","AND","ROL","???","PLP","AND","ROL","???","BIT","AND","ROL","???", /*20-2F*/
	"BMI","AND","???","???","???","AND","ROL","???","SEC","AND","???","???","???","AND","ROL","???", /*30-3F*/
	"RTI","EOR","???","???","???","EOR","LSR","???","PHA","EOR","LSR","???","JMP","EOR","LSR","???", /*40-4F*/
	"BVC","EOR","???","???","???","EOR","LSR","???","CLI","EOR","???","???","???","EOR","LSR","???", /*50-5F*/
	"RTS","ADC","???","???","???","ADC","ROR","???","PLA","ADC","ROR","???","JMP","ADC","ROR","???", /*60-6F*/
	"BVS","ADC","???","???","???","ADC","ROR","???","SEI","ADC","???","???","???","ADC","ROR","???", /*70-7F*/
	"???","STA","???","???","STY","STA","STX","???","DEY","???","TXA","???","STY","STA","STX","???", /*80-8F*/
	"BCC","STA","???","???","STY","STA","STX","???","TYA","STA","TXS","???","???","STA","???","???", /*90-9F*/
	"LDY","LDA","LDX","???","LDY","LDA","LDX","???","TAY","LDA","TAX","???","LDY","LDA","LDX","???", /*A0-AF*/
	"BCS","LDA","???","???","LDY","LDA","LDX","???","CLV","LDA","TSX","???","LDY","LDA","LDX","???", /*B0-BF*/
	"CPY","CMP","???","???","CPY","CMP","DEC","???","INY","CMP","DEX","???","CPY","CMP","DEC","???", /*C0-CF*/
	"BNE","CMP","???","???","???","CMP","DEC","???","CLD","CMP","???","???","???","CMP","DEC","???", /*D0-DF*/
	"CPX","SBC","???","???","CPX","SBC","INC","???","INX","SBC","NOP","???","CPX","SBC","INC","???", /*E0-EF*/
	"BEQ","SBC","???","???","???","SBC","INC","???","SED","SBC","???","???","???","SBC","INC","???"  /*F0-FF*/
};
static u8 addrtable[256] = {
/* x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xA xB xC xD xE xF         */
   no,ix,er,er,er,zp,zp,er,no,im,ac,er,er,ab,ab,er, /*00-0f*/
   re,iy,er,er,er,zx,zx,er,no,ay,er,er,er,ax,ax,er, /*10-1f*/
   ab,ix,er,er,zp,zp,zp,er,no,im,ac,er,ab,ab,ab,er, /*20-2f*/
   re,iy,er,er,er,zx,zx,er,no,ay,er,er,er,ax,ax,er, /*30-3f*/
   no,ix,er,er,er,zp,zp,er,no,im,ac,er,ab,ab,ab,er, /*40-4f*/
   re,iy,er,er,er,zx,zx,er,no,ay,er,er,er,ax,ax,er, /*50-5f*/
   no,ix,er,er,er,zp,zp,er,no,im,ac,er,in,ab,ab,er, /*60-6f*/
   re,iy,er,er,er,zx,zx,er,no,ay,er,er,er,ax,ax,er, /*70-7f*/
 	er,ix,er,er,zp,zp,zp,er,no,er,no,er,ab,ab,ab,er, /*80-8f*/
   re,iy,er,er,zx,zx,zy,er,no,ay,no,er,er,ax,er,er, /*90-9f*/
   im,ix,im,er,zp,zp,zp,er,no,im,no,er,ab,ab,ab,er, /*a0-af*/
   re,iy,er,er,zx,zx,zy,er,no,ay,no,er,ax,ax,ay,er, /*b0-bf*/
   im,ix,er,er,zp,zp,zp,er,no,im,no,er,ab,ab,ab,er, /*c0-cf*/
   re,iy,er,er,er,zx,zx,er,no,ay,er,er,er,ax,ax,er, /*d0-df*/
   im,ix,er,er,zp,zp,zp,er,no,im,no,er,ab,ab,ab,er, /*e0-ef*/
   re,iy,er,er,zx,zx,zx,er,no,ay,er,er,ax,ax,ax,er  /*f0-ff*/
};
#endif

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
			sprintf(buffer,"%02X %02X %02X %s $%04x,x",opcode,addr & 0xFF,(addr >> 8) & 0xFF,opcodes[opcode],addr);
			break;
		case ay:
			size = 3;
			addr = cpu_read(opcodepos+1) | (cpu_read(opcodepos+2) << 8);
			if(addr < 0x100) { /*should be zero page, so its bad opcode*/
				size = 1;
				sprintf(buffer,".u8 $%02x",opcode);
				break;
			}
			sprintf(buffer,"%02X %02X %02X %s $%04x,y",opcode,addr & 0xFF,(addr >> 8) & 0xFF,opcodes[opcode],addr);
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
