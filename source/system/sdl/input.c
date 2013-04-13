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
#include "types.h"
#include "system/system.h"

/*
todo: rewrite input.  only need a few global input variables:

todo: new input system.  stores input data into the nes struct

required variables:
	mouse x,y
	mouse buttons
	keyboard state
	joystick state (merged into keyboard state, possibly)
*/

//these global variables provide information for the device input code
int joyx,joyy;		//x and y coords for paddle/mouse
u8 joyzap;			//zapper trigger
u8 joykeys[370];	//keyboard state

// this will map joystick axises/buttons to unused keyboard buttons
#define FIRSTJOYSTATEKEY (350) // ideally should be SDLK_LAST
u8 joystate[20];	// dpad + 8 buttons is enuff' for me but let's be sure :-)

int input_init()
{
	int i;

	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);
	for(i=0;i<20;i++) {
		joystate[i] = 0;
	}
	return(0);
}

void input_kill()
{
}

void input_poll()
{
	Uint8 *keystate = SDL_GetKeyState(NULL);
	int i,x,y;

//	joyx = joyy = 0;

	//need to update mousex/mousey/mousebuttons here

	//now update key/mouse state, the input device logic will
	//decode the key/mouse data into the correct input for the nes
	for(i=0;i<300;i++)
		joykeys[i] = keystate[i];
	joyzap = (SDL_GetMouseState(&x,&y) & 1) << 4;
	
	for (i=0; i < 20; i++)
	{
		joykeys[FIRSTJOYSTATEKEY + i] = joystate[i];
	}
	
}
