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

#ifndef __nes__cart__patch_h__
#define __nes__cart__patch_h__

#include "misc/memfile.h"

#define BLOCK_DATA	0x10000
#define BLOCK_FILL	0x20000

typedef struct patchblock_s {
	struct patchblock_s *next;
	int type;
	int offset;
	int size;
	u8 *data;
} patchblock_t;

typedef struct patch_s {
	patchblock_t *blocks;
} patch_t;

patch_t *patch_load(const char *filename);
void patch_unload(patch_t *p);
int patch_file(patch_t *p,memfile_t *file);

#endif
