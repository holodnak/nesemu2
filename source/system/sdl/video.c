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

#ifdef WIN32
#include <windows.h>
#endif
#include <SDL/SDL.h>
#include "palette/palette.h"

static SDL_Surface *surface = 0;
static int flags = SDL_DOUBLEBUF | SDL_HWSURFACE;// | SDL_NOFRAME;
static int screenw,screenh;
static int screenscale;
//static u32 palette32[256];
static u32 interval = 1000 / 60;
static u32 lasttime = 0;
static palette_t *palette;

int video_reinit()
{
	//set screen info
	flags &= ~SDL_FULLSCREEN;
	screenscale = 2;
	screenw = 256 * screenscale;
	screenh = 240 * screenscale;

	//initialize surface/window
	surface = SDL_SetVideoMode(screenw,screenh,32,flags);
	SDL_WM_SetCaption("nesemu2","IDI_MAIN");
	SDL_ShowCursor(0);
	return(0);
}

int video_init()
{
	int ret = video_reinit();

	return(ret);
}

void video_kill()
{
	SDL_ShowCursor(1);
//	if(screen)
//		free(screen);
//	screen = 0;
}

void video_endframe()
{
	u32 t;
	u32 *dest;
	int pitch;

	//lock sdl surface
	SDL_LockSurface(surface);

	//draw
	dest = (u32*)surface->pixels;
	pitch = surface->pitch / 4;

	//flip buffers and unlock surface
	SDL_Flip(surface);
	SDL_UnlockSurface(surface);

	//simple frame limiter
	t = SDL_GetTicks();
	while((t - lasttime) < interval) {
#ifdef WIN32
		Sleep(interval - (t - lasttime) + 0);
#endif
		t = SDL_GetTicks();
	}
	lasttime = t;
}

void video_setpalette(palette_t *p)
{
	palette = p;
}
