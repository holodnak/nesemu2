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
#include "misc/log.h"
#include "nes/state/block.h"

block_t *block_create(u32 type,u32 size)
{
	block_t *ret = (block_t*)mem_alloc(sizeof(block_t));

	ret->type = type;
	ret->size = size;
	ret->data = (u8*)mem_alloc(size);
	memset(ret->data,0,size);
	return(ret);
}

void block_destroy(block_t *b)
{
	mem_free(b->data);
	mem_free(b);
}

block_t *block_load(memfile_t *file)
{
	block_t *ret = 0;
	u32 type;
	u32 size;

	if(memfile_read(&type,1,4,file) != 4) {
		log_printf("block_load:  error reading block type\n");
		return(0);
	}
	if(memfile_read(&size,1,4,file) != 4) {
		log_printf("block_load:  error reading block size\n");
		return(0);
	}
	ret = block_create(type,size);
	if(memfile_read(ret->data,1,size,file) != size) {
		log_printf("block_load:  error reading block data\n");
		block_destroy(ret);
		return(0);
	}
	return(ret);
}

int block_save(memfile_t *file,block_t *b)
{
	if(memfile_write(&b->type,1,4,file) != 4) {
		log_printf("block_save:  error writing block type\n");
		return(-1);
	}
	if(memfile_write(&b->size,1,4,file) != 4) {
		log_printf("block_save:  error writing block size\n");
		return(-1);
	}
	if(memfile_write(b->data,1,b->size,file) != b->size) {
		log_printf("block_save:  error writing block data\n");
		return(-1);
	}
	return(b->size + 8);
}
