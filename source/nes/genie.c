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
#include "nes/nes.h"
#include "nes/genie.h"
#include "nes/state/state.h"
#include "mappers/mapperinc.h"
#include "misc/memutil.h"
#include "misc/config.h"
#include "misc/log.h"
#include "misc/paths.h"

//holds code information
typedef struct codedata_s {
	u16 addr;
	u8 compare,data;
} codedata_t;

//ines ident for determining if the genie rom has ines header
static u8 inesident[4] = {'N','E','S',0x1A};

//genie rom image (prg + chr)
static u8 *genierom = 0;

//genie chr cache
static cache_t *geniecache = 0;

//old read function pointers
static readfunc_t readcpu;

//codes/register
static codedata_t code[3];
static u8 reg;

//kludge?
static void nullcycle() {}

//declare the mapper here
MAPPER(B_GENIE,genie_reset,nullcycle,nullcycle,genie_state);

//read handler for the cheats
static u8 genie_read_cheat(u32 addr)
{
	u8 ret = readcpu(addr);

	//reading from rom, lets cheat!
	if(addr >= 0x8000) {
		if((reg & 0x10) == 0 && addr == code[0].addr && ((reg & 2) == 0 || ret == code[0].compare)) {
			log_printf("applying code 0, addr = %04X\n",addr);
			return(code[0].data);
		}
		if((reg & 0x20) == 0 && addr == code[1].addr && ((reg & 4) == 0 || ret == code[1].compare)) {
			log_printf("applying code 1, addr = %04X\n",addr);
			return(code[1].data);
		}
		if((reg & 0x40) == 0 && addr == code[2].addr && ((reg & 8) == 0 || ret == code[2].compare)) {
			log_printf("applying code 2, addr = %04X\n",addr);
			return(code[2].data);
		}
	}

	return(ret);
}

static void genie_write(u32 addr,u8 data)
{
	if(addr >= 0x8000) {
		switch(addr) {
			case 0x8000:
				if((data & 1) == 0) {

					//unmap the write function
					mem_setwritefunc(8,0);

					//we not using genie anymore
					reg &= 0x7F;

					//save old read func
					readcpu = cpu_getreadfunc();

					//setup new read function
					cpu_setreadfunc(genie_read_cheat);

					//restore the old mapper
					nes->mapper = mapper_init(nes->cart->mapperid);

					//reset the mapper
					nes->mapper->reset(1);
				}
				else {
					reg = 0x80 | (data & 0x7E);
					log_printf("genie_write:  reg = $%02X (data = $%02X)\n",reg,data);
					log_printf("  code 0:  %s, %s\n",(reg & 0x10) ? "disabled" : "enabled",(reg & 2) ? "compare" : "no compare");
					log_printf("  code 1:  %s, %s\n",(reg & 0x20) ? "disabled" : "enabled",(reg & 4) ? "compare" : "no compare");
					log_printf("  code 2:  %s, %s\n",(reg & 0x40) ? "disabled" : "enabled",(reg & 8) ? "compare" : "no compare");
				}
				break;
			case 0x8001:	code[0].addr = (code[0].addr & 0x00FF) | (data << 8);		break;
			case 0x8002:	code[0].addr = (code[0].addr & 0xFF00) | (data << 0);		break;
			case 0x8003:	code[0].compare = data;		break;
			case 0x8004:	code[0].data = data;			break;
			case 0x8005:	code[1].addr = (code[1].addr & 0x00FF) | (data << 8);		break;
			case 0x8006:	code[1].addr = (code[1].addr & 0xFF00) | (data << 0);		break;
			case 0x8007:	code[1].compare = data;		break;
			case 0x8008:	code[1].data = data;			break;
			case 0x8009:	code[2].addr = (code[2].addr & 0x00FF) | (data << 8);		break;
			case 0x800A:	code[2].addr = (code[2].addr & 0xFF00) | (data << 0);		break;
			case 0x800B:	code[2].compare = data;		break;
			case 0x800C:	code[2].data = data;			break;
		}
		log_printf("genie write:  $%04X = $%02X\n",addr,data);
	}
}

