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
#include <ctype.h>
#include "emu/commands.h"
#include "misc/log.h"
#include "misc/config.h"
#include "nes/nes.h"

//!!!!!! kludge alert !!!!!!
extern int running;

static u32 str2int(char *str)
{
	u32 i,ret = 0;
	size_t len;
	char ch;
	int base = 10;

	if(*str == '$') {
		base = 16;
		str++;
	}
	else if(str[0] == '0' && tolower(str[1]) == 'x') {
		str += 2;
		base = 16;
	}

	len = strlen(str);

	for(i=0;i<len;i++) {
		ret *= base;
		ch = toupper(*str++);
		if(ch >= 'A' && ch <= 'F')
			ch = 10 + ch - 'A';
		else if(ch >= '0' && ch <= '9')
			ch = ch - '0';
		else {
			return((u32)-1);
		}
		ret += ch;
	}
	return(ret);
}

COMMAND_FUNC(load)
{
	//need a way to load the functions common to all systems/targets
	if(nes_load(argv[1]) == 0) {
		nes_reset(1);
		running = config_get_bool("nes.pause_on_load") ? 0 : 1;
	}
	return(0);
}

COMMAND_FUNC(unload)
{
	CHECK_CART();
	nes_unload();
	running = 0;
	return(0);
}

COMMAND_FUNC(reset)
{
	CHECK_CART();
	nes_reset(0);
	return(0);
}

COMMAND_FUNC(hardreset)
{
	CHECK_CART();
	nes_reset(1);
	return(0);
}

COMMAND_FUNC(readcpu)
{
	u32 addr;

	CHECK_ARGS(2,"usage:  readcpu <addr>\n");
	CHECK_CART();
	addr = str2int(argv[1]);
	if(addr == (u32)-1) {
		log_printf("invalid address\n");
	}
	else {
		log_printf("$%04X = $%02X\n",addr,cpu_read(addr));
	}
	return(0);
}

COMMAND_FUNC(writecpu)
{
	u32 addr,data;

	CHECK_ARGS(3,"usage:  writecpu <addr> <data>\n");
	CHECK_CART();
	addr = str2int(argv[1]);
	data = str2int(argv[2]);
	if(addr == (u32)-1) {
		log_printf("invalid address\n");
	}
	else if(data == (u32)-1) {
		log_printf("invalid data\n");
	}
	else {
		log_printf("$%04X = $%02X\n",addr,data);
		cpu_write(addr,(u8)data);
	}
	return(0);
}

COMMAND_FUNC(readppu)
{
	u32 addr,size,n;

	CHECK_ARGS(2,"usage:  readppu <addr> [size]\n");
	CHECK_CART();
	addr = str2int(argv[1]);
	if(addr == (u32)-1) {
		log_printf("invalid address\n");
	}
	else {
		if(argc >= 3) {
			size = str2int(argv[2]);
			for(n=0;n<size;n++) {
				log_printf("$%02X ",ppu_memread(addr+n));
				if((n & 0xF) == 0xF)
					log_printf("\n");
			}
		}
		else {
			log_printf("$%04X = $%02X\n",addr,ppu_memread(addr));
		}
	}
	return(0);
}
