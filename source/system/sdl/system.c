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
#ifdef WIN32
	#include <windows.h>
	#include <direct.h>
	#include <io.h>
#else
	#include <unistd.h>
#endif
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

static SDL_Joystick *joystick = 0;

int system_init()
{
	if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)) {
		printf("error at sdl init!\n");
		return(1);
	}
	if(SDL_NumJoysticks() > 0) {
		SDL_JoystickEventState(SDL_ENABLE);
		joystick = SDL_JoystickOpen(0);
	}
	return(0);
}

void system_kill()
{
	if(joystick)
		SDL_JoystickClose(joystick);
	SDL_Quit();
}

#define bit(n)	(1 << (n))
#define checkkey(key,n,evt)								\
	if(joykeys[key] && (keydown & bit(n)) == 0) {	\
		keydown |= bit(n);									\
		emu_event(evt,0);										\
	}																\
	else if(joykeys[key] == 0) {							\
		keydown &= ~bit(n);									\
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
			case SDL_JOYAXISMOTION:  /* Handle Joystick Motion */
				if (event.jaxis.axis == 0)
				{
					joystate[0] = 0;
					joystate[1] = 0;
					if(event.jaxis.value < -3200)
						joystate[0] = 1;
					else if (event.jaxis.value > 3200)
						joystate[1] = 1;
				}
				else if (event.jaxis.axis == 1)
				{
					joystate[2] = 0;
					joystate[3] = 0;
					if (event.jaxis.value < -3200)
						joystate[2] = 1;
					else if (event.jaxis.value > 3200)
						joystate[3] = 1;
				}
				break;
			case SDL_JOYBUTTONDOWN:  /* Handle Joystick Button Presses */
				joystate[event.jbutton.button + 4] = 1;
				break;
			case SDL_JOYBUTTONUP:  /* Handle Joystick Button Releases */
				joystate[event.jbutton.button + 4] = 0;
				break;
		}
	}

	//update the console
	console_update();

	//check for system key presses
	checkkey(SDLK_ESCAPE,	0,	E_QUIT);
	checkkey(SDLK_p,			1,	E_SOFTRESET);
	checkkey(SDLK_o,			2,	E_HARDRESET);
	checkkey(SDLK_F5,			3,	E_SAVESTATE);
	checkkey(SDLK_F8,			4,	E_LOADSTATE);
	checkkey(SDLK_F1,			5,	E_TOGGLERUNNING);
	checkkey(SDLK_F4,			6,	E_TOGGLEFULLSCREEN);
	checkkey(SDLK_F9,			7,	E_FLIPDISK);
	checkkey(SDLK_F10,		8,	E_DUMPDISK);
}

char *system_getcwd()
{
	static char buf[_MAX_PATH];

	if(getcwd(buf,_MAX_PATH) == NULL)
		memset(buf,0,_MAX_PATH);
	return(buf);
}

#ifdef WIN32
u64 system_gettick()
{
	LARGE_INTEGER li;

	QueryPerformanceCounter(&li);
	return(li.QuadPart);
}

u64 system_getfrequency()
{
	LARGE_INTEGER li;

	if(QueryPerformanceFrequency(&li) == 0)
		return(1);
	return(li.QuadPart);
}
#else //#elif defined(SDL)
u64 system_gettick()
{
	return(SDL_GetTicks());
}

u64 system_getfrequency()
{
	return(1000);
}
#endif
