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

typedef struct square_s {
	u8 length;			//length counter
} square_t;

typedef struct triangle_s {
	u8 length;			//length counter
} triangle_t;

typedef struct noise_s {
	u8 length;			//length counter
} noise_t;

typedef struct dpcm_s {
	u8 length;			//length counter
} dpcm_t;

typedef struct apu_s {
	square_t		square[2];
	triangle_t	triangle;
	noise_t		noise;
	dpcm_t		dpcm;
} apu_t;

int apu_init();
void apu_kill();
void apu_reset(int hard);
u8 apu_read(u32 addr);
void apu_write(u32 addr,u8 data);
void apu_state(int mode,u8 *data);

#endif
