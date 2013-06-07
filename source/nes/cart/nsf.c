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
	u8 header[10];

	//open bios file
	if((fp = fopen(filename,"rb")) == 0) {
		log_printf("loadbios:  error opening nsf bios '%s'\n",filename);
		return(1);
	}

	//get size of the nsf bios
	fseek(fp,0,SEEK_END);
	size = ftell(fp);
	fseek(fp,0,SEEK_SET);

	//bios has a 10 byte header (actually part of the first bank)
	// addr size description
	// ---- ---- -----------
	//  00   07   ident string (NSFBIOS)
   //  07   01   version hi
   //  08   01   version lo
	//  09   01   <NULL>
	fread(header,1,10,fp);

	if(memcmp(header,"NSFBIOS",7) != 0) {
		log_printf("loadbios:  nsf bios ident is bad\n");
		fclose(fp);
		return(1);
	}

	//seek back to the beginning
	fseek(fp,0,SEEK_SET);

	//load bios into sram
	ret->sram.size = (u32)size;
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
	log_printf("loadbios:  nsf bios loaded, %d bytes.  version %d.%d\n",size,header[7],header[8]);

	//success
	return(0);
}

//makes the size a multiple of 4096 for padding
static u32 padsize(u32 size)
{
	u32 ret = 0;

	while(ret < size) {
		ret += 0x1000;
	}
	return(ret);
}

int cart_load_nsf(cart_t *ret,const char *filename)
{
	int n = 0;
	char biosfile[1024];
	FILE *fp;
	size_t size;
	u32 loadaddr;
	u8 nobankswitch[8 + 8] = {0,0,0,0,0,0,0,0,  0,1,2,3,4,5,6,7};

	//clear the string
	memset(biosfile,0,1024);

	//parse the bios path
	config_get_eval_string(biosfile,"path.bios");

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

	//try to open the nsf
	if((fp = fopen(filename,"rb")) == 0) {
		log_printf("cart_load_nsf:  error opening '%s'\n",filename);
		return(1);
	}

	//get length of file
	fseek(fp,0,SEEK_END);
	size = ftell(fp);
	fseek(fp,0,SEEK_SET);

	//discount for the header
	size -= 0x80;

	if(fread(ret->data,1,0x80,fp) != 0x80) {
		log_printf("cart_load_nsf:  error reading header from '%s'\n",filename);
		n = 1;
	}
	else {
		loadaddr = ret->data[8] | (ret->data[9] << 8);

		//if the nsf doesnt use bankswitching
		if(memcmp((u8*)ret->data + 0x70,(u8*)nobankswitch,8) == 0) {
			memcpy((u8*)ret->data + 0x70,(u8*)nobankswitch + 8,8);
			ret->prg.size = (u32)size + (loadaddr & 0x7FFF);
			ret->prg.data = (u8*)mem_alloc(ret->prg.size);
			memset(ret->prg.data,0,ret->prg.size);
			fread(ret->prg.data + (loadaddr & 0x7FFF),1,size,fp);
		}

		//else the nsf is bankswitched
		else {
			ret->prg.size = (u32)size + (loadaddr & 0xFFF);
			ret->prg.data = (u8*)mem_alloc(ret->prg.size);
			memset(ret->prg.data,0,ret->prg.size);
			fread(ret->prg.data + (loadaddr & 0xFFF),1,size,fp);
		}

		//setup mapper
		ret->mapperid = B_NSF;
		log_printf("cart_load_nsf:  nsf v%d loaded, %d bytes (padded to %d), %d songs.\n",ret->data[5],size,ret->prg.size,ret->data[6]);
		log_printf("init $%04X, play $%04X\n",ret->data[0xA] | (ret->data[0xB] << 8),ret->data[0xC] | (ret->data[0xD] << 8));
	}

	//close file and return
	fclose(fp);
	return(n);
}
