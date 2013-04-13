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

//implied addressing
static INLINE void AM_IMP()
{
	TMPREG = memread(PC);
}

//immediate addressing
static INLINE void AM_IMM()
{
	EFFADDR = PC++;
}

//absolute addressing
static INLINE void AM_ABS()
{
	EFFADDR = memread(PC++);
	EFFADDR |= memread(PC++) << 8;
}

/*
Read instructions (LDA, LDX, LDY, EOR, AND, ORA, ADC, SBC, CMP, BIT, LAX, LAE, SHS, NOP)
#   address  R/W description
--- --------- --- ------------------------------------------
1     PC      R  fetch opcode, increment PC
2     PC      R  fetch low byte of address, increment PC
3     PC      R  fetch high byte of address, add index register to low address byte, increment PC
4  address+I* R  read from effective address, fix the high byte of effective address
5+ address+I  R  re-read from effective address
Notes: I denotes either index register (X or Y). 
* The high byte of the effective address may be invalid at this time, i.e. it may be smaller by $100.
+ This cycle will be executed only if the effective address was invalid during cycle #4, i.e. page boundary was crossed.
*/
//absolute x addressing (for reading only)
static INLINE void AM_AXR()
{
	EFFADDR = memread(PC++);
	EFFADDR |= memread(PC++) << 8;
	tmpi = (EFFADDR & 0xFF) + X;
	if(tmpi >= 0x100) {
		memread((EFFADDR & 0xFF00) | (u8)tmpi);
	}
	EFFADDR += X;
}

//absolute x addressing
static INLINE void AM_ABX()
{
	EFFADDR = memread(PC++);
	EFFADDR |= memread(PC++) << 8;
	tmpi = (EFFADDR & 0xFF) + X;
	memread((EFFADDR & 0xFF00) | (u8)tmpi);
	EFFADDR += X;
}

//absolute y addressing (for reading only)
static INLINE void AM_AYR()
{
	EFFADDR = memread(PC++);
	EFFADDR |= memread(PC++) << 8;
	tmpi = (EFFADDR & 0xFF) + Y;
	if(tmpi >= 0x100) {
		memread((EFFADDR & 0xFF00) | (u8)tmpi);
	}
	EFFADDR += Y;
}

//absolute y addressing
static INLINE void AM_ABY()
{
	EFFADDR = memread(PC++);
	EFFADDR |= memread(PC++) << 8;
	tmpi = (EFFADDR & 0xFF) + Y;
	memread((EFFADDR & 0xFF00) | (u8)tmpi);
	EFFADDR += Y;
}

//zero-page addressing
static INLINE void AM_ZPG()
{
	EFFADDR = memread(PC++);
}

//zero-page x addressing
static INLINE void AM_ZPX()
{
	EFFADDR = memread(PC++);
	memread(EFFADDR);
	EFFADDR = (EFFADDR + X) & 0xFF;
}

//zero-page y addressing
static INLINE void AM_ZPY()
{
	EFFADDR = memread(PC++);
	memread(EFFADDR);
	EFFADDR = (EFFADDR + Y) & 0xFF;
}

/*
Write instructions (STA, SAX)
#    address   R/W description
--- ----------- --- ------------------------------------------
1      PC       R  fetch opcode, increment PC
2      PC       R  fetch pointer address, increment PC
3    pointer    R  read from the address, add X to it
4   pointer+X   R  fetch effective address low
5  pointer+X+1  R  fetch effective address high
6    address    W  write to effective address
Note: The effective address is always fetched from zero page, i.e. the zero page boundary crossing is not handled.
*/
//indirect x
static INLINE void AM_INX()
{
	TMPREG = memread(PC++);
	memread(TMPREG);
	TMPREG += X;
	EFFADDR = memread(TMPREG++);
	EFFADDR |= memread(TMPREG) << 8;
}

/*
Read instructions (LDA, EOR, AND, ORA, ADC, SBC, CMP)
#    address   R/W description
--- ----------- --- ------------------------------------------
1      PC       R  fetch opcode, increment PC
2      PC       R  fetch pointer address, increment PC
3    pointer    R  fetch effective address low
4   pointer+1   R  fetch effective address high, add Y to low byte of effective address
5   address+Y*  R  read from effective address, fix high byte of effective address
6+  address+Y   R  read from effective address
Notes: The effective address is always fetched from zero page, i.e. the zero page boundary crossing is not handled.
* The high byte of the effective address may be invalid at this time, i.e. it may be smaller by $100.
+ This cycle will be executed only if the effective address was invalid during cycle #5, i.e. page boundary was crossed.
*/
//indirect y (for reading only)
static INLINE void AM_IYR()
{
	TMPREG = memread(PC++);
	EFFADDR = memread(TMPREG++);
	EFFADDR |= memread(TMPREG) << 8;
	tmpi = (EFFADDR & 0xFF) + Y;
	if(tmpi >= 0x100) {
		memread((EFFADDR & 0xFF00) | (u8)tmpi);
	}
	EFFADDR += Y;
}

/*
Write instructions (STA, SHA)
#    address   R/W description
--- ----------- --- ------------------------------------------
1      PC       R  fetch opcode, increment PC
2      PC       R  fetch pointer address, increment PC
3    pointer    R  fetch effective address low
4   pointer+1   R  fetch effective address high, add Y to low byte of effective address
5   address+Y*  R  read from effective address, fix high byte of effective address
6   address+Y   W  write to effective address
Notes: The effective address is always fetched from zero page, i.e. the zero page boundary crossing is not handled.
* The high byte of the effective address may be invalid at this time, i.e. it may be smaller by $100.
*/
//indirect y
static INLINE void AM_INY()
{
	TMPREG = memread(PC++);
	EFFADDR = memread(TMPREG++);
	EFFADDR |= memread(TMPREG) << 8;
	tmpi = (EFFADDR & 0xFF) + Y;
	memread((EFFADDR & 0xFF00) | (u8)tmpi);
	EFFADDR += Y;
}

//indirect addressing
static INLINE void AM_IND()
{
	TMPADDR = memread(PC++);
	TMPADDR |= memread(PC++) << 8;
	EFFADDR = memread(TMPADDR);
	TMPADDR = (TMPADDR & 0xFF00) | ((TMPADDR + 1) & 0xFF);
	EFFADDR |= memread(TMPADDR) << 8;
}

//relative addressing
static INLINE void AM_REL()
{
	TMPREG = memread(PC++);
}

//unknown addressing (for bad opcodes, and jsr)
static INLINE void AM_UNK()
{
}
