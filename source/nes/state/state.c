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
#include "nes/nes.h"
#include "nes/state/state.h"
#include "nes/state/block.h"

#define readvar(var,sz)	\
	if(fread(&var,1,sz,fp) != sz) {	\
		log_printf("state_load:  error reading ##var##\n");	\
		return(1);	\
	}

#define writevar(var,sz)	\
	if(fwrite(&var,1,sz,fp) != sz) {	\
		log_printf("state_save:  error writing ##var##\n");	\
		return(1);	\
	}

typedef struct blockfunc_s {
	u32 type;
	u32 size;
	void (*func)(int,u8*);
} blockfunc_t;

static blockfunc_t blockinfo[16];
static u32 ident = MAKEID('N','S','T','\0');
static u32 version = 0x0100;

int state_init()
{
	memset(blockinfo,0,sizeof(blockfunc_t) * 16);
	return(0);
}

void state_kill()
{
}

void state_register(u32 type,void (*func)(int,u8*))
{
	int i;

	for(i=0;i<16;i++) {
		if(blockinfo[i].type == type) {
			log_printf("state_register:  type '%08X' already registered\n",type);
			return;
		}
		if(blockinfo[i].type == 0)
			break;
	}
	blockinfo[i].type = type;
	blockinfo[i].func = func;
}

int state_load(FILE *fp)
{
	stateheader_t header;
	u32 type,size;
	u8 *data;
	int i;

	readvar(header.ident,4);
	readvar(header.version,2);
	readvar(header.flags,2);
	readvar(header.usize,4);
	readvar(header.csize,4);
	readvar(header.crc32,4);

	printf("state_load:  state header loaded.  version %04X\n",header.version);

	while(feof(fp) == 0) {
		char blockname[5] = {0,0,0,0,0};

		fread(&type,1,4,fp);
		fread(&size,1,4,fp);
		if(feof(fp))
			break;

		memcpy(blockname,&type,4);
		printf("loading block '%s' (%08X) (%d bytes)\n",blockname,type,size);
		data = (u8*)malloc(size);
		fread(data,1,size,fp);
		for(i=0;blockinfo[i].type;i++) {
			if(blockinfo[i].type == type) {
				blockinfo[i].func(STATE_LOAD,data);
				break;
			}
		}
		if(blockinfo[i].type == 0) {
			printf("no handler for block type '%s'\n",blockname);
		}
		free(data);
	}

	return(0);
}

int state_save(FILE *fp)
{
	stateheader_t header;
	block_t *block;
	int i;

	//clear the state info
	memset(&header,0,sizeof(stateheader_t));

	//set the ident/version of the state header
	header.ident = ident;
	header.version = version;

	//calculate total data size
	for(i=0;blockinfo[i].type;i++) {
		blockinfo[i].size = 0;
		blockinfo[i].func(STATE_SIZE,(u8*)&blockinfo[i].size);
		header.usize += blockinfo[i].size;
	}

	//write the state header
	writevar(header.ident,4);
	writevar(header.version,2);
	writevar(header.flags,2);
	writevar(header.usize,4);
	writevar(header.csize,4);
	writevar(header.crc32,4);

	//write each block
	for(i=0;blockinfo[i].type;i++) {
		char blockname[5] = {0,0,0,0,0};

		if(blockinfo[i].size == 0)
			continue;
		memcpy(blockname,&blockinfo[i].type,4);
		printf("saving block '%s' (%08X) (%d bytes)\n",blockname,blockinfo[i].type,blockinfo[i].size);
		block = block_create(blockinfo[i].type,blockinfo[i].size);
		blockinfo[i].func(STATE_SAVE,block->data);
		fwrite(&blockinfo[i].type,1,4,fp);
		fwrite(&blockinfo[i].size,1,4,fp);
		fwrite(block->data,1,blockinfo[i].size,fp);
		block_destroy(block);
	}

	return(0);
}
