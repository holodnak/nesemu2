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

#define OP(n,o,a)			\
	case 0x##n:				\
		AM_##a();			\
		OP_##o();			\
		break;
/*
off- ++++++++++ Positive ++++++++++  ---------- Negative ----------
set  00      20      40      60      80      a0      c0      e0      mode
+00  BRK     JSR     RTI     RTS     NOP*    LDY     CPY     CPX     Impl/immed
+01  ORA     AND     EOR     ADC     STA     LDA     CMP     SBC     (indir,x)
+02   t       t       t       t      NOP*t   LDX     NOP*t   NOP*t     ? /immed
+03  SLO*    RLA*    SRE*    RRA*    SAX*    LAX*    DCP*    ISB*    (indir,x)
+04  NOP*    BIT     NOP*    NOP*    STY     LDY     CPY     CPX     Zeropage
+05  ORA     AND     EOR     ADC     STA     LDA     CMP     SBC     Zeropage
+06  ASL     ROL     LSR     ROR     STX     LDX     DEC     INC     Zeropage
+07  SLO*    RLA*    SRE*    RRA*    SAX*    LAX*    DCP*    ISB*    Zeropage
+08  PHP     PLP     PHA     PLA     DEY     TAY     INY     INX     Implied
+09  ORA     AND     EOR     ADC     NOP*    LDA     CMP     SBC     Immediate
+0a  ASL     ROL     LSR     ROR     TXA     TAX     DEX     NOP     Accu/impl
+0b  ANC**   ANC**   ASR**   ARR**   ANE**   LXA**   SBX**   SBC*    Immediate
+0c  NOP*    BIT     JMP     JMP ()  STY     LDY     CPY     CPX     Absolute
+0d  ORA     AND     EOR     ADC     STA     LDA     CMP     SBC     Absolute
+0e  ASL     ROL     LSR     ROR     STX     LDX     DEC     INC     Absolute
+0f  SLO*    RLA*    SRE*    RRA*    SAX*    LAX*    DCP*    ISB*    Absolute
+10  BPL     BMI     BVC     BVS     BCC     BCS     BNE     BEQ     Relative
+11  ORA     AND     EOR     ADC     STA     LDA     CMP     SBC     (indir),y
+12   t       t       t       t       t       t       t       t         ?
+13  SLO*    RLA*    SRE*    RRA*    SHA**   LAX*    DCP*    ISB*    (indir),y
+14  NOP*    NOP*    NOP*    NOP*    STY     LDY     NOP*    NOP*    Zeropage,x
+15  ORA     AND     EOR     ADC     STA     LDA     CMP     SBC     Zeropage,x
+16  ASL     ROL     LSR     ROR     STX  y) LDX  y) DEC     INC     Zeropage,x
+17  SLO*    RLA*    SRE*    RRA*    SAX* y) LAX* y) DCP*    ISB*    Zeropage,x
+18  CLC     SEC     CLI     SEI     TYA     CLV     CLD     SED     Implied
+19  ORA     AND     EOR     ADC     STA     LDA     CMP     SBC     Absolute,y
+1a  NOP*    NOP*    NOP*    NOP*    TXS     TSX     NOP*    NOP*    Implied
+1b  SLO*    RLA*    SRE*    RRA*    SHS**   LAS**   DCP*    ISB*    Absolute,y
+1c  NOP*    NOP*    NOP*    NOP*    SHY**   LDY     NOP*    NOP*    Absolute,x
+1d  ORA     AND     EOR     ADC     STA     LDA     CMP     SBC     Absolute,x
+1e  ASL     ROL     LSR     ROR     SHX**y) LDX  y) DEC     INC     Absolute,x
+1f  SLO*    RLA*    SRE*    RRA*    SHA**y) LAX* y) DCP*    ISB*    Absolute,x
*/

