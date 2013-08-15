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
#include <stdlib.h>
#include <string.h>
#include "misc/memutil.h"
#include "system/sdl/console/linebuffer.h"

#define CONSOLE_LINEBUF_SIZE	128

//line buffer (circular)
static char **linebuf = 0;

//max width of strings in the buffer (for chopping them)
static size_t maxlen;

//next available line for use in the buffer
static int nextline;

//is the buffer full?  are we circling?
static int full;

static void increment_nextline()
{
	nextline++;
	if(nextline == CONSOLE_LINEBUF_SIZE) {
		nextline = 0;
		full = 1;
	}
}

static void freeline(int n)
{
	if(linebuf[n])
		mem_free(linebuf[n]);
	linebuf[n] = 0;
}

int linebuffer_init(int len)
{
	linebuf = (char**)mem_alloc(sizeof(char*) * CONSOLE_LINEBUF_SIZE);
	memset(linebuf,0,sizeof(char*) * CONSOLE_LINEBUF_SIZE);
	nextline = 0;
	maxlen = len;
	return(0);
}

void linebuffer_kill()
{
	if(linebuf) {
		linebuffer_clear();
		mem_free(linebuf);
	}
}

static void addline(char *str)
{
	freeline(nextline);
	linebuf[nextline] = mem_strdup(str);
	increment_nextline();
}
void linebuffer_add(char *str)
{
	//free line (if it is already used)
	if(strlen(str) < maxlen) {
		addline(str);
	}
	else {
		char *tmp = mem_strdup(str);
		char *tmpp,*p = tmp;
		size_t len = strlen(p);

		while(len >= maxlen) {
			tmpp = mem_strdup(p);
			tmpp[maxlen] = 0;
			addline(tmpp);
			mem_free(tmpp);
			p[maxlen-1] = 0;
//			printf("adding wrapped line:  '%s' (len = %d, maxlen = %d)\n",p,len,maxlen);
			p += maxlen;
			len -= maxlen;
		}
//		printf("adding wrapped line (tail):  '%s' (len = %d, maxlen = %d)\n",p,len,maxlen);
		addline(p);
		mem_free(tmp);
	}
}

void linebuffer_clear()
{
	int i;

	for(i=0;i<CONSOLE_LINEBUF_SIZE;i++) {
		freeline(i);
	}
	nextline = 0;
	full = 0;
}

char *linebuffer_get(int index)
{
	if(index >= CONSOLE_LINEBUF_SIZE)
		return(0);
	return(linebuf[index]);
}

//get a line relative to the last line added
char *linebuffer_getrelative(int index)
{
	int line = nextline - (1 + index);

	if(line < 0)
		line = CONSOLE_LINEBUF_SIZE - (1 + index);
	return(linebuffer_get(line));
}

//returns the number of lines used
int linebuffer_count()
{
	return(full ? CONSOLE_LINEBUF_SIZE : nextline);
}
