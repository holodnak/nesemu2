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
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "log/log.h"
#include "log/buffer.h"
#include "system/system.h"
#include "version.h"

#ifndef MAX_PATH
	#define MAX_PATH	1024
#endif

#define LOGFILENAME "nesemu2.log"

static FILE *logfd = 0;
static char logfilename[MAX_PATH] = LOGFILENAME;

int log_init()
{
	if(logfd) {
		printf("log_init:  already initialized\n");
		return(0);
	}
	sprintf(logfilename,"%s/%s",system_getcwd(),LOGFILENAME);
	if((logfd = fopen(logfilename,"wt")) == 0) {
		printf("log_init:  error opening log file '%s'\n",logfilename);
		return(1);
	}
	log_printf("log_init:  log initialized.  nesemu2 v"VERSION"\n");
	return(0);
}

void log_kill()
{
	log_buffer_flush();
	if(logfd)
		fclose(logfd);
	logfd = 0;
}

void log_print(char *str)
{
	//output message to console
	printf(str);

	//if log file isnt open, maybe logging is disabled or file isnt opened yet
	if(logfd == 0)
		return;

	//write to log file
	fputs(str,logfd);

	//flush file
	fflush(logfd);
}

void log_printf(char *str,...)
{
	char buffer[1024];						//buffer to store vsprintf'd message in
	va_list argptr;							//argument data

	va_start(argptr,str);					//get data
	vsprintf(buffer,str,argptr);			//print to buffer
	va_end(argptr);							//done!
	log_print(buffer);
}
