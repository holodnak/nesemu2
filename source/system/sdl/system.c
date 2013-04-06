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
#include <direct.h>
#include "types.h"
#include "system/main.h"

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
//	SDL_QuitSubSystem(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
	SDL_Quit();
}

void system_check_events()
{
	SDL_Event event; /* Event structure */

	while(SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_QUIT:
				quit++;
				break;
/*			case SDL_JOYAXISMOTION:
				if (event.jaxis.axis == 0)
				{
					joystate[0] = 0;
					joystate[1] = 0;
					if (event.jaxis.value < -3200)
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
			case SDL_JOYBUTTONDOWN:
				joystate[event.jbutton.button + 4] = 1;
				break;
			case SDL_JOYBUTTONUP:
				joystate[event.jbutton.button + 4] = 0;
				break;*/
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
