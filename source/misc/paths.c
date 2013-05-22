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
#include "misc/config.h"
#include "emu/emu.h"

char *paths_parse(char *src,char *dest,int len)
{
	int error = 0;
	char *tmp,*p,*p2;
	char varname[64],varname2[70];
	int pos;

	//make a copy of the string
	tmp = mem_strdup(src);

	//clear the destination string
	memset(dest,0,len);

	for(pos=0,p=tmp;*p;p++) {

		//see if we find a '%'
		if(*p == '%') {

			//skip over the '%'
			p++;

			//clear area to hold var name
			memset(varname,0,64);

			//see if it is missing the '%'
			if((p2 = strchr(p,'%')) == 0) {
				printf("missing ending '%'\n");
				error = 1;
				break;
			}

			//terminate the substring
			*p2 = 0;

			//copy substring to varname array
			strcpy(varname,p);

			//set new position in the string we parsing
			p = p2 + 1;

			//see if we need the exe path (special case)
			if(strcmp("exepath",varname) == 0) {
				strcpy(varname,"exe");
			}

			strcpy(varname2,"path.");
			strcat(varname2,varname);

			//check if these are in configuration (instead of variables)
			if(strcmp("path.data",varname2) == 0)
				p2 = config->path.data;
			else if(strcmp("path.roms",varname2) == 0)
				p2 = config->path.roms;

			//get the var data from vars
			else
				p2 = vars_get_string(vars,varname2,"");

			//copy variable data to the dest string
			while(*p2) {
				dest[pos++] = *p2++;
			}
		}

		//copy the char
		dest[pos++] = *p;
	}

	printf("dest = '%s'\n",dest);
	for(p=dest;*p;p++) {
		if(*p == '/' || *p == '\\')
			*p = PATH_SEPERATOR;
	}
	mem_free(tmp);
	return(error ? 0 : dest);
}
