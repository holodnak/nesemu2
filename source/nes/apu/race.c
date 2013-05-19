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

static void race_reset(race_t *r)
{
	r->wavehold = 0;
	r->LengthCtr1 = r->LengthCtr2 = 0;
}

static INLINE void apu_race_reset(int hard)
{
	race_reset(&nes.apu.square[0].race);
	race_reset(&nes.apu.square[1].race);
//	race_reset(&nes.apu.triangle.race);
//	race_reset(&nes.apu.noise.race);
}

static INLINE void apu_race_step()
{
	nes.apu.square[0].wavehold = nes.apu.square[0].race.wavehold;
	if (nes.apu.square[0].race.LengthCtr1)
	{
		if (nes.apu.square[0].LengthCtr == nes.apu.square[0].race.LengthCtr2)
			nes.apu.square[0].LengthCtr = nes.apu.square[0].race.LengthCtr1;
		nes.apu.square[0].race.LengthCtr1 = 0;
	}

	nes.apu.square[1].wavehold = nes.apu.square[1].race.wavehold;
	if (nes.apu.square[1].race.LengthCtr1)
	{
		if (nes.apu.square[1].LengthCtr == nes.apu.square[1].race.LengthCtr2)
			nes.apu.square[1].LengthCtr = nes.apu.square[1].race.LengthCtr1;
		nes.apu.square[1].race.LengthCtr1 = 0;
	}

	nes.apu.triangle.wavehold = nes.apu.triangle.race.wavehold;
	if (nes.apu.triangle.race.LengthCtr1)
	{
		if (nes.apu.triangle.LengthCtr == nes.apu.triangle.race.LengthCtr2)
			nes.apu.triangle.LengthCtr = nes.apu.triangle.race.LengthCtr1;
		nes.apu.triangle.race.LengthCtr1 = 0;
	}

	nes.apu.noise.wavehold = nes.apu.noise.race.wavehold;
	if (nes.apu.noise.race.LengthCtr1)
	{
		if (nes.apu.noise.LengthCtr == nes.apu.noise.race.LengthCtr2)
			nes.apu.noise.LengthCtr = nes.apu.noise.race.LengthCtr1;
		nes.apu.noise.race.LengthCtr1 = 0;
	}
}
