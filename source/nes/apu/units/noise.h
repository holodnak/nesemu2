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

#ifndef __nes__apu__noise_h__
#define __nes__apu__noise_h__

//apu noise
typedef struct noise_s {
	u8 volume, envelope, wavehold, datatype;
	u32 freq;
	u32 CurD;
	u8 Vol;
	u8 LengthCtr;
	u8 EnvCtr, Envelope;
	u8 Enabled;
	u8 EnvClk;
	u32 Cycles;
	s32 Pos;
	race_t race;
} noise_t;

void apu_noise_reset(int hard);
void apu_noise_write(u32 addr,u8 data);
void apu_noise_step();
void apu_noise_quarter();
void apu_noise_half();

#endif
