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

#ifndef __commands_h__
#define __commands_h__

#include "types.h"

#define CHECK_ARGS(num,msg)	\
	if(argc < (num)) {			\
		log_printf(msg);			\
		return(1);					\
	}

#define CHECK_CART()							\
	if(nes.cart == 0) {						\
		log_printf("no rom loaded!\n");	\
		return(1);								\
	}

#define COMMAND_FUNC(n)	int command_ ## n (int argc,char **argv)
#define COMMAND_DECL(n)	COMMAND_FUNC(n);

//general commands
COMMAND_DECL(mappers);

//nes commands
COMMAND_DECL(load);
COMMAND_DECL(unload);
COMMAND_DECL(reset);
COMMAND_DECL(hardreset);
COMMAND_DECL(readcpu);
COMMAND_DECL(writecpu);
COMMAND_DECL(loadstate);
COMMAND_DECL(savestate);

int command_execute(char *str);

#endif
