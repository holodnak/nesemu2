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

#include <string.h>
#include "cartdb/cartdb.h"
#include "cartdb/parser.h"
#include "misc/memutil.h"
#include "misc/strutil.h"
#include "misc/config.h"
#include "misc/log.h"
#include "misc/crc32.h"
#include "mappers/mappers.h"
#include "mappers/mapperid.h"

static xml_t *cartxml = 0;

int cartdb_init()
{
	char filename[1024];
	char *str,*str2,*p;
	xml_t *xml;

	config_get_eval_string(filename,"cartdb.filename");
	if(cartxml) {
		return(0);
	}

	str = strtok(filename,";");
	while(str != 0) {
		str2 = mem_strdup(str);
		p = str_eatwhitespace(str2);
		if((xml = parser_load(p)) == 0) {
			log_printf("cartdb_init:  error loading xml cart database '%s'\n",p);
		}
		else {
			log_printf("cartdb_init:  loaded xml cart database '%s'\n",p);
			if(cartxml == 0)
				cartxml = xml;
			else
				parser_merge(cartxml,&xml);
		}
		mem_free(str2);
		str = strtok(0,";");
	}

	if(cartxml == 0) {
		log_printf("cartdb_init:  no xml databases loaded, continuing without using cartdb\n");
		return(0);
	}
		
	return(0);
}

void cartdb_kill()
{
	if(cartxml) {
		parser_free(cartxml);
		parser_verifymemory();
	}
	cartxml = 0;
}

static char *find_attrib(node_t *node,char *name)
{
	attribute_t *attrib;

	if(node == 0)
		return(0);
	attrib = node->attributes;
	while(attrib) {
		if(strcmp(attrib->name,name) == 0) {
			return(attrib->data);
		}
		attrib = attrib->next;
	}
	return(0);
}

static u32 hexstr2int(char *str)
{
	u32 ret = 0;
	size_t len = strlen(str);
	u32 i;
	char ch;

	for(i=0;i<len;i++) {
		ret <<= 4;
		ch = toupper(*str++);
		if(ch >= 'A' && ch <= 'F')
			ch = 10 + ch - 'A';
		else if(ch >= '0' && ch <= '9')
			ch = ch - '0';
		else {
			log_printf("hexstr2int:  invalid character '%c'\n",ch);
			return((u32)-1);
		}
		ret |= ch;
	}
	return(ret);
}

static node_t *find_cart(u32 crc32)
{
	node_t *gamenode,*cartnode;
	char *str;
	u32 n;

	for(gamenode = cartxml->root->child; gamenode; gamenode = gamenode->next) {
		if(strcmp(gamenode->name,"game") != 0)
			continue;
		for(cartnode = gamenode->child; cartnode; cartnode = cartnode->next) {
			if(strcmp(cartnode->name,"cartridge") != 0)
				continue;
			if((str = find_attrib(cartnode,"crc")) != 0) {
				n = hexstr2int(str);
				if(n == crc32)
					return(cartnode);
			}
		}
	}
	return(0);
}

static node_t *get_child(char *name,node_t *parentnode)
{	
	node_t *node;

	for(node = parentnode->child; node; node = node->next) {
		if(strcmp(node->name,name) == 0)
			break;
	}
	return(node);
}

static node_t *get_sibling(char *name,node_t *brothernode)
{	
	node_t *node;

	for(node = brothernode->next; node; node = node->next) {
		if(strcmp(node->name,name) == 0)
			break;
	}
	return(node);
}

