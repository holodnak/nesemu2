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
#include "misc/history.h"
#include "misc/memutil.h"
#include "misc/log.h"

history_t *history_create()
{
	history_t *ret = (history_t*)mem_alloc(sizeof(history_t));

	memset(ret,0,sizeof(history_t));
	return(ret);
}

void history_destroy(history_t *h)
{
	history_clear(h);
	mem_free(h);
}

void history_add(history_t *h,char *str)
{
	historyline_t *p = (historyline_t*)mem_alloc(sizeof(historyline_t));

	p->prev = 0;
	p->next = h->lines;
	p->str = mem_strdup(str);
	if(h->lines == 0)
		h->lines = p;
	else
		h->lines->prev = p;
	h->lines = p;
	h->cur = 0;
}

void history_clear(history_t *h)
{
	historyline_t *p,*line = h->lines;

	while(line) {
		p = line;
		line = line->next;
		mem_free(p->str);
		mem_free(p);
	}
}

char *history_getcur(history_t *h)
{
	return(h->cur ? h->cur->str : 0);
}

char *history_getnext(history_t *h)
{
	if(h->cur == 0)
		h->cur = h->lines;
	else {
		if(h->cur->next)
			h->cur = h->cur->next;
	}
	return(history_getcur(h));
}

char *history_getprev(history_t *h)
{
	if(h->cur) {
		if(h->cur->prev)
			h->cur = h->cur->prev;
	}
	return(history_getcur(h));
}
