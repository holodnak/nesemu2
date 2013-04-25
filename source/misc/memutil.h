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

#ifndef __memutil_h__
#define __memutil_h__

#define mem_strdup(str)		memutil_strdup(str,__FILE__,__LINE__)
#define mem_alloc(sz)		memutil_alloc(sz,__FILE__,__LINE__)
#define mem_realloc(p,sz)	memutil_realloc(p,sz,__FILE__,__LINE__)
#define mem_free(p)			memutil_free(p,__FILE__,__LINE__)

#include <stdio.h>

int memutil_init();
void memutil_kill();
//void *mem_alloc(size_t size);
//void *mem_realloc(void *ptr,size_t size);
//void mem_free(void *ptr);
char *memutil_strdup(char *str,char *file,int line);
void *memutil_alloc(size_t size,char *file,int line);
void *memutil_realloc(void *ptr,size_t size,char *file,int line);
void memutil_free(void *ptr,char *file,int line);

#endif
