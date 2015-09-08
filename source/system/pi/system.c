/***************************************************************************
 *   Copyright (C) 2006-2009 by Dead_Body   *
 *   jamesholodnak@gmail.com   *
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
#include <unistd.h>
#include "emu/events.h"
#include "system/main.h"
#include "system/video.h"
#include "system/input.h"
#include "system/sound.h"
#include "system/sdl/console/console.h"
#include "nes/nes.h"
#include "nes/state/state.h"
#include "misc/paths.h"
#include "misc/log.h"
#include "misc/config.h"

#ifndef _MAX_PATH
	#define _MAX_PATH 4096
#endif

int system_init()
{
	if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
		printf("error at sdl init!\n");
		return(1);
	}
	return(0);
}

void system_kill()
{
	SDL_Quit();
}

void system_checkevents()
{
	static int keydown = 0;
	SDL_Event event; /* Event structure */

	//check out sdl events
	while(SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_QUIT:
				quit++;
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				console_keyevent(event.key.type,event.key.keysym.sym);
				break;
		}
	}
}

char *system_getcwd()
{
	static char buf[_MAX_PATH];

	if(getcwd(buf,_MAX_PATH) == NULL)
		memset(buf,0,_MAX_PATH);
	return(buf);
}

u64 system_gettick()
{
	return(SDL_GetTicks());
}

u64 system_getfrequency()
{
	return(1000);
}
