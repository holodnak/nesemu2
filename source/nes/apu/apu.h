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

#ifndef __nes__apu_h__
#define __nes__apu_h__

#include "nes/apu/race.h"
#include "nes/apu/units/frame.h"
#include "nes/apu/units/square.h"
#include "nes/apu/units/triangle.h"
#include "nes/apu/units/noise.h"
#include "nes/apu/units/dpcm.h"
#include "nes/apu/units/external.h"

//apu informations
typedef struct apu_s {
	//sound channels
	square_t		square[2];
	triangle_t	triangle;
	noise_t		noise;
	dpcm_t		dpcm;
	external_t	*external;

	//frame counter
	frame_t		frame;
} apu_t;

typedef external_t apu_external_t;

extern u8 LengthCounts[];

int apu_init();
void apu_kill();
void apu_reset(int hard);
void apu_step();
u8 apu_read(u32 addr);
void apu_write(u32 addr,u8 data);
void apu_setexternal(external_t *ext);
void apu_state(int mode,u8 *data);
void apu_set_region(int r);
void apu_dpcm_fetch();

#endif
