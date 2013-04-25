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

#include <SDL/SDL.h>
#include "misc/memutil.h"
#include "palette/palette.h"
#include "console.h"

#define CONSOLE_BUF_SIZE	8192

static char *buffer = 0;

int console_init()
{
	buffer = mem_alloc(CONSOLE_BUF_SIZE);
	return(0);
}

void console_kill()
{
	mem_free(buffer);
}

void console_draw(u8 *dest,int w,int h)
{

}

void console_print(char *str)
{

}

//call 60 times a second
void console_update()
{

}
