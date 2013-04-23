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

static INLINE void OP_ORA()
{
	A |= memread(EFFADDR);
	checknz(A);
}

static INLINE void OP_AND()
{
	A &= memread(EFFADDR);
	checknz(A);
}

static INLINE void OP_EOR()
{
	A ^= memread(EFFADDR);
	checknz(A);
}

static INLINE void OP_ASL()
{
	TMPREG = memread(EFFADDR);
	memwrite(EFFADDR,TMPREG);
	FLAG_C = (TMPREG >> 7) & 1;
	TMPREG <<= 1;
	memwrite(EFFADDR,TMPREG);
	checknz(TMPREG);
}

static INLINE void OP_ASLA()
{
	FLAG_C = (A >> 7) & 1;
	A <<= 1;
	checknz(A);
}

static INLINE void OP_ROL()
{
	TMPREG = memread(EFFADDR);
	memwrite(EFFADDR,TMPREG);
	tmp8 = FLAG_C;
	FLAG_C = (TMPREG >> 7) & 1;
	TMPREG = (TMPREG << 1) | tmp8;
	memwrite(EFFADDR,TMPREG);
	checknz(TMPREG);
}

static INLINE void OP_ROLA()
{
	TMPREG = FLAG_C;
	FLAG_C = (A >> 7) & 1;
	A = (A << 1) | TMPREG;
	checknz(A);
}

static INLINE void OP_ROR()
{
	TMPREG = memread(EFFADDR);
	memwrite(EFFADDR,TMPREG);
	tmp8 = FLAG_C;
	FLAG_C = TMPREG & 1;
	TMPREG = (TMPREG >> 1) | (tmp8 << 7);
	memwrite(EFFADDR,TMPREG);
	checknz(TMPREG);
}

static INLINE void OP_RORA()
{
	TMPREG = FLAG_C;
	FLAG_C = A & 1;
	A = (A >> 1) | (TMPREG << 7);
	checknz(A);
}

static INLINE void OP_LSR()
{
	TMPREG = memread(EFFADDR);
	memwrite(EFFADDR,TMPREG);
	FLAG_C = TMPREG & 1;
	TMPREG >>= 1;
	memwrite(EFFADDR,TMPREG);
	checknz(TMPREG);
}

static INLINE void OP_LSRA()
{
	FLAG_C = A & 1;
	A >>= 1;
	checknz(A);
}

static INLINE void OP_ADC()
{
	TMPREG = memread(EFFADDR);
	tmpi = A + TMPREG + FLAG_C;
	FLAG_C = (tmpi & 0xFF00) ? 1 : 0;
	FLAG_V = (((A ^ tmpi) & (TMPREG ^ tmpi)) & 0x80) ? 1 : 0;
	A = (u8)tmpi;
	checknz(A);
}

static INLINE void OP_SBC()
{
	TMPREG = memread(EFFADDR);
	tmpi = A - TMPREG - (1 - FLAG_C);
	FLAG_C = ((tmpi & 0xFF00) == 0) ? 1 : 0;
	FLAG_V = (((A ^ TMPREG) & (A ^ tmpi)) & 0x80) ? 1 : 0;
	A = (u8)tmpi;
	checknz(A);
}
