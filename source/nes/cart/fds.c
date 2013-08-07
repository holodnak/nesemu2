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
#include <stdlib.h>
#include "misc/memutil.h"
#include "misc/strutil.h"
#include "misc/log.h"
#include "misc/config.h"
#include "misc/paths.h"
#include "nes/cart/cart.h"
#include "mappers/mapperid.h"

static u8 fdsident[] = "FDS\x1a";
static u8 fdsident2[] = "\x01*NINTENDO-HVC*";

static int loadbios(cart_t *ret,char *filename)
{
	memfile_t *file;
	int n = 0;

	//open bios file
	if((file = memfile_open(filename,"rb")) == 0) {
		log_printf("loadbios:  error opening fds bios '%s'\n",filename);
		return(1);
	}

	//setup prg for bios
	ret->prg.size = 0x2000;
	ret->prg.mask = 0x1FFF;
	ret->prg.data = (u8*)mem_alloc(0x2000);

	//read bios
	if(memfile_read(ret->prg.data,1,0x2000,file) != 0x2000) {
		log_printf("loadbios:  error reading bios file '%s'\n",filename);
		n = 1;
	}
	else
		log_printf("loadbios:  loaded bios file '%s'\n",filename);

	//close bios file handle
	memfile_close(file);

	//return
	return(n);
}

int cart_load_fds(cart_t *ret,memfile_t *file)
{
	u8 header[16];
	u32 size;
	char biosfile[1024];

	//clear the string
	memset(biosfile,0,1024);

	//parse the bios path
	config_get_eval_string(biosfile,"path.bios");

	//append the path seperator
	str_appendchar(biosfile,PATH_SEPERATOR);

	//append the bios filename
	strcat(biosfile,config_get_string("nes.fds.bios"));

	//try to load bios from the bios directory
	if(loadbios(ret,biosfile) != 0) {

		//see if bios is in the current directory
		if(loadbios(ret,config_get_string("nes.fds.bios")) != 0) {
			return(1);
		}
	}

	//get length of file
	size = memfile_size(file);

	//read the header
	memfile_read(header,1,16,file);

	//check if this is raw fds disk
	if(memcmp(header,fdsident2,15) == 0) {

		//check if the file is a valid size
		if((size % 65500) != 0) {
			log_printf("cart_load_fds:  fds disk image size not multiple of 65500, aborting\n");
			return(1);
		}

		//set number of disk sides
//		ret->disksides = size / 65500;

		//skip back to the beginning
		memfile_rewind(file);
	}

	//check if this is 16-byte header fds disk
	else if(memcmp(header,fdsident,4) == 0) {

		//set number of disk sides
//		ret->disksides = header[4];
		size -= 16;

	}

	//set mapper id to fds mapper
	ret->mapperid = B_FDS;

	//setup the disk data pointers
	ret->disk.size = size;
	ret->disk.data = (u8*)mem_alloc(size);
	ret->diskoriginal.size = size;
	ret->diskoriginal.data = (u8*)mem_alloc(size);

	//read disk data into pointer
	memfile_read(ret->disk.data,1,size,file);

	//copy to original disk data pointer
	memcpy(ret->diskoriginal.data,ret->disk.data,size);

	log_printf("cart_load_fds:  loaded disk, %d sides (%d bytes)\n",ret->disk.size / 65500,size);
	return(0);
}
