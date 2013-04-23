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

static INLINE void OP_LDA()
{
	A = memread(EFFADDR);
	checknz(A);
}

static INLINE void OP_LDX()
{
	X = memread(EFFADDR);
	checknz(X);
}

static INLINE void OP_LDY()
{
	Y = memread(EFFADDR);
	checknz(Y);
}

static INLINE void OP_STA()
{
	memwrite(EFFADDR,A);
}

static INLINE void OP_STX()
{
	memwrite(EFFADDR,X);
}

static INLINE void OP_STY()
{
	memwrite(EFFADDR,Y);
}
