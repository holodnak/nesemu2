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
#include "log/log.h"
#include "nes/cart/cart.h"
#include "nes/cart/unif.h"
#include "nes/state/block.h"
#include "mappers/mappers.h"

#define makeindex(n)	((n >= '0' && n <= '9') ? (n - '0') : ((n >= 'A' && n <= 'F') ? (n - 'A' + 10) : -1))

typedef struct blockfunc_s {
	u32 type;
	void (*func)(cart_t*,block_t*);
} blockfunc_t;

typedef struct rominfo_s {
	u32	size;
	u32	crc32;
	u8		*data;
} rominfo_t;

static char ident[] = "UNIF";
static rominfo_t prg[16],chr[16];

static void block_mapr(cart_t *ret,block_t *block)	{	ret->mapperid = mapper_get_mapperid_unif(block->data);	}
static void block_name(cart_t *ret,block_t *block)	{	strncpy(ret->title,block->data,CART_TITLE_LEN);	}
static void block_mirr(cart_t *ret,block_t *block)	{	ret->mirroring = block->data[0];	}
static void block_batr(cart_t *ret,block_t *block)	{	ret->battery = block->data[0];	}
static void block_tvci(cart_t *ret,block_t *block)	{	ret->tvmode = block->data[0];		}

#define block_rom(n,var)	\
	static void block_##var##n(cart_t *ret,block_t *block) {\
		var[n].size = block->size;\
		var[n].data = (u8*)malloc(block->size);\
		memcpy(var[n].data,block->data,block->size);\
	}
#define block_crc(n,var,name)	\
	static void block_##name##n(cart_t *ret,block_t *block) {\
		memcpy(&var[n].crc32,block->data,4);\
	}

#define block_prg(n)			block_rom(n,prg)
#define block_chr(n)			block_rom(n,chr)
#define block_pck(n)			block_crc(n,prg,pck)
#define block_cck(n)			block_crc(n,chr,cck)

block_prg(0);	block_prg(1);	block_prg(2);	block_prg(3);	block_prg(4);	block_prg(5);	block_prg(6);	block_prg(7);
block_prg(8);	block_prg(9);	block_prg(10);	block_prg(11);	block_prg(12);	block_prg(13);	block_prg(14);	block_prg(15);
block_chr(0);	block_chr(1);	block_chr(2);	block_chr(3);	block_chr(4);	block_chr(5);	block_chr(6);	block_chr(7);
block_chr(8);	block_chr(9);	block_chr(10);	block_chr(11);	block_chr(12);	block_chr(13);	block_chr(14);	block_chr(15);
block_pck(0);	block_pck(1);	block_pck(2);	block_pck(3);	block_pck(4);	block_pck(5);	block_pck(6);	block_pck(7);
block_pck(8);	block_pck(9);	block_pck(10);	block_pck(11);	block_pck(12);	block_pck(13);	block_pck(14);	block_pck(15);
block_cck(0);	block_cck(1);	block_cck(2);	block_cck(3);	block_cck(4);	block_cck(5);	block_cck(6);	block_cck(7);
block_cck(8);	block_cck(9);	block_cck(10);	block_cck(11);	block_cck(12);	block_cck(13);	block_cck(14);	block_cck(15);

