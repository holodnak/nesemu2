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
#include "emu/commands.h"
#include "misc/log.h"

#define COMMAND_START	static command_t commands[] = {
#define COMMAND(n)		{"" #n "", command_ ## n},
#define COMMAND_END		{0,0}};

typedef int (*cmdfunc_t)(int argc,char **argv);

typedef struct command_s {
	char *name;
	cmdfunc_t func;
} command_t;

int command_help(int,char**);

COMMAND_START
	COMMAND(help)
	COMMAND(mappers)
	COMMAND(set)
	COMMAND(unset)
	COMMAND(quit)
	COMMAND(load)
	COMMAND(unload)
	COMMAND(reset)
	COMMAND(hardreset)
	COMMAND(readcpu)
	COMMAND(writecpu)
	COMMAND(readppu)
COMMAND_END

COMMAND_FUNC(help)
{
	command_t *c = commands;

	log_printf("available commands:\n\n   ");
	for(c=commands;c->name;c++) {
		log_printf("%s ",c->name);
	}
	return(0);
}

int command_init()
{
	return(0);
}

void command_kill()
{

}

int command_execute(char *str)
{
	int i,argc = 0;
	char *argv[64];		//not more than 64 args!

	//eat whitespace in front
	while(*str == ' ') {
		str++;
	}

	//split up the command line (needs improvement)
	argv[argc] = strtok(str," \t");
	while(argv[argc] != 0) {
		argv[++argc] = strtok(0," \t");
	}
//	log_printf("command is:  '%s' -- %d args\n",str,argc);

	//try to execute the command
	for(i=0;commands[i].name;i++) {
		if(strcmp(argv[0],commands[i].name) == 0) {
			commands[i].func(argc,argv);
			return(0);
		}
	}

	//bad command or file name!
	log_printf("invalid command '%s'\n",argv[0]);
	return(1);
}
