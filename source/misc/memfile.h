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

#ifndef __memfile_h__
#define __memfile_h__

#include "types.h"

typedef struct memfile_s {

	//file data
	u8		*data;
	u32	size;

	//current position in the data
	u32	curpos;

	//has the data been modified
	u32	changed;

	//filename and mode
	char	*filename;
	char	*mode;

	//handle
	void	*handle;

} memfile_t;

memfile_t *memfile_create();
memfile_t *memfile_open(char *filename,char *mode);
memfile_t *memfile_open_memory(u8 *data,u32 size);
void memfile_close(memfile_t *mf);
u32 memfile_size(memfile_t *mf);
u32 memfile_tell(memfile_t *mf);
int memfile_seek(memfile_t *mf,int pos,int mode);
void memfile_rewind(memfile_t *mf);
int memfile_eof(memfile_t *mf);
u32 memfile_read(void *data,int chunksize,int chunks,memfile_t *mf);
u32 memfile_write(void *data,int chunksize,int chunks,memfile_t *mf);
u32 memfile_copy(memfile_t *dest,memfile_t *src,u32 size);
u32 memfile_fill(memfile_t *dest,u8 ch,u32 size);
int memfile_getc(memfile_t *mf);
int memfile_putc(u8 ch,memfile_t *mf);
char *memfile_gets(char *str,int n,memfile_t *mf);

#endif
