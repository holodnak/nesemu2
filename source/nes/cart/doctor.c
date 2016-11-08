/***************************************************************************
 *   Copyright (C) 2013-2016 by James Holodnak                             *
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
#include <stdlib.h>
#include "misc/memutil.h"
#include "misc/strutil.h"
#include "misc/log.h"
#include "misc/config.h"
#include "misc/paths.h"
#include "nes/cart/cart.h"
#include "mappers/mapperid.h"

static u8 fdsident2[] = "\x01*NINTENDO-HVC*";

static int loadbios_new(char *filename, u8 **data, int *size)
{
	memfile_t *file;
	int n = 0;

	*size = 0;
	*data = 0;

	//open bios file
	if ((file = memfile_open(filename, "rb")) == 0) {
		log_printf("loadbios:  error opening '%s'\n", filename);
		return(1);
	}

	//setup prg for bios
	*size = 0x2000;
	*data = (u8*)mem_alloc(*size);

	//read bios
	if (memfile_read(*data, 1, 0x2000, file) != 0x2000) {
		log_printf("loadbios:  error reading bios file '%s'\n", filename);
		n = 1;
	}
	else
		log_printf("loadbios:  loaded bios file '%s'\n", filename);

	//close bios file handle
	memfile_close(file);

	//return
	return(n);
}

static int loadbios(cart_t *ret, char *filename)
{
	memfile_t *file;
	int n = 0;

	//open bios file
	if ((file = memfile_open(filename, "rb")) == 0) {
		log_printf("loadbios:  error opening fds bios '%s'\n", filename);
		return(1);
	}

	//setup prg for bios
	ret->prg.size = 0x4000;
	ret->prg.mask = 0x3FFF;
	ret->prg.data = (u8*)mem_alloc(ret->prg.size);

	//read bios
	if (memfile_read(ret->prg.data, 1, 0x2000, file) != 0x2000) {
		log_printf("loadbios:  error reading bios file '%s'\n", filename);
		n = 1;
	}
	else
		log_printf("loadbios:  loaded bios file '%s'\n", filename);

	//close bios file handle
	memfile_close(file);

	//return
	return(n);
}

void copy_block(u8 *dst, u8 *src, int size) {
	memcpy(dst, src, size);
}

int cart_load_doctor(cart_t *ret, memfile_t *file)
{
	u8 *tmpbuf, *ptr;
	u8 gdheader[3];
	u8 header[16];
	u32 size;
	int i, o;
	char biosfile[1024];
	char drbiosfile[1024];

	//clear the string
	memset(biosfile, 0, 1024);

	//parse the bios path
	config_get_eval_string(biosfile, "path.bios");
	config_get_eval_string(drbiosfile, "path.bios");

	//append the path seperator
	str_appendchar(biosfile, PATH_SEPERATOR);
	str_appendchar(drbiosfile, PATH_SEPERATOR);

	//append the bios filename
	strcat(biosfile, config_get_string("nes.fds.bios"));
	strcat(drbiosfile, "dr6+_rom.bin");

	//try to load bios from the bios directory
/*	if (loadbios(ret, biosfile) != 0) {

		//see if bios is in the current directory
		if (loadbios(ret, config_get_string("nes.fds.bios")) != 0) {
			return(1);
		}
	}*/

	u8 *bios, *drbios;
	int bioslen, drbioslen;

	loadbios_new(biosfile, &bios, &bioslen);
	loadbios_new(drbiosfile, &drbios, &drbioslen);

	ret->prg.size = 0x4000;
	ret->prg.mask = 0x3FFF;
	ret->prg.data = (u8*)mem_alloc(0x4000);
	memcpy(ret->prg.data + 0x0000, bios, 0x2000);
	memcpy(ret->prg.data + 0x2000, drbios, 0x2000);

	mem_free(bios);
	mem_free(drbios);

	//get length of file
	size = memfile_size(file);

	//read the game doctor header and following fds header
	memfile_read(gdheader, 1, 3, file);
	memfile_read(header, 1, 16, file);

	//check if disk is valid
	if (memcmp(header, fdsident2, 15) == 0) {

		//skip back to the beginning
		memfile_seek(file, 3, SEEK_SET);
	}

	//bad disk
	else {
		log_printf("cart_load_doctor:  bad gamedoctor image.\n");
		return(1);
	}

	//set mapper id to game doctor mapper
	ret->mapperid = B_DOCTOR;

	//setup the disk data pointers
	ret->disk.size = size;
	ret->disk.data = (u8*)mem_alloc(size);
	ret->diskoriginal.size = 0;
	ret->diskoriginal.data = 0;

	//convert the format to fds format
	tmpbuf = (u8*)mem_alloc(size);
	memfile_read(tmpbuf, 1, size, file);

	ptr = ret->disk.data;
	i = 0;
	o = 0;

	memcpy(ptr + o, tmpbuf + i, 0x38);
	i += 0x38 + 2;
	o += 0x38;

	memcpy(ptr + o, tmpbuf + i, 2);
	i += 2 + 2;
	o += 2;

	while (tmpbuf[i] == 3) {

		int size = (tmpbuf[i + 13] | (tmpbuf[i + 14] << 8)) + 1;

		memcpy(ptr + o, tmpbuf + i, 16);
		i += 16 + 2;
		o += 16;

		memcpy(ptr + o, tmpbuf + i, size);
		i += size + 2;
		o += size;
	}

	log_printf("cart_load_doctor:  loaded disk, %d sides (%d bytes)\n", ret->disk.size / 65500, size);
	mem_free(tmpbuf);
	return(0);
}
