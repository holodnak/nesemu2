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

static INLINE void OP_BIT()
{
	TMPREG = memread(EFFADDR);
	FLAG_V = (TMPREG >> 6) & 1;
	FLAG_N = (TMPREG >> 7) & 1;
	FLAG_Z = (A & TMPREG) ? 0 : 1;
}

static INLINE void OP_CMP()
{
	tmpi = A - memread(EFFADDR);
	FLAG_C = (tmpi >= 0) ? 1 : 0;
	checknz((u8)tmpi);
}

static INLINE void OP_CPX()
{
	tmpi = X - memread(EFFADDR);
	FLAG_C = (tmpi >= 0) ? 1 : 0;
	checknz((u8)tmpi);
}

static INLINE void OP_CPY()
{
	tmpi = Y - memread(EFFADDR);
	FLAG_C = (tmpi >= 0) ? 1 : 0;
	checknz((u8)tmpi);
}
