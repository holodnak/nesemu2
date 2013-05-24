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

#ifndef __nes__apu__square_h__
#define __nes__apu__square_h__

typedef struct square_s {
	u8 volume,envelope,wavehold,duty;
	u8 swpspeed,swpdir,swpstep,swpenab;
	u32 freq;
	u8 Vol;
	u8 CurD;
	u8 LengthCtr;
	u8 EnvCtr, Envelope, BendCtr;
	u8 Enabled, ValidFreq, Active;
	u8 EnvClk, SwpClk;
	u32 Cycles;
	s32 Pos;
	race_t race;
} square_t;

#define apu_square0_reset(hard)			apu_square_reset(&nes->apu.square[0],hard)
#define apu_square0_write(addr,data)	apu_square_write(&nes->apu.square[0],addr,data)
#define apu_square0_step()					apu_square_step(&nes->apu.square[0])
#define apu_square0_quarter()				apu_square_quarter(&nes->apu.square[0])
#define apu_square0_half()					apu_square_half(&nes->apu.square[0])

#define apu_square1_reset(hard)			apu_square_reset(&nes->apu.square[1],hard)
#define apu_square1_write(addr,data)	apu_square_write(&nes->apu.square[1],addr,data)
#define apu_square1_step()					apu_square_step(&nes->apu.square[1])
#define apu_square1_quarter()				apu_square_quarter(&nes->apu.square[1])
#define apu_square1_half()					apu_square_half(&nes->apu.square[1])

#endif
