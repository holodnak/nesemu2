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