static INLINE void cpu_step()
{
	OPADDR = PC;
	OPCODE = memread(PC++);
	switch(nes.cpu.opcode) {

#ifdef CPU_UNDOC
		OP(00, BRK,IMM) OP(01, ORA,INX) OP(02, UNK,UNK) OP(03, SLO,INX) OP(04,NOPR,ZPG) OP(05, ORA,ZPG) OP(06, ASL,ZPG) OP(07, SLO,ZPG)
		OP(08, PHP,IMP) OP(09, ORA,IMM) OP(0A,ASLA,IMP) OP(0B, AAC,IMM) OP(0C,NOPR,ABS) OP(0D, ORA,ABS) OP(0E, ASL,ABS) OP(0F, SLO,ABS)
		OP(10, BPL,REL) OP(11, ORA,IYR) OP(12, UNK,UNK) OP(13, SLO,INY) OP(14,NOPR,ZPX) OP(15, ORA,ZPX) OP(16, ASL,ZPX) OP(17, SLO,ZPX)
		OP(18, CLC,IMP) OP(19, ORA,AYR) OP(1A, NOP,IMP) OP(1B, SLO,ABY) OP(1C,NOPR,AXR) OP(1D, ORA,AXR) OP(1E, ASL,ABX) OP(1F, SLO,ABX)
		OP(20, JSR,IMP) OP(21, AND,INX) OP(22, UNK,UNK) OP(23, RLA,INX) OP(24, BIT,ZPG) OP(25, AND,ZPG) OP(26, ROL,ZPG) OP(27, RLA,ZPG)
		OP(28, PLP,IMP) OP(29, AND,IMM) OP(2A,ROLA,IMP) OP(2B, AAC,IMM) OP(2C, BIT,ABS) OP(2D, AND,ABS) OP(2E, ROL,ABS) OP(2F, RLA,ABS)
		OP(30, BMI,REL) OP(31, AND,IYR) OP(32, UNK,UNK) OP(33, RLA,INY) OP(34,NOPR,ZPX) OP(35, AND,ZPX) OP(36, ROL,ZPX) OP(37, RLA,ZPX)
		OP(38, SEC,IMP) OP(39, AND,AYR) OP(3A, NOP,IMP) OP(3B, RLA,ABY) OP(3C,NOPR,AXR) OP(3D, AND,AXR) OP(3E, ROL,ABX) OP(3F, RLA,ABX)
		OP(40, RTI,IMP) OP(41, EOR,INX) OP(42, UNK,UNK) OP(43, SRE,INX) OP(44,NOPR,ZPG) OP(45, EOR,ZPG) OP(46, LSR,ZPG) OP(47, SRE,ZPG)
		OP(48, PHA,IMP) OP(49, EOR,IMM) OP(4A,LSRA,IMP) OP(4B, ASR,IMM) OP(4C, JMP,ABS) OP(4D, EOR,ABS) OP(4E, LSR,ABS) OP(4F, SRE,ABS)
		OP(50, BVC,REL) OP(51, EOR,IYR) OP(52, UNK,UNK) OP(53, SRE,INY) OP(54,NOPR,ZPX) OP(55, EOR,ZPX) OP(56, LSR,ZPX) OP(57, SRE,ZPX)
		OP(58, CLI,IMP) OP(59, EOR,AYR) OP(5A, NOP,IMP) OP(5B, SRE,ABY) OP(5C,NOPR,AXR) OP(5D, EOR,AXR) OP(5E, LSR,ABX) OP(5F, SRE,ABX)
		OP(60, RTS,IMP) OP(61, ADC,INX) OP(62, UNK,UNK) OP(63, RRA,INX) OP(64,NOPR,ZPG) OP(65, ADC,ZPG) OP(66, ROR,ZPG) OP(67, RRA,ZPG)
		OP(68, PLA,IMP) OP(69, ADC,IMM) OP(6A,RORA,IMP) OP(6B, ARR,IMM) OP(6C, JMP,IND) OP(6D, ADC,ABS) OP(6E, ROR,ABS) OP(6F, RRA,ABS)
		OP(70, BVS,REL) OP(71, ADC,IYR) OP(72, UNK,UNK) OP(73, RRA,INY) OP(74,NOPR,ZPX) OP(75, ADC,ZPX) OP(76, ROR,ZPX) OP(77, RRA,ZPX)
		OP(78, SEI,IMP) OP(79, ADC,AYR) OP(7A, NOP,IMP) OP(7B, RRA,ABY) OP(7C,NOPR,AXR) OP(7D, ADC,AXR) OP(7E, ROR,ABX) OP(7F, RRA,ABX)
		OP(80,NOPR,IMM) OP(81, STA,INX) OP(82, NOP,IMM) OP(83, SAX,INX) OP(84, STY,ZPG) OP(85, STA,ZPG) OP(86, STX,ZPG) OP(87, SAX,ZPG)
		OP(88, DEY,IMP) OP(89, NOP,IMM) OP(8A, TXA,IMP) OP(8B, UNK,UNK) OP(8C, STY,ABS) OP(8D, STA,ABS) OP(8E, STX,ABS) OP(8F, SAX,ABS)
		OP(90, BCC,REL) OP(91, STA,INY) OP(92, UNK,UNK) OP(93, UNK,UNK) OP(94, STY,ZPX) OP(95, STA,ZPX) OP(96, STX,ZPY) OP(97, SAX,ZPY)
		OP(98, TYA,IMP) OP(99, STA,ABY) OP(9A, TXS,IMP) OP(9B, UNK,UNK) OP(9C, SYA,ABX) OP(9D, STA,ABX) OP(9E, SXA,ABY) OP(9F, UNK,UNK)
		OP(A0, LDY,IMM) OP(A1, LDA,INX) OP(A2, LDX,IMM) OP(A3, LAX,INX) OP(A4, LDY,ZPG) OP(A5, LDA,ZPG) OP(A6, LDX,ZPG) OP(A7, LAX,ZPG)
		OP(A8, TAY,IMP) OP(A9, LDA,IMM) OP(AA, TAX,IMP) OP(AB, ATX,IMM) OP(AC, LDY,ABS) OP(AD, LDA,ABS) OP(AE, LDX,ABS) OP(AF, LAX,ABS)
		OP(B0, BCS,REL) OP(B1, LDA,IYR) OP(B2, UNK,UNK) OP(B3, LAX,IYR) OP(B4, LDY,ZPX) OP(B5, LDA,ZPX) OP(B6, LDX,ZPY) OP(B7, LAX,ZPY)
		OP(B8, CLV,IMP) OP(B9, LDA,AYR) OP(BA, TSX,IMP) OP(BB, UNK,UNK) OP(BC, LDY,AXR) OP(BD, LDA,AXR) OP(BE, LDX,AYR) OP(BF, LAX,AYR)
		OP(C0, CPY,IMM) OP(C1, CMP,INX) OP(C2, NOP,IMM) OP(C3, DCP,INX) OP(C4, CPY,ZPG) OP(C5, CMP,ZPG) OP(C6, DEC,ZPG) OP(C7, DCP,ZPG)
		OP(C8, INY,IMP) OP(C9, CMP,IMM) OP(CA, DEX,IMP) OP(CB, AXS,IMM) OP(CC, CPY,ABS) OP(CD, CMP,ABS) OP(CE, DEC,ABS) OP(CF, DCP,ABS)
		OP(D0, BNE,REL) OP(D1, CMP,IYR) OP(D2, UNK,UNK) OP(D3, DCP,INY) OP(D4,NOPR,ZPX) OP(D5, CMP,ZPX) OP(D6, DEC,ZPX) OP(D7, DCP,ZPX)
		OP(D8, CLD,IMP) OP(D9, CMP,AYR) OP(DA, NOP,IMP) OP(DB, DCP,ABY) OP(DC,NOPR,AXR) OP(DD, CMP,AXR) OP(DE, DEC,ABX) OP(DF, DCP,ABX)
		OP(E0, CPX,IMM) OP(E1, SBC,INX) OP(E2, NOP,IMM) OP(E3, ISB,INX) OP(E4, CPX,ZPG) OP(E5, SBC,ZPG) OP(E6, INC,ZPG) OP(E7, ISB,ZPG)
		OP(E8, INX,IMP) OP(E9, SBC,IMM) OP(EA, NOP,IMP) OP(EB, SBC,IMM) OP(EC, CPX,ABS) OP(ED, SBC,ABS) OP(EE, INC,ABS) OP(EF, ISB,ABS)
		OP(F0, BEQ,REL) OP(F1, SBC,IYR) OP(F2, UNK,UNK) OP(F3, ISB,INY) OP(F4,NOPR,ZPX) OP(F5, SBC,ZPX) OP(F6, INC,ZPX) OP(F7, ISB,ZPX)
		OP(F8, SED,IMP) OP(F9, SBC,AYR) OP(FA, NOP,IMP) OP(FB, ISB,ABY) OP(FC,NOPR,AXR) OP(FD, SBC,AXR) OP(FE, INC,ABX) OP(FF, ISB,ABX)
#else
		OP(00, BRK,IMP) OP(01, ORA,INX) OP(02, UNK,UNK) OP(03, UNK,UNK) OP(04, UNK,UNK) OP(05, ORA,ZPG) OP(06, ASL,ZPG) OP(07, UNK,UNK)
		OP(08, PHP,IMP) OP(09, ORA,IMM) OP(0A,ASLA,IMP) OP(0B, UNK,UNK) OP(0C, UNK,UNK) OP(0D, ORA,ABS) OP(0E, ASL,ABS) OP(0F, UNK,UNK)
		OP(10, BPL,REL) OP(11, ORA,IYR) OP(12, UNK,UNK) OP(13, UNK,UNK) OP(14, UNK,UNK) OP(15, ORA,ZPX) OP(16, ASL,ZPX) OP(17, UNK,UNK)
		OP(18, CLC,IMP) OP(19, ORA,AYR) OP(1A, UNK,UNK) OP(1B, UNK,UNK) OP(1C, UNK,UNK) OP(1D, ORA,AXR) OP(1E, ASL,ABX) OP(1F, UNK,UNK)
		OP(20, JSR,IMP) OP(21, AND,INX) OP(22, UNK,UNK) OP(23, UNK,UNK) OP(24, BIT,ZPG) OP(25, AND,ZPG) OP(26, ROL,ZPG) OP(27, UNK,UNK)
		OP(28, PLP,IMP) OP(29, AND,IMM) OP(2A,ROLA,IMP) OP(2B, UNK,UNK) OP(2C, BIT,ABS) OP(2D, AND,ABS) OP(2E, ROL,ABS) OP(2F, UNK,UNK)
		OP(30, BMI,REL) OP(31, AND,IYR) OP(32, UNK,UNK) OP(33, UNK,UNK) OP(34, UNK,UNK) OP(35, AND,ZPX) OP(36, ROL,ZPX) OP(37, UNK,UNK)
		OP(38, SEC,IMP) OP(39, AND,AYR) OP(3A, UNK,UNK) OP(3B, UNK,UNK) OP(3C, UNK,UNK) OP(3D, AND,AXR) OP(3E, ROL,ABX) OP(3F, UNK,UNK)
		OP(40, RTI,IMP) OP(41, EOR,INX) OP(42, UNK,UNK) OP(43, UNK,UNK) OP(44, UNK,UNK) OP(45, EOR,ZPG) OP(46, LSR,ZPG) OP(47, UNK,UNK)
		OP(48, PHA,IMP) OP(49, EOR,IMM) OP(4A,LSRA,IMP) OP(4B, UNK,UNK) OP(4C, JMP,ABS) OP(4D, EOR,ABS) OP(4E, LSR,ABS) OP(4F, UNK,UNK)
		OP(50, BVC,REL) OP(51, EOR,IYR) OP(52, UNK,UNK) OP(53, UNK,UNK) OP(54, UNK,UNK) OP(55, EOR,ZPX) OP(56, LSR,ZPX) OP(57, UNK,UNK)
		OP(58, CLI,IMP) OP(59, EOR,AYR) OP(5A, UNK,UNK) OP(5B, UNK,UNK) OP(5C, UNK,UNK) OP(5D, EOR,AXR) OP(5E, LSR,ABX) OP(5F, UNK,UNK)
		OP(60, RTS,IMP) OP(61, ADC,INX) OP(62, UNK,UNK) OP(63, UNK,UNK) OP(64, UNK,UNK) OP(65, ADC,ZPG) OP(66, ROR,ZPG) OP(67, UNK,UNK)
		OP(68, PLA,IMP) OP(69, ADC,IMM) OP(6A,RORA,IMP) OP(6B, UNK,UNK) OP(6C, JMP,IND) OP(6D, ADC,ABS) OP(6E, ROR,ABS) OP(6F, UNK,UNK)
		OP(70, BVS,REL) OP(71, ADC,IYR) OP(72, UNK,UNK) OP(73, UNK,UNK) OP(74, UNK,UNK) OP(75, ADC,ZPX) OP(76, ROR,ZPX) OP(77, UNK,UNK)
		OP(78, SEI,IMP) OP(79, ADC,AYR) OP(7A, UNK,UNK) OP(7B, UNK,UNK) OP(7C, UNK,UNK) OP(7D, ADC,AXR) OP(7E, ROR,ABX) OP(7F, UNK,UNK)
		OP(80, UNK,UNK) OP(81, STA,INX) OP(82, UNK,UNK) OP(83, UNK,UNK) OP(84, STY,ZPG) OP(85, STA,ZPG) OP(86, STX,ZPG) OP(87, UNK,UNK)
		OP(88, DEY,IMP) OP(89, UNK,UNK) OP(8A, TXA,IMP) OP(8B, UNK,UNK) OP(8C, STY,ABS) OP(8D, STA,ABS) OP(8E, STX,ABS) OP(8F, UNK,UNK)
		OP(90, BCC,REL) OP(91, STA,INY) OP(92, UNK,UNK) OP(93, UNK,UNK) OP(94, STY,ZPX) OP(95, STA,ZPX) OP(96, STX,ZPY) OP(97, UNK,UNK)
		OP(98, TYA,IMP) OP(99, STA,ABY) OP(9A, TXS,IMP) OP(9B, UNK,UNK) OP(9C, UNK,UNK) OP(9D, STA,ABX) OP(9E, UNK,UNK) OP(9F, UNK,UNK)
		OP(A0, LDY,IMM) OP(A1, LDA,INX) OP(A2, LDX,IMM) OP(A3, UNK,UNK) OP(A4, LDY,ZPG) OP(A5, LDA,ZPG) OP(A6, LDX,ZPG) OP(A7, UNK,UNK)
		OP(A8, TAY,IMP) OP(A9, LDA,IMM) OP(AA, TAX,IMP) OP(AB, UNK,UNK) OP(AC, LDY,ABS) OP(AD, LDA,ABS) OP(AE, LDX,ABS) OP(AF, UNK,UNK)
		OP(B0, BCS,REL) OP(B1, LDA,IYR) OP(B2, UNK,UNK) OP(B3, UNK,UNK) OP(B4, LDY,ZPX) OP(B5, LDA,ZPX) OP(B6, LDX,ZPY) OP(B7, UNK,UNK)
		OP(B8, CLV,IMP) OP(B9, LDA,AYR) OP(BA, TSX,IMP) OP(BB, UNK,UNK) OP(BC, LDY,AXR) OP(BD, LDA,AXR) OP(BE, LDX,AYR) OP(BF, UNK,UNK)
		OP(C0, CPY,IMM) OP(C1, CMP,INX) OP(C2, UNK,UNK) OP(C3, UNK,UNK) OP(C4, CPY,ZPG) OP(C5, CMP,ZPG) OP(C6, DEC,ZPG) OP(C7, UNK,UNK)
		OP(C8, INY,IMP) OP(C9, CMP,IMM) OP(CA, DEX,IMP) OP(CB, UNK,UNK) OP(CC, CPY,ABS) OP(CD, CMP,ABS) OP(CE, DEC,ABS) OP(CF, UNK,UNK)
		OP(D0, BNE,REL) OP(D1, CMP,IYR) OP(D2, UNK,UNK) OP(D3, UNK,UNK) OP(D4, UNK,UNK) OP(D5, CMP,ZPX) OP(D6, DEC,ZPX) OP(D7, UNK,UNK)
		OP(D8, CLD,IMP) OP(D9, CMP,AYR) OP(DA, UNK,UNK) OP(DB, UNK,UNK) OP(DC, UNK,UNK) OP(DD, CMP,AXR) OP(DE, DEC,ABX) OP(DF, UNK,UNK)
		OP(E0, CPX,IMM) OP(E1, SBC,INX) OP(E2, UNK,UNK) OP(E3, UNK,UNK) OP(E4, CPX,ZPG) OP(E5, SBC,ZPG) OP(E6, INC,ZPG) OP(E7, UNK,UNK)
		OP(E8, INX,IMP) OP(E9, SBC,IMM) OP(EA, NOP,IMP) OP(EB, UNK,UNK) OP(EC, CPX,ABS) OP(ED, SBC,ABS) OP(EE, INC,ABS) OP(EF, UNK,UNK)
		OP(F0, BEQ,REL) OP(F1, SBC,IYR) OP(F2, UNK,UNK) OP(F3, UNK,UNK) OP(F4, UNK,UNK) OP(F5, SBC,ZPX) OP(F6, INC,ZPX) OP(F7, UNK,UNK)
		OP(F8, SED,IMP) OP(F9, SBC,AYR) OP(FA, UNK,UNK) OP(FB, UNK,UNK) OP(FC, UNK,UNK) OP(FD, SBC,AXR) OP(FE, INC,ABX) OP(FF, UNK,UNK)
#endif
		//should never get here
		default:
			log_printf("cpu_execute:  bug:  bad opcode $%02X at $%04X\n",OPCODE,OPADDR);
			break;
	}
	if(PREV_NMISTATE) {
		NMISTATE = 0;
		execute_nmi();
	}
	else if(PREV_IRQSTATE && (FLAG_I == 0)) {
		execute_irq();
	}
}

u32 cpu_execute(u32 cycles)
{
	u64 start = CYCLES;
	u64 stop = CYCLES + cycles;

	while(CYCLES < stop) {
		cpu_step();
	}
	return((u32)(CYCLES - start));
}

void cpu_execute_frame()
{
	u32 curframe = FRAMES;

	while(FRAMES == curframe) {
		cpu_step();
	}
}
