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

static patchblock_t *createblock()
{
	patchblock_t *ret = 0;

	ret = (patchblock_t*)mem_alloc(sizeof(patchblock_t));
	memset(ret,0,sizeof(patchblock_t));
	return(ret);
}

int patch_load_ips(patch_t *ret,const char *filename)
{
	u8 data[4];
	FILE *fp;
	u32 size;
	char eof[4] = "EOF";
	patchblock_t *block,*cur;

	//open rom file
	if((fp = fopen(filename,"rb")) == 0) {
		log_printf("patch_load_ips:  error opening '%s'\n",filename);
		return(1);
	}

	//get length of file
	fseek(fp,0,SEEK_END);
	size = ftell(fp);
	fseek(fp,0,SEEK_SET);

	//skip the header, determineformat already verified it
	fseek(fp,5,SEEK_SET);

	while(feof(fp) == 0) {
		fread(data,1,3,fp);

		//check for eof marker
		if(memcmp(data,eof,3) == 0)
			break;

		//create new patch block
		block = createblock();

		//offset
		block->offset = (data[0] << 16) | (data[1] << 8) | data[2];

		//size
		fread(data,1,2,fp);
		block->size = (data[0] << 8) | data[1];

		log_printf("patch address = $%X, size = %d\n",block->offset,block->size);
		//read block data
		if(block->size) {
			block->type = BLOCK_DATA;
			block->data = (u8*)mem_alloc(block->size);
			fread(block->data,1,block->size,fp);
		}
		else {
			block->type = BLOCK_FILL;
			fread(data,1,2,fp);
			block->size = (data[0] << 8) | data[1];
			block->data = (u8*)mem_alloc(1);
			fread(block->data,1,1,fp);
		}

		//store block
		if(ret->blocks == 0) {
			ret->blocks = block;
		}
		else {
			cur->next = block;
		}
		cur = block;
	}

	//close file and return
	fclose(fp);
	log_printf("patch_load_ips:  loaded '%s'\n",filename);
	return(0);
}
