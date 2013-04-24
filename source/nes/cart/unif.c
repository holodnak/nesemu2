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
#include "misc/log.h"
#include "nes/cart/cart.h"
#include "nes/cart/unif.h"
#include "nes/state/block.h"
#include "mappers/mappers.h"

#define BLOCKFUNCSTART()		static blockfunc_t blockfuncs[] = {
#define BLOCKFUNCEND()			{0,0}};
#define BLOCKFUNC(name)			{name,block_##name},
#define BLOCKFUNCDECL(name)	static void block_##name(cart_t *ret,block_t *block)

#define block_rom(n,var,name)	\
	BLOCKFUNCDECL(name##n) {\
		var[0x##n].size = block->size;\
		var[0x##n].data = (u8*)malloc(block->size);\
		memcpy(var[0x##n].data,block->data,block->size);\
	}
#define block_crc(n,var,name)	\
	BLOCKFUNCDECL(name##n) {\
		memcpy(&var[0x##n].crc32,block->data,4);\
	}

#define block_prg(n)		block_rom(n,prg,ID_PRG)
#define block_chr(n)		block_rom(n,chr,ID_CHR)
#define block_pck(n)		block_crc(n,prg,ID_PCK)
#define block_cck(n)		block_crc(n,chr,ID_CCK)

typedef struct blockfunc_s {
	u32 type;
	void (*func)(cart_t*,block_t*);
} blockfunc_t;

typedef struct romdata_s {
	u32	size;
	u32	crc32;
	u8		*data;
} romdata_t;

static const char ident[] = "UNIF";
static romdata_t prg[16],chr[16];
static char board[128];

BLOCKFUNCDECL(ID_MAPR)	{	ret->mapperid = mapper_get_mapperid_unif(block->data); strcpy(board,block->data);	}
BLOCKFUNCDECL(ID_NAME)	{	strncpy(ret->title,block->data,CART_TITLE_LEN);	}
BLOCKFUNCDECL(ID_MIRR)	{	ret->mirroring = block->data[0];	}
BLOCKFUNCDECL(ID_BATR)	{	ret->battery = block->data[0];	}
BLOCKFUNCDECL(ID_TVCI)	{	ret->tvmode = block->data[0];		}

block_prg(0);	block_prg(1);	block_prg(2);	block_prg(3);	block_prg(4);	block_prg(5);	block_prg(6);	block_prg(7);
block_prg(8);	block_prg(9);	block_prg(A);	block_prg(B);	block_prg(C);	block_prg(D);	block_prg(E);	block_prg(F);
block_chr(0);	block_chr(1);	block_chr(2);	block_chr(3);	block_chr(4);	block_chr(5);	block_chr(6);	block_chr(7);
block_chr(8);	block_chr(9);	block_chr(A);	block_chr(B);	block_chr(C);	block_chr(D);	block_chr(E);	block_chr(F);
block_pck(0);	block_pck(1);	block_pck(2);	block_pck(3);	block_pck(4);	block_pck(5);	block_pck(6);	block_pck(7);
block_pck(8);	block_pck(9);	block_pck(A);	block_pck(B);	block_pck(C);	block_pck(D);	block_pck(E);	block_pck(F);
block_cck(0);	block_cck(1);	block_cck(2);	block_cck(3);	block_cck(4);	block_cck(5);	block_cck(6);	block_cck(7);
block_cck(8);	block_cck(9);	block_cck(A);	block_cck(B);	block_cck(C);	block_cck(D);	block_cck(E);	block_cck(F);

BLOCKFUNCSTART()
	BLOCKFUNC(ID_MAPR)
	BLOCKFUNC(ID_NAME)
	BLOCKFUNC(ID_MIRR)
	BLOCKFUNC(ID_BATR)
	BLOCKFUNC(ID_TVCI)
	BLOCKFUNC(ID_PRG0)	BLOCKFUNC(ID_PRG1)	BLOCKFUNC(ID_PRG2)	BLOCKFUNC(ID_PRG3)
	BLOCKFUNC(ID_PRG4)	BLOCKFUNC(ID_PRG5)	BLOCKFUNC(ID_PRG6)	BLOCKFUNC(ID_PRG7)
	BLOCKFUNC(ID_PRG8)	BLOCKFUNC(ID_PRG9)	BLOCKFUNC(ID_PRGA)	BLOCKFUNC(ID_PRGB)
	BLOCKFUNC(ID_PRGC)	BLOCKFUNC(ID_PRGD)	BLOCKFUNC(ID_PRGE)	BLOCKFUNC(ID_PRGF)
	BLOCKFUNC(ID_CHR0)	BLOCKFUNC(ID_CHR1)	BLOCKFUNC(ID_CHR2)	BLOCKFUNC(ID_CHR3)
	BLOCKFUNC(ID_CHR4)	BLOCKFUNC(ID_CHR5)	BLOCKFUNC(ID_CHR6)	BLOCKFUNC(ID_CHR7)
	BLOCKFUNC(ID_CHR8)	BLOCKFUNC(ID_CHR9)	BLOCKFUNC(ID_CHRA)	BLOCKFUNC(ID_CHRB)
	BLOCKFUNC(ID_CHRC)	BLOCKFUNC(ID_CHRD)	BLOCKFUNC(ID_CHRE)	BLOCKFUNC(ID_CHRF)
BLOCKFUNCEND()

static int load_unif_block(cart_t *ret,FILE *fp)
{
	block_t *block = block_load(fp);
	int i;

	if(block == 0) {
		log_printf("load_unif_block:  error loading block\n");
		return(-1);
	}
	for(i=0;blockfuncs[i].type;i++) {
		if(blockfuncs[i].type == block->type) {
			blockfuncs[i].func(ret,block);
			break;
		}
	}
	return(block->size + 8);
}

static void glue_data(data_t *rom,romdata_t *roms)
{
	u32 size,pos;
	int i;

	for(size=0,i=0;i<16;i++) {
		size += roms[i].size;
	}
	rom->size = size;
	rom->data = (u8*)malloc(size);
	for(pos=0,i=0;i<16;i++) {
		memcpy(rom->data + pos,roms[i].data,roms[i].size);
		free(roms[i].data);
		pos += roms[i].size;
	}
}

int cart_load_unif(cart_t *ret,const char *filename)
{
	u8 header[32];
	FILE *fp;
	u32 pos,size,tmp;

	//open rom file
	if((fp = fopen(filename,"rb")) == 0) {
		log_printf("cart_load_unif:  error opening '%s'\n",filename);
		return(1);
	}

	//get file size
	fseek(fp,0,SEEK_END);
	size = ftell(fp);
	fseek(fp,0,SEEK_SET);

	//read 32 byte header
	fread(header,1,32,fp);
	pos = 32;

	//clear the prg/chr rom data
	memset(prg,0,sizeof(romdata_t)*16);
	memset(chr,0,sizeof(romdata_t)*16);

	//clear the board name
	memset(board,0,128);

	//load the unif blocks
	while(pos < size) {
		if((tmp = load_unif_block(ret,fp)) == -1)
			break;
		pos += tmp;
	}

	//check the crc32's (if blocks are present)
//	for(i=0;i<16;i++) {
//	}

	//glue together the prg/chr data
	glue_data(&ret->prg,prg);
	glue_data(&ret->chr,chr);

	//tile cache stuff
	if(ret->chr.size) {
		//allocate memory for the tile cache
		ret->cache = (cache_t*)malloc(ret->chr.size);
		ret->cache_hflip = (cache_t*)malloc(ret->chr.size);

		//convert all chr tiles to cache tiles
		cache_tiles(ret->chr.data,ret->cache,ret->chr.size / 16,0);
		cache_tiles(ret->chr.data,ret->cache_hflip,ret->chr.size / 16,1);
	}

	log_printf("cart_load_unif:  loaded ok.  %dkb prg, %dkb chr, board '%s'\n",
		ret->prg.size / 1024,ret->chr.size / 1024,board);

	//close file and return
	fclose(fp);
	return(0);
}	
