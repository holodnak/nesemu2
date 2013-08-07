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
#include "misc/paths.h"
#include "misc/memutil.h"
#include "misc/strutil.h"
#include "misc/config.h"
#include "misc/log.h"
#include "emu/emu.h"
#include "system/main.h"

char *paths_normalize(char *str)
{
	char *p;

	for(p=str;*p;p++) {
		if(*p == '/' || *p == '\\')
			*p = PATH_SEPERATOR;
	}
	return(str);
}

void paths_makestatefilename(char *romfilename,char *dest,int len)
{
	char *p,*tmp = mem_strdup(romfilename);

	//clear the string
	memset(dest,0,len);

	//parse the state path
	config_get_eval_string(dest,"path.state");

	//append the path seperator
	str_appendchar(dest,PATH_SEPERATOR);

	//normalize the path seperators
	paths_normalize(tmp);

	//find the last path seperator
	p = strrchr(tmp,PATH_SEPERATOR);

	//if not found then it is plain filename
	p = (p == 0) ? tmp : p + 1;

	//append the rom filename
	strcat(dest,p);

	//append the state extension)
	strcat(dest,".state");

	//free the temporary string
	mem_free(tmp);
}
