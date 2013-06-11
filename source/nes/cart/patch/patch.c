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
#ifdef WIN32
	#include <io.h>
#else
	#include <unistd.h>
#endif
#include "nes/cart/patch/patch.h"
#include "nes/cart/patch/ips.h"
#include "nes/cart/patch/ups.h"
#include "misc/memutil.h"
#include "misc/log.h"

enum patchformat_e {
	FORMAT_ERROR	= -1,
	FORMAT_UNKNOWN	= 0,
	FORMAT_IPS,
	FORMAT_UPS,
};

static int determineformat(const char *filename)
{
	u8 ident_ips[] = "PATCH\x0";
	u8 ident_ups[] = "UPS1";
	u8 header[6];
	FILE *fp;

	//see if file exists and we can read
	if(access(filename,04) != 0) {
		log_printf("patch.c/determineformat:  file '%s' doesnt exist or isnt readable\n",filename);
		return(FORMAT_ERROR);
	}

	//open filename given
	if((fp = fopen(filename,"rb")) == 0) {
		log_printf("patch.c/determineformat:  error opening '%s'\n",filename);
		return(FORMAT_ERROR);
	}

	//read first 6 bytes and close the file
	fread(header,1,6,fp);
	fclose(fp);

	//check if ips format
	if(memcmp(header,ident_ips,5) == 0) {
		return(FORMAT_IPS);
	}

	//check if ups
	if(memcmp(header,ident_ups,4) == 0)
		return(FORMAT_UPS);

	return(FORMAT_UNKNOWN);
}

//load patch and return its data
patch_t *patch_load(const char *filename)
{
	patch_t *ret = 0;
	int format,n;

	//find out what format the file is
	format = determineformat(filename);

	//print errors for unknown format or error loading
	if(format == FORMAT_UNKNOWN) {
		log_printf("patch_load:  unknown file format in '%s'\n",filename);
		return(0);
	}
	if(format == FORMAT_ERROR) {
		log_printf("patch_load:  error reading '%s'\n",filename);
		return(0);
	}

	//allocate data
	ret = (patch_t*)mem_alloc(sizeof(patch_t));
	memset(ret,0,sizeof(patch_t));

	//load the file
	switch(format) {
		case FORMAT_IPS:	n = patch_load_ips(ret,filename);	break;
		case FORMAT_UPS:	n = patch_load_ups(ret,filename);	break;
	}

	//if error, free data and print error
	if(n != 0) {
		mem_free(ret);
		log_printf("patch_load:  error loading '%s'\n",filename);
		return(0);
	}

	return(ret);
}

void patch_unload(patch_t *p)
{
	patchblock_t *b;

	if(p == 0)
		return;
	while(p->blocks) {
		b = p->blocks;
		p->blocks = p->blocks->next;
		mem_free(b->data);
		mem_free(b);
	}
	mem_free(p);
}

int patch_file(patch_t *p,memfile_t *file)
{
	patchblock_t *b = p->blocks;
	int n;

	while(b) {
		log_printf("patching at %d, %d bytes\n",b->offset,b->size);
		if(memfile_seek(file,b->offset,SEEK_SET) != 0) {
			return(1);
		}
		for(n=0;n<b->size;n++) {
			if(b->type == BLOCK_FILL)
				memfile_putc(b->data[0],file);
			else if(b->type == BLOCK_DATA)
				memfile_putc(b->data[n],file);
		}
		b = b->next;
	}
	return(0);
}
