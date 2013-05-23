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
#include "nes/cart/cart.h"
#include "nes/cart/nsf.h"
#include "misc/log.h"
#include "misc/paths.h"
#include "misc/config.h"
#include "misc/memutil.h"
#include "mappers/mapperid.h"

static int loadbios(cart_t *ret,char *filename)
{
	FILE *fp;
	size_t size;
	u8 header[12];

	//open bios file
	if((fp = fopen(filename,"rb")) == 0) {
		log_printf("loadbios:  error opening nsf bios '%s'\n",filename);
		return(1);
	}

	//get size of the nsf bios
	fseek(fp,0,SEEK_END);
	size = ftell(fp);
	fseek(fp,0,SEEK_SET);

	//bios has a 12 byte header:
	// addr size description
	// ---- ---- -----------
	//  00   07   ident string (NSFBIOS)
   //  07   01   version hi
   //  08   01   version lo
	//  09   01   number of 1kb prg banks
	//  0A   02   chr size in bytes
	fread(header,1,12,fp);

	if(memcmp(header,"NSFBIOS",7) != 0) {
		log_printf("loadbios:  nsf bios ident is bad\n");
		fclose(fp);
		return(1);
	}

	//load bios prg into sram, load the chr into chr
	ret->sram.size = header[9] * 1024;
	ret->sram.mask = ret->sram.size - 1;
	ret->sram.data = (u8*)mem_alloc(ret->sram.size);

	//read bios
	if(fread(ret->sram.data,1,ret->sram.size,fp) != ret->sram.size) {
		log_printf("loadbios:  error reading nsf bios file '%s'\n",filename);
		fclose(fp);
		return(1);
	}

	//close bios file handle
	fclose(fp);
	log_printf("loadbios:  nsf bios loaded.  version %d.%d\n",header[7],header[8]);

	//success
	return(0);
}

int cart_load_nsf(cart_t *ret,const char *filename)
{
	nsf_t header;
	char biosfile[1024];

	//clear the string
	memset(biosfile,0,1024);

	//parse the bios path
	paths_parse(config->path.bios,biosfile,1024);

	//append the path seperator
	biosfile[strlen(biosfile)] = PATH_SEPERATOR;

	//append the bios filename
	strcat(biosfile,"nsfbios.bin");

	//try to load bios from the bios directory
	if(loadbios(ret,biosfile) != 0) {

		//see if bios is in the current directory
		if(loadbios(ret,"nsfbios.bin") != 0) {
			return(1);
		}
	}

	ret->mapperid = B_NSF;

	return(0);
}
