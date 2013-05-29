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
#include "types.h"
#include "system/main.h"
#include "system/input.h"
#include "misc/paths.h"

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

//this function looks around for a configuration file.  it checks:
//  1. current working directory
//  2. $HOME directory
//  3. same directory the executable is in
//if it isnt found, it defaults the executable directory
int system_findconfig(char *dest)
{
	char *cwd = system_getcwd();
	char *home = getenv("HOME");

	//first look in the current working directory
	sprintf(dest,"%s%c%s",cwd,PATH_SEPERATOR,CONFIG_FILENAME);
	log_printf("looking for configuration at '%s'\n",dest);
	if(access(dest,06) == 0) {
		return(0);
	}

	//check the users home directory
	if(home) {
		sprintf(dest,"%s%c.nesemu%c%s",home,PATH_SEPERATOR,PATH_SEPERATOR,CONFIG_FILENAME);
		log_printf("looking for configuration at '%s'\n",dest);
		if(access(dest,06) == 0) {
			return(0);
		}
	}

#ifdef WIN32
	//win32 it is ok to store in the same directory as executable
	//now check the executable directory
	sprintf(dest,"%s%c%s",exepath,PATH_SEPERATOR,CONFIG_FILENAME);
	log_printf("looking for configuration at '%s'\n",dest);
	if(access(dest,06) == 0) {
		return(0);
	}

	//set default configuration filename
	sprintf(dest,"%s%c%s",exepath,PATH_SEPERATOR,CONFIG_FILENAME);

#else
	//linux it is not ok to store in the same directory as executable (/usr/bin or something)
	if(home) {
		sprintf(dest,"%s%c%s",home,PATH_SEPERATOR,CONFIG_FILENAME);
	}
	else {
		log_printf("system_findconfig:  HOME environment var not set!  using current directory.\n");
	}
#endif

	return(1);
}