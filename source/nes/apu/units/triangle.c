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

#include "nes/nes.h"

#define tri	nes.apu.triangle

static s8 TriangleDuty[32] = {
	+7,+6,+5,+4,+3,+2,+1,+0,
	-1,-2,-3,-4,-5,-6,-7,-8,
	-8,-7,-6,-5,-4,-3,-2,-1,
	+0,+1,+2,+3,+4,+5,+6,+7,
};

static INLINE void checkactive()
{
	tri.Active = tri.LengthCtr && tri.LinCtr;
	if (tri.freq < 4)
		tri.Pos = 0;	// beyond hearing range
	else	tri.Pos = TriangleDuty[tri.CurD] * 8;
}

void apu_triangle_reset(int hard)
{
	tri.linear = tri.wavehold = 0;
	tri.freq = 0;
	tri.CurD = 0;
	tri.LengthCtr = tri.LinCtr = 0;
	tri.Enabled = tri.Active = 0;
	tri.LinClk = 0;
	tri.Pos = 0;
	tri.Cycles = 1;
}

void apu_triangle_write(u32 addr,u8 data)
{
	switch (addr)
	{
	case 0:	tri.linear = data & 0x7F;
//		Race::Triangle_wavehold = (data >> 7) & 0x1;
		break;
	case 2:	tri.freq &= 0x700;
		tri.freq |= data;
		break;
	case 3:	tri.freq &= 0xFF;
		tri.freq |= (data & 0x7) << 8;
		if (tri.Enabled)
		{
//			Race::Triangle_LengthCtr1 = LengthCounts[(Val >> 3) & 0x1F];
//			Race::Triangle_LengthCtr2 = LengthCtr;
		}
		tri.LinClk = 1;
		break;
	case 4:	tri.Enabled = data ? 1 : 0;
		if (!tri.Enabled)
			tri.LengthCtr = 0;
		break;
	}
	checkactive();
}

void apu_triangle_step()
{
	if (!tri.Cycles--)
	{
		tri.Cycles = tri.freq;
		if (tri.Active)
		{
			tri.CurD++;
			tri.CurD &= 0x1F;
			if (tri.freq < 4)
				tri.Pos = 0;	// beyond hearing range
			else	tri.Pos = TriangleDuty[tri.CurD] * 8;
		}
	}
}

void apu_triangle_quarter()
{
	if (tri.LinClk)
		tri.LinCtr = tri.linear;
	else	if (tri.LinCtr)
		tri.LinCtr--;
	if (!tri.wavehold)
		tri.LinClk = 0;
	checkactive();
}

void apu_triangle_half()
{
	if (tri.LengthCtr && !tri.wavehold)
		tri.LengthCtr--;
	checkactive();
}