int genie_loadrom(char *filename)
{
	FILE *fp;
	size_t len;
	u8 buf[16];

	//try to open the file
	if((fp = fopen(filename,"rb")) == 0) {
		log_printf("genie_load:  error opening game genie rom '%s'\n",filename);
		return(1);
	}

	//read 16 bytes to see if it has a ines header
	fread(buf,1,16,fp);

	//get the file size of the file
	fseek(fp,0,SEEK_END);
	len = ftell(fp);
	fseek(fp,0,SEEK_SET);

	//allocate memory for genierom/geniecache
	genierom = (u8*)mem_alloc(0x1000 + 0x400);
	geniecache = (cache_t*)mem_alloc(0x400);

	//see if there is an ines header, we know we have 16kb prg, 8kb chr
	if(memcmp(buf,inesident,4) == 0) {
		fseek(fp,16,SEEK_SET);
		fread(genierom,1,0x1000,fp);
		fseek(fp,16 + 0x4000,SEEK_SET);
		fread(genierom + 0x1000,1,0x400,fp);
	}

	//we have no header and 16kb prg, 8kb chr
	else if(len == 0x6000) {
		fread(genierom,1,0x1000,fp);
		fseek(fp,0x4000,SEEK_SET);
		fread(genierom + 0x1000,1,0x400,fp);
	}

	//if we have raw prg/chr rom dump 4kb prg, 1kb chr
	else if(len == 0x1400) {
		fread(genierom,1,0x1400,fp);
	}

	//now what?  error!
	else {
		fclose(fp);
		genie_unload();
		log_printf("genie_load:  unable to load genie rom '%s'\n",filename);
		return(1);
	}

	//cache the genie chr
	cache_tiles(genierom + 0x1000,geniecache,64,0);

	//close the file, output message, return
	fclose(fp);
	log_printf("genie_load:  loaded game genie rom '%s' (%d bytes)\n",filename,len);
	return(0);
}

int genie_load()
{
	char biosfile[1024];

	//if the rom isnt loaded, we need to do that now
	if(genierom == 0) {

		//clear the string
		memset(biosfile,0,1024);

		//parse the bios path
		paths_parse(config->path.bios,biosfile,1024);

		//append the path seperator
		biosfile[strlen(biosfile)] = PATH_SEPERATOR;

		//append the bios filename
		strcat(biosfile,config->nes.gamegenie.bios);

		//try to load bios from the bios directory
		if(genie_loadrom(biosfile) != 0) {

			//see if bios is in the current directory
			if(genie_loadrom(config->nes.gamegenie.bios) != 0) {
				return(1);
			}
		}

		//register the save state stuff
		state_register(B_GG,genie_state);
	}

	//load in the genie mapper
	nes->mapper = &mapperB_GENIE;

	return(0);
}

void genie_unload()
{
	if(genierom) {
		state_unregister(B_GG);
		mem_free(genierom);
		mem_free(geniecache);
		genierom = 0;
		geniecache = 0;
		log_printf("genie_unload:  unloaded game genie.\n");
	}
}

void genie_reset(int hard)
{
	int i;

	//manually map in the prg and set write handlers
	for(i=8;i<16;i++) {
		mem_setreadptr(i,genierom);
		mem_setwritefunc(8,genie_write);
	}

	//manually map the game genie chr/cache
	for(i=0;i<8;i++) {
		nes->ppu.readpages[i] = genierom + 0x1000;
		nes->ppu.writepages[i] = 0;
		nes->ppu.cachepages[i] = geniecache;
		nes->ppu.cachepages_hflip[i] = geniecache;
	}

	//setup genie registers
	memset(code,0,sizeof(codedata_t) * 3);
	reg = 0x80;

	log_printf("genie_reset:  ok\n");
}

void genie_state(int mode,u8 *data)
{
	STATE_U8(reg);
	STATE_U16(code[0].addr);
	STATE_U8(code[0].compare);
	STATE_U8(code[0].data);
	STATE_U16(code[1].addr);
	STATE_U8(code[1].compare);
	STATE_U8(code[1].data);
	STATE_U16(code[2].addr);
	STATE_U8(code[2].compare);
	STATE_U8(code[2].data);
}
