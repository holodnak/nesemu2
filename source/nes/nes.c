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
#include "misc/log.h"
#include "misc/config.h"
#include "nes/nes.h"
#include "nes/io.h"
#include "nes/memory.h"
#include "nes/genie.h"
#include "nes/state/state.h"

nes_t nes;

//kludge (or possibly not)
static void mapper_state(int mode,u8 *data)	{	nes.mapper->state(mode,data);		}

//non-kludges
static void wram_state(int mode,u8 *data)		{	STATE_ARRAY_U8(nes.cart->wram.data,nes.cart->wram.size);		}
static void sram_state(int mode,u8 *data)		{	STATE_ARRAY_U8(nes.cart->sram.data,nes.cart->sram.size);		}
static void vram_state(int mode,u8 *data)
{
	STATE_ARRAY_U8(nes.cart->vram.data,nes.cart->vram.size);
	if(mode == STATE_LOAD) {
		cache_tiles(nes.cart->vram.data,nes.cart->vcache,nes.cart->vram.size / 16,0);
		cache_tiles(nes.cart->vram.data,nes.cart->vcache_hflip,nes.cart->vram.size / 16,1);
	}
}

static void svram_state(int mode,u8 *data)
{
	STATE_ARRAY_U8(nes.cart->svram.data,nes.cart->svram.size);
	if(mode == STATE_LOAD) {
		cache_tiles(nes.cart->svram.data,nes.cart->svcache,nes.cart->svram.size / 16,0);
		cache_tiles(nes.cart->svram.data,nes.cart->svcache_hflip,nes.cart->svram.size / 16,1);
	}
}

int nes_init()
{
	int ret = 0;
	
	memset(&nes,0,sizeof(nes_t));
	nes_set_inputdev(0,I_NULL);
	nes_set_inputdev(1,I_NULL);
	nes_set_inputdev(2,I_NULL);
	state_init();
	state_register(B_NES,nes_state);
	state_register(B_MAPR,mapper_state);
	state_register(B_WRAM,wram_state);
	state_register(B_SRAM,sram_state);
	state_register(B_VRAM,vram_state);
	state_register(B_SVRAM,svram_state);
	ret += cpu_init();
	ret += ppu_init();
	ret += apu_init();
	return(ret);
}

void nes_kill()
{
	state_kill();
	cpu_kill();
	ppu_kill();
	apu_kill();
	cart_unload(nes.cart);
	genie_unload();
}

int nes_load_cart(cart_t *c)
{
	mapper_t *m;

	if(c->mapperid < 0) {
		log_printf("nes_load_cart:  cart with unsupported mapperid loaded (id = %d)\n",c->mapperid);
		return(1);
	}
	if((m = mapper_init(c->mapperid)) == 0) {
		log_printf("nes_load_cart:  error calling mapper_init\n");
		return(1);
	}
	log_printf("nes_load_cart:  success\n");
	nes.cart = c;
	nes.mapper = m;
	return(0);
}

int nes_load(char *filename)
{
	cart_t *c;
	int ret;

	//make sure cart is unload
	nes_unload();

	//try to load the file into a cart_t struct
	if((c = cart_load(filename)) == 0) {
		log_printf("nes_load:  error loading '%s'\n",filename);
		return(1);
	}

	if(strlen(c->title))
		log_printf("nes_load:  loaded file '%s' (title = '%s')\n",filename,c->title);
	else
		log_printf("nes_load:  loaded file '%s'\n",filename);

	//check cartdb here

	//see if the nes accepts it (mapper is supported)
	if((ret = nes_load_cart(c)) != 0)
		cart_unload(c);
	return(ret);
}

void nes_unload()
{
	if(nes.cart)
		cart_unload(nes.cart);
	nes.cart = 0;
	nes.mapper = 0;
}

void nes_set_inputdev(int n,int id)
{
	if(n < 2)
		nes.inputdev[n] = inputdev_get(id);
	else
		nes.expdev = inputdev_get(id);
}

void nes_reset(int hard)
{
	int i;

	//zero out all read/write pages/functions
	for(i=0;i<64;i++) {
		nes.cpu.readfuncs[i] = 0;
		nes.cpu.readpages[i] = 0;
		nes.cpu.writefuncs[i] = 0;
		nes.cpu.writepages[i] = 0;
		nes.ppu.readfuncs[i / 4] = 0;
		nes.ppu.readpages[i / 4] = 0;
		nes.ppu.writefuncs[i / 4] = 0;
		nes.ppu.writepages[i / 4] = 0;
	}

	//setup read/write funcs
	mem_setreadfunc(2,ppu_read);
	mem_setreadfunc(3,ppu_read);
	mem_setreadfunc(4,nes_read_4000);
	mem_setwritefunc(2,ppu_write);
	mem_setwritefunc(3,ppu_write);
	mem_setwritefunc(4,nes_write_4000);

	//set cart mirroring
	mem_setmirroring(nes.cart->mirroring);

	//setup default cpu/ppu read/write funcs
	cpu_setreadfunc(0);
	cpu_setwritefunc(0);
	ppu_setreadfunc(0);
	ppu_setwritefunc(0);

	//load in the game genie if it is enabled and we doing hard reset
	if(config->nes.gamegenie.enabled != 0) {
		if(hard)
			genie_load();
	}
	else {
		//make sure we dont have a game genie block in the state if it is disabled
		genie_unload();
	}

	//reset the mapper
	nes.mapper->reset(hard);

	//reset the cpu, ppu, and apu
	ppu_reset(hard);
	cpu_reset(hard);
	apu_reset(hard);

	//clear some memory for hard reset
	if(hard) {
		memset(nes.cpu.ram,0,0x800);
		memset(nes.ppu.nametables,0,0x800);
		memset(nes.ppu.oam,0,256);
		memset(nes.ppu.palette,0,32);
	}
}

void nes_frame()
{
	cpu_execute_frame();
}

void nes_state(int mode,u8 *data)
{
	STATE_U8(nes.strobe);
}

void nes_savestate(char *filename)
{
	FILE *fp;

	if((fp = fopen(filename,"wb")) == 0) {
		log_printf("nes_savestate:  error opening file '%s'\n",filename);
		return;
	}
	log_printf("nes_savestate:  saving state to '%s'\n",filename);
	state_save(fp);
	fclose(fp);
}

void nes_loadstate(char *filename)
{
	FILE *fp;

	if((fp = fopen(filename,"rb")) == 0) {
		log_printf("nes_loadstate:  error opening file '%s'\n",filename);
		return;
	}
	log_printf("nes_loadstate:  loading state from '%s'\n",filename);
	state_load(fp);
	fclose(fp);
}
