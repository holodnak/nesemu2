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
#include "misc/log.h"
#include "emu/emu.h"
#include "system/main.h"

char *paths_parse(char *src,char *dest,int len)
{
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

			//clear area to hold var name
			memset(varname,0,64);

			//see if it is missing the '%'
			if((p2 = strchr(p + 1,'%')) == 0) {
				log_printf("paths_parse:  missing ending '%', just copying\n");
			}

			//not missing, replace with variable data
			else {
				//skip over the '%'
				p++;

				//terminate the substring
				*p2 = 0;

				//copy substring to varname array
				strcpy(varname,p);

				//set new position in the string we parsing
				p = p2 + 1;

				//see if we need the exe path (special case)
				if(strcmp("exepath",varname) == 0) {
					p2 = exepath;
				}

				//user home directory
				else if(strcmp("home",varname) == 0) {
					p2 = exepath;
				}

				//see if it comes from the configuration
				else if(strncmp("config.",varname,7) == 0) {
					p2 = config_get_string((char*)varname + 7,"");
				}

				//else we are using a variable from our var list
				else {
					p2 = vars_get_string(vars,varname2,"");
				}

				//see if it has a % in it
				if(strrchr(p2,'%') != 0) {
					char *tmp2 = (char*)mem_alloc(1024);

					p2 = paths_parse(p2,tmp2,1024);
					while(p2 && *p2) {
						dest[pos++] = *p2++;
					}
					mem_free(tmp2);
				}

				else {
					//copy variable data to the dest string
					while(p2 && *p2) {
						dest[pos++] = *p2++;
					}
				}
			}
		}

		//copy the char
		dest[pos++] = *p;
	}

	//normalize the path
	for(p=dest;*p;p++) {
		if(*p == '/' || *p == '\\')
			*p = PATH_SEPERATOR;
	}

	//free tmp string and return
	mem_free(tmp);
	return(dest);
}

void paths_makestatefilename(char *romfilename,char *dest,int len)
{
	char *p,*tmp = mem_strdup(romfilename);

	//clear the string
	memset(dest,0,len);

	//parse the state path
	paths_parse(config->path.state,dest,len);

	//append the path seperator
	dest[strlen(dest)] = PATH_SEPERATOR;

	//normalize the path seperators
	for(p=tmp;*p;p++) {
		if(*p == '/' || *p == '\\')
			*p = PATH_SEPERATOR;
	}

	//find the last path seperator
	p = strrchr(tmp,PATH_SEPERATOR);

	//if not found then it is plain filename
	p = (p == 0) ? tmp : p + 1;

/*	//this code removed because if you have smb.fds and smb.nes they use the same state filename
	//find the extension
	p2 = strrchr(tmp,'.');

	//if found, end the string here
	if(p2) {
		*p2 = 0;
	}*/

	//append the rom filename
	strcat(dest,p);

	//append the state extension)
	strcat(dest,".state");

	//free the temporary string
	mem_free(tmp);
}
