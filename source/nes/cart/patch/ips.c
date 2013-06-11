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
#include "nes/cart/patch/patch.h"
#include "misc/memutil.h"

int patch_apply_ips(patch_t *p,memfile_t *file)
{
	u8 data[4];
	u32 size,offset,n;
	char eof[4] = "EOF";

	//skip over the header, it has been verified (hopefully)
	memfile_seek(p,5,SEEK_SET);

	while(memfile_eof(p) == 0) {
		memfile_read(data,1,3,p);

		//check for eof marker
		if(memcmp(data,eof,3) == 0)
			break;

		//get offset and seek there
		offset = (data[0] << 16) | (data[1] << 8) | data[2];
		memfile_seek(file,offset,SEEK_SET);

		//get size of chunk
		memfile_read(data,1,2,p);
		size = (data[0] << 8) | data[1];

		//read chunk data
		if(size) {
			memfile_copy(file,p,size);
		}

		//read block fill
		else {
			memfile_read(data,1,2,p);
			size = (data[0] << 8) | data[1];
			memfile_fill(file,memfile_getc(p),size);
		}
	}
	log_printf("patch_apply_ips:  patched ok.\n");
	return(0);
}
