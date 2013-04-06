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
#include <stdlib.h>
#include <string.h>
#include "log/log.h"
#include "nes/cart/cart.h"
#include "nes/cart/ines.h"
#include "nes/cart/ines20.h"

#define FREE(p) {	\
	if(p) {			\
		free(p);		\
		p = 0;		\
	}					\
}
#define FREE_DATA(d) {	\
	FREE(d.data);			\
	d.size = 0;				\
	d.mask = 0;				\
}

#define FORMAT_ERROR		-1
#define FORMAT_UNKNOWN	0
#define FORMAT_INES		1
#define FORMAT_INES20	2
#define FORMAT_UNIF		3
#define FORMAT_FDS		4
#define FORMAT_RAWFDS	5
#define FORMAT_NSF		6
#define FORMAT_SPLIT		7

//thanks tnse for this, long ago
static u32 createmask(u32 size)
{
	u32 x,y,result;

	result = size - 1;
	for(x=0;x<32;x++) {
		y = 1 << x;
		if(y == size)
			return(result);
		if(y > size) {
			result = y - 1;
			return(result);
		}
	}
	return(0xFFFFFFFF);
}

int determineformat(const char *filename)
{
	u8 ident_ines[] = "NES\x1a";
	u8 ident_unif[] = "UNIF";
	u8 ident_fds[] = "FDS\x1a";
	u8 ident_rawfds[] = "\x01*NINTENDO-HVC*";
	u8 ident_nsf[] = "NESM\x1a";
	u8 header[16];
	FILE *fp;

	//open filename given
	if((fp = fopen(filename,"rb")) == 0) {
		log_printf("determineformat:  error opening '%s'\n",filename);
		return(FORMAT_ERROR);
	}

	//read first 16 bytes and close the file
	fread(header,1,16,fp);
	fclose(fp);

	//check if ines format
	if(memcmp(header,ident_ines,4) == 0) {
		if((header[15] == 0) && ((header[7] & 0xC) == 0x8))
			return(FORMAT_INES20);
		return(FORMAT_INES);
	}

	//check if unif
	if(memcmp(header,ident_unif,4) == 0)
		return(FORMAT_UNIF);

	//check if fds disk
	if(memcmp(header,ident_fds,4) == 0)
		return(FORMAT_FDS);

	//check if raw fds disk
	if(memcmp(header,ident_rawfds,15) == 0)
		return(FORMAT_RAWFDS);

	//check if raw fds disk
	if(memcmp(header,ident_nsf,5) == 0)
		return(FORMAT_NSF);

	return(FORMAT_UNKNOWN);
}

cart_t *cart_load(const char *filename)
{
	cart_t *ret = 0;
	int format,n;

	//find out what format the file is
	format = determineformat(filename);

	//print errors for unknown format or error loading
	if(format == FORMAT_UNKNOWN) {
		log_printf("cart_load:  unknown file format in '%s'\n",filename);
		return(0);
	}
	if(format == FORMAT_ERROR) {
		log_printf("cart_load:  error reading '%s'\n",filename);
		return(0);
	}

	//allocate memory for struct
	ret = (cart_t*)malloc(sizeof(cart_t));
	memset(ret,0,sizeof(cart_t));

	//load the file
	switch(format) {
		case FORMAT_INES:		n = cart_load_ines(ret,filename);	break;
		case FORMAT_INES20:	n = cart_load_ines20(ret,filename);	break;
//		case FORMAT_UNIF:		n = cart_load_unif(ret,filename);	break;
//		case FORMAT_FDS:		n = cart_load_fds(ret,filename);		break;
//		case FORMAT_RAWFDS:	n = cart_load_fds(ret,filename);		break;
//		case FORMAT_NSF:		n = cart_load_nsf(ret,filename);		break;
//		case FORMAT_SPLIT:	n = cart_load_split(ret,filename);	break;
	}

	//if error, free data and print error
	if(n != 0) {
		free(ret);
		log_printf("cart_load:  error loading '%s'\n",filename);
		return(0);
	}

	//rom loaded ok, create necessary masks
	ret->prg.mask = createmask(ret->prg.size);
	ret->chr.mask = createmask(ret->chr.size);
	ret->trainer.mask = createmask(ret->trainer.size);
	ret->pc10rom.mask = createmask(ret->pc10rom.size);

	//finished
	return(ret);
}

void cart_unload(cart_t *r)
{
	if(r) {
		FREE_DATA(r->prg);
		FREE_DATA(r->chr);
		FREE_DATA(r->vram);
		FREE_DATA(r->wram);
		FREE_DATA(r->sram);
		FREE_DATA(r->svram);
		FREE_DATA(r->trainer);
		FREE_DATA(r->pc10rom);
		FREE(r);
	}
}

static void allocdata(data_t *data,int len)
{
	//if size hasnt changed, return
	if(data->size == len)
		return;

	//set size and create mask
	data->size = len;
	data->mask = createmask(len);

	//allocate (or reallocate) memory
	if(data->data == 0)
		data->data = malloc(len);
	else
		data->data = realloc(data->data,len);
}

void cart_setsramsize(cart_t *r,int banks)
{
	allocdata(&r->sram,banks * 1024);
	log_printf("cart_setsramsize:  sram size set to %dkb\n",banks);
}

void cart_setwramsize(cart_t *r,int banks)
{
	allocdata(&r->wram,banks * 1024);
	log_printf("cart_setwramsize:  wram size set to %dkb\n",banks);
}

void cart_setvramsize(cart_t *r,int banks)
{
	allocdata(&r->vram,banks * 1024);
	log_printf("cart_setvramsize:  vram size set to %dkb\n",banks);
}

void cart_setsvramsize(cart_t *r,int banks)
{
	allocdata(&r->vram,banks * 1024);
	log_printf("cart_setvramsize:  vram size set to %dkb\n",banks);
}
