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
#include "misc/config.h"
#include "nes/cart/cart.h"
#include "mappers/mapperid.h"

static u8 fdsident[] = "FDS\x1a";
static u8 fdsident2[] = "\x01*NINTENDO-HVC*";

int cart_load_fds(cart_t *ret,const char *filename)
{
	u8 header[16];
	FILE *fp;
	u32 size;
	char *biosfile;

	//get bios filename
	biosfile = config_get_string("fds.bios","disksys.rom");

	//open bios file
	if((fp = fopen(biosfile,"rb")) == 0) {
		log_printf("cart_load_fds:  error opening fds bios '%s'\n",biosfile);
		return(1);
	}

	//setup prg for bios
	ret->prg.size = 0x2000;
	ret->prg.data = (u8*)mem_alloc(0x2000);

	//read bios
	if(fread(ret->prg.data,1,0x2000,fp) != 0x2000) {
		log_printf("cart_load_fds:  error reading bios file '%s'\n",biosfile);
		fclose(fp);
		return(1);
	}

	//close bios file handle
	fclose(fp);
	log_printf("cart_load_fds:  loaded bios file '%s'\n",biosfile);

	//open disk file
	if((fp = fopen(filename,"rb")) == 0) {
		log_printf("cart_load_fds:  error opening '%s'\n",filename);
		return(1);
	}

	//get length of file
	fseek(fp,0,SEEK_END);
	size = ftell(fp);
	fseek(fp,0,SEEK_SET);

	//read the header
	fread(header,1,16,fp);

	//check if this is raw fds disk
	if(memcmp(header,fdsident2,15) == 0) {

		//check if the file is a valid size
		if((size % 65500) != 0) {
			log_printf("cart_load_fds:  fds disk image size not multiple of 65500, aborting\n");
			fclose(fp);
			return(1);
		}

		//set number of disk sides
//		ret->disksides = size / 65500;

		//skip back to the beginning
		fseek(fp,0,SEEK_SET);
	}

	//check if this is 16-byte header fds disk
	else if(memcmp(header,fdsident,4) == 0) {

		//set number of disk sides
//		ret->disksides = header[4];

	}

	ret->mapperid = B_FDS;

	//setup the disk data pointers
	ret->disk.size = size;
	ret->disk.data = (u8*)mem_alloc(size);
	ret->diskoriginal.size = size;
	ret->diskoriginal.data = (u8*)mem_alloc(size);

	//read disk data into pointer
	fread(ret->disk.data,1,size,fp);

	//copy to original disk data pointer
	memcpy(ret->diskoriginal.data,ret->disk.data,size);

	log_printf("cart_load_fds:  loaded disk, %d sides\n",ret->disk.size / 65500);
	return(0);
}
