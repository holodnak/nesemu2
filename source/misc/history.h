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

#ifndef __history_h__
#define __history_h__

typedef struct historyline_s {
	struct historyline_s *prev,*next;
	char *str;
} historyline_t;

typedef struct history_s {
	historyline_t *lines;
	historyline_t *cur;
} history_t;

history_t *history_create();
void history_destroy(history_t *h);
void history_add(history_t *h,char *str);
void history_clear(history_t *h);
char *history_getcur(history_t *h);
char *history_getnext(history_t *h);
char *history_getprev(history_t *h);

#endif
