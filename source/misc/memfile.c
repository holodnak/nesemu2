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

#include <stdio.h>
#include <string.h>
#include "misc/memfile.h"
#include "misc/memutil.h"
#include "misc/log.h"

memfile_t *memfile_create()
{
	memfile_t *ret = 0;

	//allocate struct data
	ret = (memfile_t*)mem_alloc(sizeof(memfile_t));
	memset(ret,0,sizeof(memfile_t));

	//return
	return(ret);
}

memfile_t *memfile_open(char *filename,char *mode)
{
	FILE *fp;
	memfile_t *ret = 0;

	//try to open the file
	if((fp = fopen(filename,mode)) == 0) {
		log_printf("memfile_open:  error opening '%s'\n",filename);
		return(0);
	}

	//create new file
	ret = memfile_create();

	//copy filename and mode
	ret->filename = mem_strdup(filename);
	ret->mode = mem_strdup(mode);

	//get file size
	fseek(fp,0,SEEK_END);
	ret->size = (u32)ftell(fp);
	fseek(fp,0,SEEK_SET);

	//allocate file data and read the file into it
	ret->data = (u8*)mem_alloc(ret->size);
	if(fread(ret->data,1,ret->size,fp) != ret->size) {
		memfile_close(ret);
		return(0);
	}

	//if append mode, seek to end
	if(strchr(mode,'a')) {
		memfile_seek(ret,0,SEEK_END);
	}

	//initialize the current position and changed var
	ret->curpos = 0;
	ret->changed = 0;

	//save file handle
	ret->handle = (void*)fp;

	return(ret);
}

memfile_t *memfile_open_memory(u8 *data,u32 size)
{
	memfile_t *ret;

	ret = memfile_create();
	ret->size = size;
	ret->data = (u8*)mem_dup(data,size);
	return(ret);
}

void memfile_close(memfile_t *mf)
{
	FILE *fp = (FILE*)mf->handle;

	//is there is a filename associated with this file, it should already be open
	if(mf->filename) {
		if(strchr(mf->mode,'w')) {
			log_printf("memfile_close:  writing changes to '%s'\n",mf->filename);
			fseek(fp,0,SEEK_SET);
			fwrite(mf->data,1,mf->size,fp);
		}
		fclose(fp);
		mem_free(mf->filename);
		mem_free(mf->mode);
	}
	if(mf->data)
		mem_free(mf->data);
	mem_free(mf);
}

u32 memfile_size(memfile_t *mf)
{
	return(mf->size);
}

u32 memfile_tell(memfile_t *mf)
{
	return(mf->curpos);
}

int memfile_seek(memfile_t *mf,int pos,int mode)
{
	if(mode == SEEK_END)
		mf->curpos = mf->size;
	else if(mode == SEEK_SET)
		mf->curpos = pos;
	else if(mode == SEEK_CUR)
		mf->curpos += pos;
	if(mf->curpos < 0) {
		mf->curpos = 0;
		log_printf("memfile_seek:  trying to seek before beginning of file\n");
		return(1);
	}
	if(mf->curpos > mf->size) {
		mf->curpos = mf->size;
		log_printf("memfile_seek:  trying to seek past end of file\n");
		return(1);
	}
	return(0);
}

void memfile_rewind(memfile_t *mf)
{
	memfile_seek(mf,0,SEEK_SET);
}

int memfile_eof(memfile_t *mf)
{
	if(mf->curpos == mf->size)
		return(1);
	return(0);
}

u32 memfile_read(void *data,int chunksize,int chunks,memfile_t *mf)
{
	int size = chunksize * chunks;

	if(memfile_eof(mf) != 0) {
		log_printf("memfile_read:  cannot read past eof\n");
		return(0);
	}
	memcpy(data,mf->data + mf->curpos,size);
	mf->curpos += size;
	return(chunks);
}

u32 memfile_write(void *data,int chunksize,int chunks,memfile_t *mf)
{
	u32 size = chunksize * chunks;
	u32 newsize = mf->curpos + size;

	if(newsize > mf->size) {
		mf->data = mem_realloc(mf->data,newsize);
		mf->size = newsize;
	}
	memcpy(mf->data + mf->curpos,data,size);
	mf->curpos += size;
	mf->changed++;
	return(chunks);
}

u32 memfile_copy(memfile_t *dest,memfile_t *src,u32 size)
{
	while(size) {
		memfile_putc(memfile_getc(src),dest);
		size--;
	}
	return(size);
}

u32 memfile_fill(memfile_t *dest,u8 ch,u32 size)
{
	while(size) {
		memfile_putc(ch,dest);
		size--;
	}
	return(size);
}

int memfile_getc(memfile_t *mf)
{
	int ret;

	if(memfile_eof(mf) != 0) {
		log_printf("memfile_getc:  cannot read past eof\n");
		return(-1);
	}
	ret = (int)mf->data[mf->curpos];
	mf->curpos++;
	return(ret);
}

int memfile_putc(u8 ch,memfile_t *mf)
{
	u32 newsize = mf->curpos + 1;

	if(newsize > mf->size) {
		mf->data = mem_realloc(mf->data,newsize);
		mf->size = newsize;
	}
	mf->data[mf->curpos] = ch;
	mf->curpos++;
	mf->changed++;
	return(0);
}
