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

static int determineformat(memfile_t *file)
{
	u8 ident_ips[] = "PATCH\x0";
	u8 ident_ups[] = "UPS1";
	u8 header[6];

	//read first 6 bytes and seek to beginning
	memfile_rewind(file);
	memfile_read(header,1,6,file);
	memfile_rewind(file);

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
	return(memfile_open((char*)filename,"rb"));
}

patch_t *patch_load_memory(u8 *data,u32 size)
{
	return(memfile_open_memory(data,size));
}

void patch_unload(patch_t *p)
{
	if(p)
		memfile_close(p);
}

int patch_apply(patch_t *p,memfile_t *file)
{
	int format,n;

	log_printf("patch_apply:  applying patch...\n");

	//find out what format the file is
	switch(determineformat(p)) {
		case FORMAT_IPS:	n = patch_apply_ips(p,file);	break;
		case FORMAT_UPS:	n = patch_apply_ups(p,file);	break;
		default:
			log_printf("patch_apply:  bad patch format.\n");
			n = 1;
			break;
	}

	return(n);
}
