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

#include <stdlib.h>
#include <string.h>
#include "log/log.h"
#include "nes/nes.h"
#include "nes/memory.h"
#include "nes/ppu/ppu.h"

nes_t nes;

int nes_init()
{
	int ret = 0;
	
	memset(&nes,0,sizeof(nes_t));
	ret += cpu_init();
	ret += ppu_init();
	return(ret);
}

void nes_kill()
{
	cpu_kill();
	ppu_kill();
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

	//make sure cart is unload
	nes_unload();

	//try to load the file into a cart_t struct
	if((c = cart_load(filename)) == 0) {
		log_printf("nes_load:  error loading '%s'\n",filename);
		return(1);
	}

	log_printf("nes_load:  loaded file '%s'\n",filename);

	//see if the nes accepts it and return
	return(nes_load_cart(c));
}

void nes_unload()
{
	if(nes.cart)
		cart_unload(nes.cart);
	nes.cart = 0;
	nes.mapper = 0;
}

u8 apu_read(u32 addr)
{
	return(0);
}

void apu_write(u32 addr,u8 data)
{

}

void nes_reset(int hard)
{
	int i;

log_printf("1");

	//zero out all read/write pages/functions
	for(i=0;i<16;i++) {
		nes.cpu.readfuncs[i] = 0;
		nes.cpu.readpages[i] = 0;
		nes.cpu.writefuncs[i] = 0;
		nes.cpu.writepages[i] = 0;
		nes.ppu.readpages[i] = 0;
		nes.ppu.writepages[i] = 0;
	}

log_printf("2");

	//setup read/write funcs
	mem_setreadfunc(2,ppu_read);
	mem_setreadfunc(3,ppu_read);
	mem_setreadfunc(4,apu_read);
	mem_setwritefunc(2,ppu_write);
	mem_setwritefunc(3,ppu_write);
	mem_setwritefunc(4,apu_write);

log_printf("3");

	//set cart mirroring
	mem_setmirroring(nes.cart->mirroring);

	//reset the mapper, cpu, and ppu
log_printf("4");
	nes.mapper->reset(hard);
log_printf("5");
	cpu_reset(hard);
log_printf("6");
	ppu_reset(hard);
	//clear some memory for hard reset
	if(hard) {
		memset(nes.cpu.ram,0,0x800);
		memset(nes.ppu.nametables,0,0x800);
		memset(nes.ppu.oam,0,256);
		memset(nes.ppu.palette,0,32);
		memset(nes.ppu.screen,0,256*240*sizeof(u16));
	}
}

void nes_frame()
{
	u32 curframe = nes.ppu.frames;

	log_printf("executing frame %d\n",curframe);
	while(nes.ppu.frames == curframe) {
		cpu_step();
	}
}
