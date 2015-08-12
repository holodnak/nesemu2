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
#include <ctype.h>
#include "misc/strutil.h"

//check if a char is whitespace
static int iswhitespace(char ch)
{
	if(ch == ' ' || ch == '\t' || ch == '\n')
		return(1);
	return(0);
}

//eat whitespace from beginning and end of the string
char *str_eatwhitespace(char *str)
{
	char *p,*ret = str;

	while(iswhitespace(*ret))
		ret++;
	p = ret + strlen(ret) - 1;
	while(iswhitespace(*p))
		*p-- = 0;
	return(ret);
}

//append a single char to the end of a string
void str_appendchar(char *str,char ch)
{
	int n = strlen(str);

	str[n] = ch;
	str[n+1] = 0;
}

//converts a numerical string to unsigned number
u32 str_tou32(char *str)
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