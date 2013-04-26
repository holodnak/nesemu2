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
#include "system/input.h"
#include "palette/palette.h"
#include "system/sdl/console/console.h"

#define CONSOLE_BUF_SIZE	8192

static char *buffer = 0;
static int showing;

int console_init()
{
	buffer = mem_alloc(CONSOLE_BUF_SIZE);
	showing = 0;
	return(0);
}

void console_kill()
{
	mem_free(buffer);
}

void console_draw(u32 *dest,int w,int h)
{

}

void console_print(char *str)
{

}

void console_show()
{
	showing++;
}

void console_hide()
{

}

//call 60 times a second
void console_update()
{
	static keydown = 0;

	if(joykeys[SDLK_BACKQUOTE] && (keydown & 1) == 0) {
		if(showing == 0)
			console_show();
		else
			console_hide();
		keydown |= 1;
	}
	else if(joykeys[SDLK_BACKQUOTE] == 0)
		keydown &= ~1;

}
