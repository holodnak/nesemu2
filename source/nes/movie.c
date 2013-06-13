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
#include "misc/memutil.h"
#include "misc/memfile.h"

#define MOVIE_PREALLOC_SIZE	1024

static memfile_t *state;

int nes_movie_init()
{
	state = 0;
	nes->movie.mode = 0;
	nes->movie.pos = 0;
	nes->movie.len = 0;
	nes->movie.data = 0;
	return(0);
}

void nes_movie_kill()
{
	if(nes->movie.data)
		mem_free(nes->movie.data);
	if(state)
		memfile_close(state);
}

//process frame for movie, MUST be called after input updates
void nes_movie_frame()
{
	if(nes->movie.endframe == nes->ppu.frames) {
		log_printf("nes_movie_frame:  reached end of movie, stopping.\n");
		nes_movie_stop();
	}
	else {
		nes->inputdev[0]->movie(nes->movie.mode);
		nes->inputdev[1]->movie(nes->movie.mode);
		nes->expdev->movie(nes->movie.mode);
	}
}

//initialize movie data for recording
int nes_movie_record()
{
	if(nes->movie.data)
		mem_free(nes->movie.data);
	nes->movie.mode = MOVIE_RECORD;
	nes->movie.pos = 0;
	nes->movie.len = MOVIE_PREALLOC_SIZE;
	nes->movie.data = (u8*)mem_alloc(nes->movie.len);

	nes->movie.startframe = nes->ppu.frames;

	//save state here for loading
	state = memfile_create();
	state_save(state);

	return(0);
}

//setup for playing data
int nes_movie_play()
{
	if(state == 0)
		return(1);

	state_load(state);
	//load state for here

	nes->movie.mode = MOVIE_PLAY;
	nes->movie.pos = 0;
	return(0);
}

//stop recording/playing
int nes_movie_stop()
{
	log_printf("stopping movie.\n");
	nes->movie.mode = 0;
	nes->movie.endframe = nes->ppu.frames;
	return(0);
}

u8 nes_movie_read_u8()
{
	u8 ret;

	//make sure we are playing
	if((nes->movie.mode & MOVIE_PLAY) == 0) {
		log_printf("nes_movie_read_u8:  not playing, cannot read data\n");
		return(0xFF);
	}

	//make sure we dont read past end of the data
	if(nes->movie.pos < nes->movie.len)
		ret = nes->movie.data[nes->movie.pos++];
	else {
		log_printf("nes_movie_read_u8:  playing past end of movie, stopping\n");
		nes->movie.mode = 0;
		ret = 0xFF;
	}

	return(ret);
}

void nes_movie_write_u8(u8 data)
{
	//make sure we are recording
	if((nes->movie.mode & MOVIE_RECORD) == 0) {
		log_printf("nes_movie_write_u8:  not recording, cannot write data\n");
		return;
	}

	//see if we are at the end of the movie data
	if(nes->movie.pos == nes->movie.len) {
		nes->movie.len += MOVIE_PREALLOC_SIZE;
		nes->movie.data = (u8*)mem_realloc(nes->movie.data,nes->movie.len);
	}

	//write the byte
	nes->movie.data[nes->movie.pos++] = data;
}
