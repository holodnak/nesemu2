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
#include "misc/config.h"
#include "misc/log.h"
#include "misc/crc32.h"
#include "mappers/mappers.h"
#include "mappers/mapperid.h"

static xml_t *cartxml = 0;

int cartdb_init()
{
	char *filename = config_get_eval_string("cartdb.filename");

	if(cartxml) {
		return(0);
	}
	if((cartxml = parser_load(filename)) == 0) {
		log_printf("cartdb_init:  error loading xml cart database '%s'\n",filename);
		log_printf("cartdb_init:  continuing without using it\n");
		return(0);

	}
	log_printf("cartdb_init:  loaded xml cart database '%s'\n",filename);
	return(0);
}

void cartdb_kill()
{
	if(cartxml)
		parser_free(cartxml);
	cartxml = 0;
}

static char *find_attrib(node_t *node,char *name)
{
	attribute_t *attrib;

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

static node_t *get_boardnode(node_t *cartnode)
{	
	node_t *node;

	for(node = cartnode->child; node; node = node->next) {
		if(strcmp(node->name,"board") == 0)
			break;
	}
	return(node);
}

int cartdb_find(cart_t *cart)
{
	node_t *cartnode,*boardnode;
	u32 crc32,n,id;
	char *str,*str2;

	if(cartxml == 0)
		return(1);

	//calculate crc32 of entire image
	crc32 = crc32_block(cart->prg.data,cart->prg.size,0);
	crc32 = crc32_block(cart->chr.data,cart->chr.size,crc32);

	//try to find cart node with same crc32
	cartnode = find_cart(crc32);
	if(cartnode) {

		//copy game title
		if((str = find_attrib(cartnode->parent,"name")))
			strcpy(cart->title,str);

		//see if board node exists
		if((boardnode = get_boardnode(cartnode))) {

			//get unif board name and ines mapper number
			str = find_attrib(boardnode,"type");
			str2 = find_attrib(boardnode,"mapper");
			n = str2 ? atoi(str2) : -1;

			//see if this board is supported by the unif mappers
			if((id = mapper_get_mapperid_unif(str ? str : "")) == B_UNSUPPORTED) {

				//check if this ines mapper is supported
				if((id = mapper_get_mapperid_ines(n)) == B_UNSUPPORTED) {

					//not supported, use mapperid from the rom loader maybe it is different
					id = cart->mapperid;
				}
				else {
					log_printf("cartdb_find:  cart found.  ines mapper %d supported.  (board '%s')\n",n,str ? str : "");

					//save ines -> unif conversions to add to the unif section
				{{{{{{{
					FILE *fp = fopen("c:\\mingw\\home\\ines2unf.txt","at");

					if(fp) {
						fprintf(fp,"%d = %s\n",n,str ? str : "<UNKNOWN>");
						fclose(fp);
					}
				}}}}}}}
				}
			}
			else
				log_printf("cartdb_find:  cart found.  unif board '%s' supported.\n",str);

			cart->mapperid = id;
			return(0);
		}
	}

	log_printf("cartdb_find:  cart not found in database.\n");
	return(1);
}
