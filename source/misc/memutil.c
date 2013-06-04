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
	char file[128];
	int line;
	int flags;
} memchunk_t;

static memchunk_t *chunks = 0;

static int num_alloc;
static int num_realloc;
static int num_free;
static size_t num_bytes;

static char *bytestr(size_t sz)
{
	static char str[64];

	if(sz < 1024)
		sprintf(str,"%db",sz);
	else if(sz < 1024 * 1024)
		sprintf(str,"%.2fkb",(double)sz / 1024.0f);
	else if(sz < 1024 * 1024 * 1024)
		sprintf(str,"%.2fmb",(double)sz / 1024.0f / 1024.0f);
	return(str);
}

int memutil_init()
{
	if(chunks == 0) {
		chunks = (memchunk_t*)malloc(sizeof(memchunk_t) * MAX_CHUNKS);
		memset(chunks,0,sizeof(memchunk_t)*MAX_CHUNKS);
	}
	num_alloc = 0;
	num_realloc = 0;
	num_free = 0;
	num_bytes = 0;
	return(0);
}

void memutil_kill()
{
	int i;

	for(i=0;i<MAX_CHUNKS;i++) {
		if(chunks[i].flags & 1) {
			log_printf("mem_kill:  memory not free'd in file %s @ line %d\n",chunks[i].file,chunks[i].line);
		}
	}
	free(chunks);
	chunks = 0;
	log_printf("mem_kill:  num alloc, realloc, free = %d, %d, %d (%s was allocated)\n",num_alloc,num_realloc,num_free,bytestr(num_bytes));
}

char *memutil_strdup(char *str,char *file,int line)
{
	size_t size = strlen(str) + 1;
	char *ret;

	ret = memutil_alloc(size,file,line);
	strcpy(ret,str);
	return(ret);
}

void *memutil_alloc(size_t size,char *file,int line)
{
	void *ret;
	int i;

	ret = malloc(size);
	num_alloc++;
	num_bytes += size;
	for(i=0;i<MAX_CHUNKS;i++) {
		if(chunks[i].flags == 0) {
			chunks[i].flags |= 1;
			strcpy(chunks[i].file,file);
			chunks[i].line = line;
			chunks[i].ptr = ret;
			break;
		}
	}
	return(ret);
}

void *memutil_realloc(void *ptr,size_t size,char *file,int line)
{
	void *ret;
	int i;

	if(ptr == 0)
		return(memutil_alloc(size,file,line));
	ret = realloc(ptr,size);
	num_realloc++;
	num_bytes += size;
	for(i=0;i<MAX_CHUNKS;i++) {
		if(chunks[i].ptr == ptr) {
			chunks[i].flags |= 1;
			strcpy(chunks[i].file,file);
			chunks[i].line = line;
			chunks[i].ptr = ret;
			break;
		}
	}
	if(i == MAX_CHUNKS) {
		log_printf("memutil_realloc:  trying to realloc memory not found in list in file %s @ line %d\n",file,line);
	}
	return(ret);
}

void memutil_free(void *ptr,char *file,int line)
{
	int i;

	free(ptr);
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
}
