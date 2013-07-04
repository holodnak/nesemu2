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

#define noi nes->apu.noise

static u32 NoiseFreqNTSC[16] = {
	0x004,0x008,0x010,0x020,0x040,0x060,0x080,0x0A0,
	0x0CA,0x0FE,0x17C,0x1FC,0x2FA,0x3F8,0x7F2,0xFE4,
};

static u32 NoiseFreqPAL[16] = {
	0x004,0x007,0x00E,0x01E,0x03C,0x058,0x076,0x094,
	0x0BC,0x0EC,0x162,0x1D8,0x2C4,0x3B0,0x762,0xEC2,
};

static u32 *NoiseFreqTable;

static INLINE void apu_noise_reset(int hard)
{
	noi.volume = noi.envelope = noi.wavehold = noi.datatype = 0;
	noi.freq = 0;
	noi.Vol = 0;
	noi.LengthCtr = 0;
	noi.Envelope = 0;
	noi.Enabled = 0;
	noi.EnvClk = 0;
	noi.Pos = 0;
	noi.CurD = 1;
	noi.Cycles = 1;
	noi.EnvCtr = 1;
}

static INLINE void apu_noise_write(u32 addr,u8 data)
{
	switch (addr)
	{
	case 0:	noi.volume = data & 0x0F;
		noi.envelope = data & 0x10;
		noi.race.wavehold = data & 0x20;
		noi.Vol = noi.envelope ? noi.volume : noi.Envelope;
		if (noi.LengthCtr)
			noi.Pos = ((noi.CurD & 0x4000) ? -2 : 2) * noi.Vol;
		break;
	case 2:	noi.freq = data & 0xF;
		noi.datatype = data & 0x80;
		break;
	case 3:	if (noi.Enabled)
		{
			noi.race.LengthCtr1 = LengthCounts[(data >> 3) & 0x1F];
			noi.race.LengthCtr2 = noi.LengthCtr;
		}
		noi.EnvClk = 1;
		break;
	case 4:	noi.Enabled = data ? 1 : 0;
		if (!noi.Enabled)
			noi.LengthCtr = 0;
		break;
	}
}

static INLINE void apu_noise_step()
{
	// this uses pre-decrement due to the lookup table
	if (!--noi.Cycles)
	{
		noi.Cycles = NoiseFreqTable[noi.freq];
		if (noi.datatype)
			noi.CurD = (noi.CurD << 1) | (((noi.CurD >> 14) ^ (noi.CurD >> 8)) & 0x1);
		else	noi.CurD = (noi.CurD << 1) | (((noi.CurD >> 14) ^ (noi.CurD >> 13)) & 0x1);
		if (noi.LengthCtr)
			noi.Pos = ((noi.CurD & 0x4000) ? -2 : 2) * noi.Vol;
	}
}

static INLINE void apu_noise_quarter()
{
	if (noi.EnvClk)
	{
		noi.EnvClk = 0;
		noi.Envelope = 0xF;
		noi.EnvCtr = noi.volume;
	}
	else if (!noi.EnvCtr--)
	{
		noi.EnvCtr = noi.volume;
		if (noi.Envelope)
			noi.Envelope--;
		else	noi.Envelope = noi.wavehold ? 0xF : 0x0;
	}
	noi.Vol = noi.envelope ? noi.volume : noi.Envelope;
	if (noi.LengthCtr)
		noi.Pos = ((noi.CurD & 0x4000) ? -2 : 2) * noi.Vol;
}

static INLINE void apu_noise_half()
{
	if (noi.LengthCtr && !noi.wavehold)
		noi.LengthCtr--;
}