static blockfunc_t blockfuncs[] = {
	{ID_MAPR,	block_mapr},
	{ID_NAME,	block_name},
	{ID_MIRR,	block_mirr},
	{ID_BATR,	block_batr},
	{ID_TVCI,	block_tvci},
	{ID_PRG0,	block_prg0},
	{ID_PRG1,	block_prg1},
	{ID_PRG2,	block_prg2},
	{ID_PRG3,	block_prg3},
	{ID_PRG4,	block_prg4},
	{ID_PRG5,	block_prg5},
	{ID_PRG6,	block_prg6},
	{ID_PRG7,	block_prg7},
	{ID_PRG8,	block_prg8},
	{ID_PRG9,	block_prg9},
	{ID_PRGA,	block_prg10},
	{ID_PRGB,	block_prg11},
	{ID_PRGC,	block_prg12},
	{ID_PRGD,	block_prg13},
	{ID_PRGE,	block_prg14},
	{ID_PRGF,	block_prg15},
	{ID_CHR0,	block_chr0},
	{ID_CHR1,	block_chr1},
	{ID_CHR2,	block_chr2},
	{ID_CHR3,	block_chr3},
	{ID_CHR4,	block_chr4},
	{ID_CHR5,	block_chr5},
	{ID_CHR6,	block_chr6},
	{ID_CHR7,	block_chr7},
	{ID_CHR8,	block_chr8},
	{ID_CHR9,	block_chr9},
	{ID_CHRA,	block_chr10},
	{ID_CHRB,	block_chr11},
	{ID_CHRC,	block_chr12},
	{ID_CHRD,	block_chr13},
	{ID_CHRE,	block_chr14},
	{ID_CHRF,	block_chr15},
	{ID_PCK0,	block_pck0},
	{ID_PCK1,	block_pck1},
	{ID_PCK2,	block_pck2},
	{ID_PCK3,	block_pck3},
	{ID_PCK4,	block_pck4},
	{ID_PCK5,	block_pck5},
	{ID_PCK6,	block_pck6},
	{ID_PCK7,	block_pck7},
	{ID_PCK8,	block_pck8},
	{ID_PCK9,	block_pck9},
	{ID_PCKA,	block_pck10},
	{ID_PCKB,	block_pck11},
	{ID_PCKC,	block_pck12},
	{ID_PCKD,	block_pck13},
	{ID_PCKE,	block_pck14},
	{ID_PCKF,	block_pck15},
	{ID_CCK0,	block_cck0},
	{ID_CCK1,	block_cck1},
	{ID_CCK2,	block_cck2},
	{ID_CCK3,	block_cck3},
	{ID_CCK4,	block_cck4},
	{ID_CCK5,	block_cck5},
	{ID_CCK6,	block_cck6},
	{ID_CCK7,	block_cck7},
	{ID_CCK8,	block_cck8},
	{ID_CCK9,	block_cck9},
	{ID_CCKA,	block_cck10},
	{ID_CCKB,	block_cck11},
	{ID_CCKC,	block_cck12},
	{ID_CCKD,	block_cck13},
	{ID_CCKE,	block_cck14},
	{ID_CCKF,	block_cck15},
	{0,			0},
};

static int load_unif_block(cart_t *ret,FILE *fp)
{
	block_t *block = block_load(fp);
	int i;

	if(block == 0)
		return(-1);
	for(i=0;blockfuncs[i].type;i++) {
		if(blockfuncs[i].type == block->type) {
			blockfuncs[i].func(ret,block);
			break;
		}
	}
	printf("loaded '%s'\n",&block->type);
	return(block->size + 8);
}

int cart_load_unif(cart_t *ret,const char *filename)
{
	u8 header[32];
	FILE *fp;
	u32 i,pos,size;

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
	memset(prg,0,sizeof(rominfo_t)*16);
	memset(chr,0,sizeof(rominfo_t)*16);

	//load the unif blocks
	while(pos < size) {
		if((i = load_unif_block(ret,fp)) == -1)
			break;
		pos += i;
	}

	//glue together the prg/chr data
	for(size=0,i=0;i<16;i++) {
		size += prg[i].size;
	}
	ret->prg.size = size;
	ret->prg.data = (u8*)malloc(size);
	for(pos=0,i=0;i<16;i++) {
		memcpy(ret->prg.data + pos,prg[i].data,prg[i].size);
		free(prg[i].data);
		pos += prg[i].size;
	}
	log_printf("prg size = %d\n",size);

	for(size=0,i=0;i<16;i++) {
		size += chr[i].size;
	}
	ret->chr.size = size;
	ret->chr.data = (u8*)malloc(size);
	for(pos=0,i=0;i<16;i++) {
		memcpy(ret->chr.data + pos,chr[i].data,chr[i].size);
		free(chr[i].data);
		pos += chr[i].size;
	}
	log_printf("chr size = %d\n",ret->chr.size);

	//tile cache stuff
	if(ret->chr.size) {
		//allocate memory for the tile cache
		ret->cache = (cache_t*)malloc(ret->chr.size);
		ret->cache_hflip = (cache_t*)malloc(ret->chr.size);

		//convert all chr tiles to cache tiles
		cache_tiles(ret->chr.data,ret->cache,ret->chr.size / 16,0);
		cache_tiles(ret->chr.data,ret->cache_hflip,ret->chr.size / 16,1);
	}

	//close file and return
	fclose(fp);
	return(0);
}	
