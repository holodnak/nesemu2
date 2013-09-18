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

static int parse_ines20_header(cart_t *ret,u8 *header)
{
	int mapper,submapper;

	//prg/chr sizes
	ret->prg.size = header[4] * 0x4000;
	ret->chr.size = header[5] * 0x2000;
	ret->prg.size += (((header[9] >> 0) & 0xF) << 8) * 0x4000;
	ret->chr.size += (((header[9] >> 4) & 0xF) << 8) * 0x2000;

	//mirroring info
	ret->mirroring = header[6] & 1;
	if(header[6] & 8)
		ret->mirroring = MIRROR_4;

	//battery backed ram
	if(header[6] & 2) {
		ret->battery = 1;
	}

	//mapper (lower bits, upper bits, then extended bits)
	mapper = (header[6] & 0xF0) >> 4;
	mapper |= header[7] & 0xF0;
	mapper |= (header[8] & 0xF) << 8;

	//submapper number
	submapper = (header[8] & 0xF0) >> 4;

	//wram/sram sizes
	if(header[10] & 0xF)
		ret->wram.size += 0x80 << ((header[10] & 0xF) - 1);

	if(header[10] & 0xF0)
		ret->wram.size += 0x80 << (((header[10] >> 4) & 0xF) - 1);

	//pc10 rom
	if(header[7] & 2)
		ret->pc10rom.size = 8192;

	//print some rom infos
	log_printf("parse_ines20_header:  %dkb prg, %dkb chr, %dkb sram/wram, mapper %d.%d, %s mirroring\n",
		ret->prg.size / 1024,ret->chr.size / 1024,ret->wram.size / 1024,mapper,submapper,
		(ret->mirroring == MIRROR_4) ? "four screen" :
		((ret->mirroring == 0) ? "horizontal" : "vertical"));

	//get internal board id
	ret->mapperid = mapper_get_mapperid_ines20(mapper,submapper);

	return(0);
}

static int load_chunk(data_t *data,memfile_t *file)
{
	int len = 0;

	//if this chunk has a size, it exists (parse_ines_header determines this)
	if(data->size) {
		data->data = (u8*)mem_alloc(data->size);
		len = (int)memfile_read(data->data,1,data->size,file);
	}
	return(len);
}

int cart_load_ines20(cart_t *ret,memfile_t *file)
{
	u8 header[16];
	u32 size;

	//get length of file
	size = memfile_size(file);

	//read 16 byte header and parse its data
	memfile_read(header,1,16,file);
	parse_ines20_header(ret,header);

	//load each chunk from the file
	load_chunk(&ret->trainer,file);
	load_chunk(&ret->prg,file);
	load_chunk(&ret->chr,file);
	load_chunk(&ret->pc10rom,file);

	//check for title
	if((size - memfile_tell(file)) == 128)
		memfile_read(ret->title,1,128,file);

	return(0);
}
