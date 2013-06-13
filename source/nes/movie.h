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

#ifndef __nes__movie_h__
#define __nes__movie_h__

#include "types.h"

typedef struct movie_s {
	u8		*data;
	u32	len,pos;
	int	mode;

	u64	startframe,endframe;
} movie_t;

int nes_movie_init();
void nes_movie_kill();
void nes_movie_frame();
int nes_movie_record();
int nes_movie_play();
int nes_movie_stop();
u8 nes_movie_read_u8();
void nes_movie_write_u8(u8 data);

#endif
