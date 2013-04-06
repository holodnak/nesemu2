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

/*
Relative addressing (BCC, BCS, BNE, BEQ, BPL, BMI, BVC, BVS)
#   address  R/W description
--- --------- --- ---------------------------------------------
1     PC      R  fetch opcode, increment PC
2     PC      R  fetch operand, increment PC
3     PC      R  Fetch opcode of next instruction. If branch is taken, add operand to PCL. Otherwise increment PC.
4+    PC*     R  Fetch opcode of next instruction. Fix PCH. If it did not change, increment PC.
5!    PC      R  Fetch opcode of next instruction, increment PC.
Notes: The opcode fetch of the next instruction is included to this diagram for illustration
  purposes. When determining real execution times, remember to subtract the last cycle.
* The high byte of Program Counter (PCH) may be invalid at this time, i.e. it may be smaller or bigger by $100.
+ If branch is taken, this cycle will be executed.
! If branch occurs to different page, this cycle will be executed.
*/
static INLINE void BRANCH(int n)
{
	if(n == 0)
		return;
	memread(PC);
	TMPADDR = PC + (s8)TMPREG;
	if((TMPADDR ^ PC) & 0xFF00) {
		memread((PC & 0xFF00) | (TMPADDR & 0xFF));
	}
	PC = TMPADDR;
}

static INLINE void OP_BMI()
{
	BRANCH(FLAG_N);
}

static INLINE void OP_BPL()
{
	BRANCH((~FLAG_N) & 1);
}

static INLINE void OP_BVS()
{
	BRANCH(FLAG_V);
}

static INLINE void OP_BVC()
{
	BRANCH((~FLAG_V) & 1);
}

static INLINE void OP_BCS()
{
	BRANCH(FLAG_C);
}

static INLINE void OP_BCC()
{
	BRANCH((~FLAG_C) & 1);
}

static INLINE void OP_BEQ()
{
	BRANCH(FLAG_Z);
}

static INLINE void OP_BNE()
{
	BRANCH((~FLAG_Z) & 1);
}
