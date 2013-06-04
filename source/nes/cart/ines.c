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
#include <string.h>
#include <stdlib.h>
#include "misc/memutil.h"
#include "misc/log.h"
#include "nes/cart/cart.h"
#include "mappers/mappers.h"

static int parse_ines_header(cart_t *ret,u8 *header)
{
	int mapper;

	//prg/chr sizes
	ret->prg.size = header[4] * 0x4000;
	ret->chr.size = header[5] * 0x2000;

	//mirroring info
	ret->mirroring = header[6] & 1;
	if(header[6] & 8)
		ret->mirroring = MIRROR_4;

	//battery backed ram
	if(header[6] & 2) {
		ret->battery = 1;
	}

	//mapper (lower bits)
	mapper = (header[6] & 0xF0) >> 4;

	//check if the header is clean
	if(memcmp(&header[8],"\0\0\0\0\0\0\0\0",8) != 0)
		log_printf("parse_ines_header:  dirty header! (%c%c%c%c%c%c%c%c%c)\n",header[7],header[8],header[9],header[10],header[11],header[12],header[13],header[14],header[15]);
	else {
		//mapper upper bits
		mapper |= header[7] & 0xF0;

		//pc10 rom
		if(header[7] & 2)
			ret->pc10rom.size = 8192;
	}

	//print some rom infos
	log_printf("parse_ines_header:  %dkb prg, %dkb chr, mapper %d, %s mirroring\n",
		ret->prg.size / 1024,ret->chr.size / 1024,mapper,
		(ret->mirroring == MIRROR_4) ? "four screen" :
		((ret->mirroring == 0) ? "horizontal" : "vertical"));

	//get internal board id
	ret->mapperid = mapper_get_mapperid_ines(mapper);

	return(0);
}

static int load_chunk(data_t *data,FILE *fp)
{
	int len = 0;

	//if this chunk has a size, it exists (parse_ines_header determines this)
	if(data->size) {
		data->data = (u8*)mem_alloc(data->size);
		len = (int)fread(data->data,1,data->size,fp);
	}
	return(len);
}

int cart_load_ines(cart_t *ret,const char *filename)
{
	u8 header[16];
	FILE *fp;
	u32 size;

	//open rom file
	if((fp = fopen(filename,"rb")) == 0) {
		log_printf("cart_load_ines:  error opening '%s'\n",filename);
		return(1);
	}

	//get length of file
	fseek(fp,0,SEEK_END);
	size = ftell(fp);
	fseek(fp,0,SEEK_SET);

	//read 16 byte header and parse its data
	fread(header,1,16,fp);
	parse_ines_header(ret,header);

	//load each chunk from the file
	load_chunk(&ret->trainer,fp);
	load_chunk(&ret->prg,fp);
	load_chunk(&ret->chr,fp);
	load_chunk(&ret->pc10rom,fp);

	//check for title
	if((size - ftell(fp)) == 128)
		fread(ret->title,1,128,fp);

	//close file and return
	fclose(fp);
	return(0);
}
