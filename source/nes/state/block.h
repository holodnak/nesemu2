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

#ifndef __blocks_h__
#define __blocks_h__

#include <stdio.h>
#include "types.h"

#define MAKEID(c1,c2,c3,c4)	(((c1) << 0) | ((c2) << 8) | ((c3) << 16) | ((c4) << 24))

//if state data is gzipped
#define STATE_FLAG_GZIP		0x8000

//block stored in memory
typedef struct block_s {
	u8		type;		//block type
	u32	size;		//block size
	u8		*data;	//block data
} block_t;

block_t *block_create(u32 type,u32 size);
void block_destroy(block_t *b);
block_t *block_load(FILE *fp);
int block_save(FILE *fp,block_t *b);

#endif
