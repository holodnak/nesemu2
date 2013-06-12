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
#include "misc/memutil.h"
#include "misc/log.h"
#include "misc/paths.h"
#include "misc/crc32.h"
#include "misc/memfile.h"
#include "nes/cart/cart.h"
#include "nes/cart/ines.h"
#include "nes/cart/ines20.h"
#include "nes/cart/unif.h"
#include "nes/cart/fds.h"
#include "nes/cart/nsf.h"
#include "nes/cart/patch/patch.h"

#define FREE(p) {	\
	if(p) {			\
		mem_free(p);		\
		p = 0;		\
	}					\
}
#define FREE_DATA(d) {	\
	FREE(d.data);			\
	d.size = 0;				\
	d.mask = 0;				\
}

#define generate_mask_and_crc32(d)	\
	d.mask = createmask(d.size);		\
	d.crc32 = crc32(d.data,d.size);

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

static int determineformat(memfile_t *file)
{
	u8 ident_ines[] = "NES\x1a";
	u8 ident_unif[] = "UNIF";
	u8 ident_fds[] = "FDS\x1a";
	u8 ident_rawfds[] = "\x01*NINTENDO-HVC*";
	u8 ident_nsf[] = "NESM\x1a";
	u8 header[16];

	//read first 16 bytes and reset curpos
	memfile_read(header,1,16,file);
	memfile_rewind(file);

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
	return(cart_load_patched(filename,0));
}

cart_t *cart_load_patched(const char *filename,const char *patchfilename)
{
	cart_t *ret = 0;
	int format,n;
	memfile_t *file;

	//try to open file
	if((file = memfile_open((char*)filename,"rb")) == 0) {
		log_printf("cart_load:  error opening '%s'\n",filename);
		return(0);
	}

	//find out what format the file is
	format = determineformat(file);

	//print errors for unknown format or error loading
	if(format == FORMAT_UNKNOWN) {
		log_printf("cart_load:  unknown file format in '%s'\n",filename);
		memfile_close(file);
		return(0);
	}
	if(format == FORMAT_ERROR) {
		log_printf("cart_load:  error reading '%s'\n",filename);
		memfile_close(file);
		return(0);
	}

	//allocate memory for struct
	ret = (cart_t*)mem_alloc(sizeof(cart_t));
	memset(ret,0,sizeof(cart_t));

	//if patch filename was passed, load the patch and patch the file in memory
	if(patchfilename) {
		if((ret->patch = patch_load(patchfilename))) {
			patch_apply(ret->patch,file);
			memfile_seek(file,0,SEEK_SET);
		}
	}

	//load the file
	switch(format) {
		case FORMAT_INES:		n = cart_load_ines(ret,file);		break;
		case FORMAT_INES20:	n = cart_load_ines20(ret,file);	break;
		case FORMAT_UNIF:		n = cart_load_unif(ret,file);		break;
		case FORMAT_FDS:
		case FORMAT_RAWFDS:	n = cart_load_fds(ret,file);		break;
		case FORMAT_NSF:		n = cart_load_nsf(ret,file);		break;
//		case FORMAT_SPLIT:	n = cart_load_split(ret,file);	break;
	}

	//if error, free data and print error
	if(n != 0) {
		mem_free(ret);
		memfile_close(file);
		log_printf("cart_load:  error loading '%s'\n",filename);
		return(0);
	}

	//tile cache stuff
	if(ret->chr.size) {
		//allocate memory for the tile cache
		ret->cache = (cache_t*)mem_alloc(ret->chr.size);
		ret->cache_hflip = (cache_t*)mem_alloc(ret->chr.size);

		//convert all chr tiles to cache tiles
		cache_tiles(ret->chr.data,ret->cache,ret->chr.size / 16,0);
		cache_tiles(ret->chr.data,ret->cache_hflip,ret->chr.size / 16,1);
	}

	//see if title exists and clean it up
	if(strlen(ret->title)) {
		char *p = ret->title;
		int i;

		for(i=0;i<512;i++,p++) {
			if(*p == 0x0D || *p == 0x0A) {
				*p = ' ';
				continue;
			}
			if(*p == 0x1A) {
				*p = 0;
				break;
			}
		}
	}

	//rom loaded ok, create necessary masks and generate crc32's
	generate_mask_and_crc32(ret->prg);
	generate_mask_and_crc32(ret->chr);
	generate_mask_and_crc32(ret->trainer);
	generate_mask_and_crc32(ret->pc10rom);

	//store the filename
	if(ret->patch) {
		char *p;

		paths_normalize(ret->patch->filename);
		p = strrchr(ret->patch->filename,PATH_SEPERATOR);
		if(p == 0)
			p = ret->patch->filename;
		else
			p++;
		ret->filename = (char*)mem_alloc(strlen(filename) + strlen(p) + 2);
		sprintf(ret->filename,"%s+%s",filename,p);
	}
	else
		ret->filename = mem_strdup((char*)filename);
	log_printf("cart_load:  rom filename is '%s'\n",ret->filename);

	//close file
	memfile_close(file);

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
		FREE_DATA(r->disk);
		FREE_DATA(r->diskoriginal);
		FREE(r->cache);
		FREE(r->cache_hflip);
		FREE(r->vcache);
		FREE(r->vcache_hflip);
		FREE(r->svcache);
		FREE(r->svcache_hflip);
		FREE(r->filename);
		patch_unload(r->patch);
		FREE(r);
	}
}

#define ram_alloc(size,ptr)	(u8*)(ptr ? mem_realloc(ptr,size) : mem_alloc(size))

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
		data->data = mem_alloc(len);
	else
		data->data = mem_realloc(data->data,len);

	//zero out the newly allocated memory
	memset(data->data,0,len);
}

void cart_setsramsize(cart_t *r,int banks)
{
	allocdata(&r->sram,banks * 0x1000);
	log_printf("cart_setsramsize:  sram size set to %dkb\n",banks * 0x1000 / 1024);
}

void cart_setwramsize(cart_t *r,int banks)
{
	allocdata(&r->wram,banks * 0x1000);
	log_printf("cart_setwramsize:  wram size set to %dkb\n",banks * 0x1000 / 1024);
}

void cart_setvramsize(cart_t *r,int banks)
{
	allocdata(&r->vram,banks * 1024);
	log_printf("cart_setvramsize:  vram size set to %dkb\n",banks);

	//tile cache data
	r->vcache = (cache_t*)ram_alloc(r->vram.size,r->vcache);
	r->vcache_hflip = (cache_t*)ram_alloc(r->vram.size,r->vcache_hflip);
}

void cart_setsvramsize(cart_t *r,int banks)
{
	allocdata(&r->svram,banks * 1024);
	log_printf("cart_setsvramsize:  svram size set to %dkb\n",banks);

	//tile cache data
	r->svcache = (cache_t*)ram_alloc(r->svram.size,r->svcache);
	r->svcache_hflip = (cache_t*)ram_alloc(r->svram.size,r->svcache_hflip);
}
