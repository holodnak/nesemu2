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

#define dpcm nes->apu.dpcm

static u32 DpcmFreqTable[16] = {
	0x1AC,0x17C,0x154,0x140,0x11E,0x0FE,0x0E2,0x0D6,
	0x0BE,0x0A0,0x08E,0x080,0x06A,0x054,0x048,0x036,
};

static INLINE void apu_dpcm_reset(int hard)
{
	dpcm.freq = dpcm.wavehold = dpcm.doirq = dpcm.pcmdata = dpcm.addr = dpcm.len = 0;
	dpcm.CurAddr = dpcm.SampleLen = 0;
	dpcm.silenced = 1;
	dpcm.shiftreg = dpcm.buffer = 0;
	dpcm.LengthCtr = 0;
	dpcm.Pos = 0;

	dpcm.Cycles = 1;
	dpcm.bufempty = 1;
	dpcm.fetching = 0;
	dpcm.outbits = 8;
}

static INLINE void apu_dpcm_write(u32 addr,u8 data)
{
	switch (addr)
	{
	case 0:	dpcm.freq = data & 0xF;
		dpcm.wavehold = (data >> 6) & 0x1;
		dpcm.doirq = data >> 7;
		if (!dpcm.doirq)
			cpu_clear_irq(IRQ_DPCM);
		break;
	case 1:	dpcm.pcmdata = data & 0x7F;
		dpcm.Pos = (dpcm.pcmdata - 0x40) * 3;
		break;
	case 2:	dpcm.addr = data;
		break;
	case 3:	dpcm.len = data;
		break;
	case 4:	if (data)
		{
			if (!dpcm.LengthCtr)
			{
				dpcm.CurAddr = 0xC000 | (dpcm.addr << 6);
				dpcm.LengthCtr = (dpcm.len << 4) + 1;
			}
		}
		else	dpcm.LengthCtr = 0;
		cpu_clear_irq(IRQ_DPCM);
		break;
	}
}

static INLINE void apu_dpcm_step()
{
	// this uses pre-decrement due to the lookup table
	if (!--dpcm.Cycles)
	{
		dpcm.Cycles = DpcmFreqTable[dpcm.freq];
		if (!dpcm.silenced)
		{
			if (dpcm.shiftreg & 1)
			{
				if (dpcm.pcmdata <= 0x7D)
					dpcm.pcmdata += 2;
			}
			else
			{
				if (dpcm.pcmdata >= 0x02)
					dpcm.pcmdata -= 2;
			}
			dpcm.shiftreg >>= 1;
			dpcm.Pos = (dpcm.pcmdata - 0x40) * 3;
		}
		if (!--dpcm.outbits)
		{
			dpcm.outbits = 8;
			if (!dpcm.bufempty)
			{
				dpcm.shiftreg = dpcm.buffer;
				dpcm.bufempty = 1;
				dpcm.silenced = 0;
			}
			else	dpcm.silenced = 1;
		}
	}
	if (dpcm.bufempty && !dpcm.fetching && dpcm.LengthCtr)
	{
		dpcm.fetching = 1;
//		CPU::PCMCycles = 4;
		// decrement LengthCtr now, so $4015 reads are updated in time
		dpcm.LengthCtr--;
	}
}

static INLINE void apu_dpcm_fetch()
{
	dpcm.buffer = cpu_read(dpcm.CurAddr);
	cpu_tick();
	dpcm.bufempty = 0;
	dpcm.fetching = 0;
	if (++dpcm.CurAddr == 0x10000)
		dpcm.CurAddr = 0x8000;
	if (!dpcm.LengthCtr)
	{
		if (dpcm.wavehold)
		{
			dpcm.CurAddr = 0xC000 | (dpcm.addr << 6);
			dpcm.LengthCtr = (dpcm.len << 4) + 1;
		}
		else if (dpcm.doirq)
			cpu_set_irq(IRQ_DPCM);
	}
}

#undef dpcm