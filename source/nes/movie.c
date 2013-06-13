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

#include <string.h>
#include "nes/nes.h"
#include "misc/memutil.h"
#include "misc/memfile.h"
#include "misc/crc32.h"
#include "system/video.h"

#define MOVIE_PREALLOC_SIZE	1024

static char movie_ident[] = "NMV\0";

int nes_movie_init()
{
	memset(&nes->movie,0,sizeof(movie_t));
	return(0);
}

void nes_movie_kill()
{
	nes_movie_unload();
}

int nes_movie_load(char *filename)
{
	memfile_t *file;
	u32 size;
	u8 flags;
	int devid[3];
	char ident[5];

	nes_movie_unload();
	if((file = memfile_open(filename,"rb")) == 0) {
		log_printf("nes_movie_load:  error opening movie '%s'\n",filename);
		return(1);
	}
	//need header and stuff here
	memfile_read(ident,1,4,file);
	if(memcmp(ident,movie_ident,4) != 0) {
		log_printf("nes_movie_load:  bad movie ident\n");
		memfile_close(file);
		return(2);
	}

	//read movie flags
	memfile_read(&flags,1,sizeof(u8),file);
	if(flags & 1)
		nes->movie.mode |= MOVIE_TEST;

	//set required input devices
	memfile_read(&devid[0],1,sizeof(int),file);
	memfile_read(&devid[1],1,sizeof(int),file);
	memfile_read(&devid[2],1,sizeof(int),file);
	nes_set_inputdev(0,devid[0]);
	nes_set_inputdev(1,devid[1]);
	nes_set_inputdev(2,devid[2]);

	//read movie data
	memfile_read(&nes->movie.startframe,1,sizeof(u32),file);
	memfile_read(&nes->movie.endframe,1,sizeof(u32),file);
	memfile_read(&nes->movie.crc32,1,sizeof(u32),file);
	memfile_read(&nes->movie.len,1,sizeof(u32),file);
	log_printf("nes_movie_load:  start, end = %d, %d :: len = %d bytes\n",nes->movie.startframe,nes->movie.endframe,nes->movie.len);
	nes->movie.data = (u8*)mem_alloc(nes->movie.len);
	memfile_read(nes->movie.data,1,nes->movie.len,file);
	size = memfile_size(file) - memfile_tell(file);
	nes->movie.state = memfile_create();
	memfile_copy(nes->movie.state,file,size);
	memfile_close(file);
	return(0);
}

int nes_movie_save(char *filename)
{
	memfile_t *file;
	u8 flags;

	if(nes->movie.len == 0) {
		log_printf("nes_movie_save:  no movie data to save\n");
		return(0);
	}
	if(nes->movie.mode & MOVIE_RECORD) {
		log_printf("nes_movie_save:  still recording, stopping...\n");
		nes_movie_stop();
	}
	if((file = memfile_open(filename,"wb")) == 0) {
		log_printf("nes_movie_save:  error opening movie '%s'\n",filename);
		return(1);
	}
	memfile_seek(nes->movie.state,0,SEEK_SET);

	flags = (nes->movie.mode & MOVIE_TEST) ? 1 : 0;

	//header (just ident for now)
	memfile_write(movie_ident,1,4,file);
	memfile_write(&flags,1,sizeof(u8),file);

	//save input device configuration
	memfile_write(&nes->inputdev[0]->id,1,sizeof(int),file);
	memfile_write(&nes->inputdev[1]->id,1,sizeof(int),file);
	memfile_write(&nes->expdev->id,1,sizeof(int),file);

	//save movie data
	memfile_write(&nes->movie.startframe,1,sizeof(u32),file);
	memfile_write(&nes->movie.endframe,1,sizeof(u32),file);
	memfile_write(&nes->movie.crc32,1,sizeof(u32),file);
	memfile_write(&nes->movie.len,1,sizeof(u32),file);
	log_printf("nes_movie_save:  start, end = %d, %d :: len = %d bytes\n",nes->movie.startframe,nes->movie.endframe,nes->movie.len);
	memfile_write(nes->movie.data,1,nes->movie.len,file);

	//append savestate to end of movie
	memfile_copy(file,nes->movie.state,memfile_size(nes->movie.state));
	memfile_close(file);
	return(0);
}

void nes_movie_unload()
{
	if(nes->movie.data)
		mem_free(nes->movie.data);
	if(nes->movie.state)
		memfile_close(nes->movie.state);
	memset(&nes->movie,0,sizeof(movie_t));
}

//process frame for movie, MUST be called after input updates
void nes_movie_frame()
{
	if((nes->movie.mode & MOVIE_PLAY) && nes->movie.endframe == nes->ppu.frames) {
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
	//get rid of previous data
	if(nes->movie.data)
		nes_movie_unload();

	//initialize the movie struct
	nes->movie.mode = MOVIE_RECORD;
	nes->movie.pos = 0;
	nes->movie.len = MOVIE_PREALLOC_SIZE;
	nes->movie.data = (u8*)mem_alloc(nes->movie.len);
	nes->movie.startframe = nes->ppu.frames;

	//save state here for loading
	nes->movie.state = memfile_create();
	state_save(nes->movie.state);

	return(0);
}

//setup for playing data
int nes_movie_play()
{
	if(nes->movie.state == 0) {
		log_printf("nes_movie_play:  cannot play movie, no data loaded\n");
		return(1);
	}

	memfile_seek(nes->movie.state,0,SEEK_SET);
	state_load(nes->movie.state);
	//load state for here

	nes->movie.mode = MOVIE_PLAY;
	nes->movie.pos = 0;
	return(0);
}

//stop recording/playing
int nes_movie_stop()
{
	u32 crc;

	log_printf("nes_movie_stop:  stopping movie.\n");
	if(nes->movie.mode & MOVIE_RECORD) {
		nes->movie.len = nes->movie.pos;
		nes->movie.endframe = nes->ppu.frames;
		nes->movie.crc32 = crc32(video_getscreen(),256 * 240);
		log_printf("screen crc32 = %08X (frame %d)\n",nes->movie.crc32,nes->ppu.frames);
	}
	if(nes->movie.mode & MOVIE_PLAY) {
		crc = crc32(video_getscreen(),256 * 240);
		if(crc != nes->movie.crc32) {
			log_printf("error:  screen crc32 doesnt match!  (wanted %08X, got %08X)\n",nes->movie.crc32,crc);
		}
		else
			log_printf("screen crc32 matched!\n");
	}
	nes->movie.mode = 0;
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
		nes_movie_stop();
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
		log_printf("nes_movie_write_u8:  increasing size of movie buffer (%d bytes)\n",nes->movie.len);
	}

	//write the byte
	nes->movie.data[nes->movie.pos++] = data;
}