//finds the correct mapper id using the information given
static int determine_mapperid(cart_t *cart,char *type,char *mapper,char *chip)
{
	int n,ret = B_UNSUPPORTED;

	//turn mapper string into mapper integer
	n = mapper ? atoi(mapper) : -1;

	//see if this board is supported by the unif mappers
	if((ret = mapper_get_mapperid_unif(type ? type : "")) == B_UNSUPPORTED) {

		//check if this ines mapper is supported
		if((ret = mapper_get_mapperid_ines(n)) == B_UNSUPPORTED) {

			//not supported, use mapperid from the rom loader maybe it is different
			ret = cart->mapperid;
		}
		else {
			log_printf("determine_mapperid:  ines mapper %d supported.  (board '%s')\n",n,type ? type : "");

			//save ines -> unif conversions to add to the unif section
		{{{{{{{
			FILE *fp = fopen("c:\\mingw\\home\\ines2unf.txt","at");

			if(fp) {
				fprintf(fp,"%d = %s\n",n,type ? type : "<UNKNOWN>");
				fclose(fp);
			}
		}}}}}}}

		}
	}
	else
		log_printf("determine_mapperid:  unif board '%s' supported.\n",type);

	//process the board further using chiptype

	//SxROM boards, process mmc1 type
	if(ret == B_NINTENDO_SxROM) {
		if(chip == 0)
			log_printf("determine_mapperid:  using default mmc1 chip (MMC1B)\n");
		else {
			if(strncmp(chip,"MMC1A",5) == 0)
				ret = B_NINTENDO_SxROM_MMC1A;
			else if(strncmp(chip,"MMC1B",5) == 0)
				ret = B_NINTENDO_SxROM_MMC1B;
			else if(strncmp(chip,"MMC1C",5) == 0)
				ret = B_NINTENDO_SxROM_MMC1C;
			log_printf("determine_mapperid:  mmc1 chip type:  %s\n",chip);
		}
	}

	//leave these alone!
	if(n == 93) {
		ret = B_SUNSOFT_2;
		log_printf("determine_mapperid:  sunsoft-3R board.  using sunsoft-2 mapper.\n");
	}

	return(ret);
}

static int sizestr2int(char *str)
{
	char *tmp = strdup(str);
	char *p = tmp;
	int ret = -1;

	//convert to lowercase
	while(*p) {
		*p = tolower(*p);
		p++;
	}

	//search for the k for kilobytes
	if((p = strchr(tmp,'k')) != 0) {
		*p = 0;
		ret = atoi(tmp) * 1024;
	}

	//search for the m for megabytes
	else if((p = strchr(tmp,'m')) != 0) {
		*p = 0;
		ret = atoi(tmp) * 1024 * 1024;
	}

	//must be just bytes (or gigabytes...)
	else {
		ret = atoi(tmp);
	}

	free(tmp);
	return(ret);
}

static int hasbattery(node_t *node)
{
	char *tmp = find_attrib(node,"battery");

	if(tmp && strcmp(tmp,"1") == 0)
		return(1);
	return(0);
}

int cartdb_find(cart_t *cart)
{
	node_t *cartnode,*boardnode,*node;
	u32 crc32,wramsize,vramsize,battery;
	char *str,*str2,*tmp;

	if(cartxml == 0)
		return(1);

	//calculate crc32 of entire image
	crc32 = crc32_block(cart->prg.data,cart->prg.size,0);
	crc32 = crc32_block(cart->chr.data,cart->chr.size,crc32);

	//initialize the size and battery vars
	wramsize = vramsize = 0;
	battery = 0;

	//try to find cart node with same crc32
	cartnode = find_cart(crc32);
	if(cartnode) {

		//copy game title
		if((str = find_attrib(cartnode->parent,"name")))
			strcpy(cart->title,str);

		//see if board node exists
		if((boardnode = get_child("board",cartnode))) {

			//get unif board name and ines mapper number
			str = find_attrib(boardnode,"type");
			str2 = find_attrib(boardnode,"mapper");

			//find the chip used
			node = get_child("chip",boardnode);
			tmp = find_attrib(node,"type");

			//set mapperid with information discovered
			cart->mapperid = determine_mapperid(cart,str,str2,tmp);

			//find the vram size
			node = get_child("vram",boardnode);
			while(node) {
				tmp = find_attrib(node,"size");
				if(tmp) {
					battery |= hasbattery(node) << 1;
					vramsize += sizestr2int(tmp);
				}
				node = get_sibling("vram",node);
			}

			//find the wram size and battery status
			node = get_child("wram",boardnode);
			while(node) {
				tmp = find_attrib(node,"size");
				if(tmp) {
					battery |= hasbattery(node);
					wramsize += sizestr2int(tmp);
				}
				node = get_sibling("wram",node);
			}

			//debug messages
			if(wramsize) {
				cart_setwramsize(cart,wramsize / 1024);
				if(battery & 1)
					cart->battery |= 1;
			}
			if(vramsize) {
				cart_setvramsize(cart,vramsize / 1024);
				if(battery & 2)
					cart->battery |= 2;
			}
			return(0);
		}
	}

	log_printf("cartdb_find:  cart not found in database.\n");
	return(1);
}
