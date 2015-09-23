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

#include <stdlib.h>
#include <string.h>
#include "misc/memutil.h"
#include "misc/log.h"

#define MAX_CHUNKS	1024

typedef struct memchunk_s {
	void *ptr;
	size_t size;
	char file[256];
	int line;
	int flags;
} memchunk_t;

static memchunk_t chunks[MAX_CHUNKS];

static int inited = 0;
static int num_alloc;
static int num_realloc;
static int num_free;
static int max_chunks;
static size_t max_bytes;
static size_t num_bytes;

static char *bytestr(size_t sz)
{
	static char str[64];

	if(sz < 1024)
		sprintf(str,"%ub",sz);
	else if(sz < 1024 * 1024)
		sprintf(str,"%.2fkb",(double)sz / 1024.0f);
	else if(sz < 1024 * 1024 * 1024)
		sprintf(str,"%.2fmb",(double)sz / 1024.0f / 1024.0f);
	return(str);
}

//check if inited already, if not do it
static void checkinited()
{
	if(inited == 0)
		memutil_init();
}

//count number of used chunks
static void memutil_count()
{
	int i,chunknum = 0;
	size_t bytes = 0;

	for(i=0;i<MAX_CHUNKS;i++) {
		if(chunks[i].flags) {
			chunknum++;
			bytes += chunks[i].size;
		}
	}
	max_chunks = (chunknum > max_chunks) ? chunknum : max_chunks;
	max_bytes = (bytes > max_bytes) ? bytes : max_bytes;
}

int memutil_init()
{
	if(inited)
		return(0);
//	if(chunks == 0) {
//		chunks = (memchunk_t*)malloc(sizeof(memchunk_t) * MAX_CHUNKS);
		memset(chunks,0,sizeof(memchunk_t)*MAX_CHUNKS);
//	}
	num_alloc = 0;
	num_realloc = 0;
	num_free = 0;
	num_bytes = 0;
	max_chunks = 0;
	max_bytes = 0;
	inited = 1;
	return(0);
}

void memutil_kill()
{
	int i;

	checkinited();
	for(i=0;i<MAX_CHUNKS;i++) {
		if(chunks[i].flags & 1) {
			log_printf("mem_kill:  memory not free'd in file %s @ line %d\n",chunks[i].file,chunks[i].line);
			free(chunks[i].ptr);
		}
		memset(&chunks[i],0,sizeof(memchunk_t));
	}
//	free(chunks);
//	chunks = 0;
	log_printf("mem_kill:  num alloc, realloc, free = %d, %d, %d (%s was allocated)\n",num_alloc,num_realloc,num_free,bytestr(num_bytes));
	log_printf("mem_kill:  max_chunks, max_bytes = %d, %s\n",max_chunks,bytestr(max_bytes));
}

char *memutil_strdup(char *str,char *file,int line)
{
	size_t size = strlen(str) + 1;
	char *ret;

	ret = memutil_alloc(size,file,line);
	strcpy(ret,str);
	return(ret);
}

void *memutil_dup(void *data,size_t size,char *file,int line)
{
	void *ret = memutil_alloc(size,file,line);

	if(ret)
		memcpy(ret,data,size);
	return(ret);
}

void *memutil_alloc(size_t size,char *file,int line)
{
	void *ret;
	int i;

	checkinited();
	if ((ret = malloc(size)) == 0) {
		log_printf("memutil_alloc:  unable to alloc %d bytes\n", size);
		exit(-1);
	}
	memset(ret,0,size);
	num_alloc++;
	num_bytes += size;
	for(i=0;i<MAX_CHUNKS;i++) {
		if(chunks[i].flags == 0) {
			chunks[i].flags |= 1;
			strcpy(chunks[i].file,file);
			chunks[i].line = line;
			chunks[i].ptr = ret;
			chunks[i].size = size;
			break;
		}
	}
	if(i == MAX_CHUNKS) {
		log_printf("memutil_alloc:  max number of chunks reached.  increase and recompile.\n");
		return(0);
	}
	memutil_count();
	return(ret);
}

void *memutil_realloc(void *ptr,size_t size,char *file,int line)
{
	void *ret;
	int i;

	checkinited();
	if(ptr == 0)
		return(memutil_alloc(size,file,line));
	ret = realloc(ptr,size);
	num_realloc++;
	for(i=0;i<MAX_CHUNKS;i++) {
		if(chunks[i].ptr == ptr) {
			chunks[i].flags |= 1;
			strcpy(chunks[i].file,file);
			chunks[i].line = line;
			chunks[i].ptr = ret;
			if(size > chunks[i].size)
				num_bytes += size - chunks[i].size;
			chunks[i].size = size;
			break;
		}
	}
	if(i == MAX_CHUNKS) {
		log_printf("memutil_realloc:  trying to realloc memory not found in list in file %s @ line %d\n",file,line);
	}
	memutil_count();
	return(ret);
}

void memutil_free(void *ptr,char *file,int line)
{
	int i;

	checkinited();
	num_free++;
	for(i=0;i<MAX_CHUNKS;i++) {
		if(chunks[i].ptr == ptr && chunks[i].flags) {
			memset(&chunks[i],0,sizeof(memchunk_t));
			break;
		}
	}
	if(i == MAX_CHUNKS) {
		log_printf("memutil_free:  trying to free memory not in list in file %s @ line %d\n",file,line);
	}
	free(ptr);
	memutil_count();
}
