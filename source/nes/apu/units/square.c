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

static const s8 SquareDuty[4][8] = {
	{-4,-4,-4,-4,-4,-4,-4,+4},
	{-4,-4,-4,-4,-4,-4,+4,+4},
	{-4,-4,-4,-4,+4,+4,+4,+4},
	{+4,+4,+4,+4,+4,+4,-4,-4},
};

static INLINE void squarecheckactive(square_t *sq)
{
	sq->ValidFreq = (sq->freq >= 0x8) && ((sq->swpdir) || !((sq->freq + (sq->freq >> sq->swpstep)) & 0x800));
	sq->Active = sq->LengthCtr && sq->ValidFreq;
	sq->Pos = sq->Active ? (SquareDuty[sq->duty][sq->CurD] * sq->Vol) : 0;
}

static INLINE void apu_square_reset(square_t *sq,int hard)
{
	sq->volume = sq->envelope = sq->wavehold = sq->duty = 0;
	sq->swpspeed = sq->swpdir = sq->swpstep = sq->swpenab = 0;
	sq->freq = 0;
	sq->Vol = 0;
	sq->CurD = 0;
	sq->LengthCtr = 0;
	sq->Envelope = 0;
	sq->Enabled = sq->ValidFreq = sq->Active = 0;
	sq->EnvClk = sq->SwpClk = 0;
	sq->Pos = 0;
	sq->Cycles = 1;
	sq->EnvCtr = 1;
	sq->BendCtr = 1;
}

static INLINE void apu_square_write(square_t *sq,u32 addr,u8 data)
{
	switch (addr)
	{
	case 0:	sq->volume = data & 0xF;
		sq->envelope = data & 0x10;
		sq->race.wavehold = data & 0x20;
		sq->duty = (data >> 6) & 0x3;
		sq->Vol = sq->envelope ? sq->volume : sq->Envelope;
		break;
	case 1:	sq->swpstep = data & 0x07;
		sq->swpdir = data & 0x08;
		sq->swpspeed = (data >> 4) & 0x7;
		sq->swpenab = data & 0x80;
		sq->SwpClk = 1;
		break;
	case 2:	sq->freq &= 0x700;
		sq->freq |= data;
		break;
	case 3:	sq->freq &= 0xFF;
		sq->freq |= (data & 0x7) << 8;
		if (sq->Enabled)
		{
			sq->race.LengthCtr1 = LengthCounts[(data >> 3) & 0x1F];
			sq->race.LengthCtr2 = sq->LengthCtr;
		}
		sq->CurD = 0;
		sq->EnvClk = 1;
		break;
	case 4:	sq->Enabled = data ? 1 : 0;
		if (!sq->Enabled)
			sq->LengthCtr = 0;
		break;
	}
	squarecheckactive(sq);
}

static INLINE void apu_square_step(square_t *sq)
{
	if (!sq->Cycles--)
	{
		sq->Cycles = sq->freq << 1;
		sq->CurD = (sq->CurD - 1) & 0x7;
		if (sq->Active)
			sq->Pos = SquareDuty[sq->duty][sq->CurD] * sq->Vol;
	}
}

static INLINE void apu_square_quarter(square_t *sq)
{
	if (sq->EnvClk)
	{
		sq->EnvClk = 0;
		sq->Envelope = 0xF;
		sq->EnvCtr = sq->volume;
	}
	else if (!sq->EnvCtr--)
	{
		sq->EnvCtr = sq->volume;
		if (sq->Envelope)
			sq->Envelope--;
		else	sq->Envelope = sq->wavehold ? 0xF : 0x0;
	}
	sq->Vol = sq->envelope ? sq->volume : sq->Envelope;
	squarecheckactive(sq);
}

static INLINE void apu_square_half(square_t *sq)
{
	if (!sq->BendCtr--)
	{
		sq->BendCtr = sq->swpspeed;
		if (sq->swpenab && sq->swpstep && sq->ValidFreq)
		{
			int sweep = sq->freq >> sq->swpstep;
			sq->freq += sq->swpdir ? ~sweep : sweep;
		}
	}
	if (sq->SwpClk)
	{
		sq->SwpClk = 0;
		sq->BendCtr = sq->swpspeed;
	}
	if (sq->LengthCtr && !sq->wavehold)
		sq->LengthCtr--;
	squarecheckactive(sq);
}
