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
#include "log/log.h"
#include "palette/palette.h"
#include "system/video.h"

static SDL_Surface *surface = 0;
static int flags = SDL_DOUBLEBUF | SDL_HWSURFACE;// | SDL_NOFRAME;
static int screenw,screenh,screenbpp;
static int screenscale;
//static u32 palette32[256];
static u32 interval = 1000 / 60;
static u32 lasttime = 0;
static palette_t *palette = 0;
static u16 *screen;

int video_reinit()
{
	//set screen info
	flags &= ~SDL_FULLSCREEN;
	screenscale = 1;
	screenw = 256 * screenscale;
	screenh = 240 * screenscale;
	screenbpp = 32;

	//initialize surface/window
	surface = SDL_SetVideoMode(screenw,screenh,screenbpp,flags);
	SDL_WM_SetCaption("nesemu2","IDI_MAIN");
	SDL_ShowCursor(0);

	log_printf("video initialized:  %dx%dx%d %s\n",screenw,screenh,screenbpp,(flags & SDL_FULLSCREEN) ? "fullscreen" : "windowed");

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
	int pitch,x,y;
	u32 *dest;
	u16 *src;

	//lock sdl surface
	SDL_LockSurface(surface);

	//draw
	pitch = surface->pitch / 4;
	dest = (u32*)surface->pixels;
	src = screen;

	for(y=0;y<240;y++) {
		for(x=0;x<256;x++) {
			palentry_t *e = &palette->pal[src[x] >> 8][src[x] & 0x3F];
			dest[x] = (e->r << 16) | (e->g << 8) | (e->b << 0);
		}
		dest += pitch;
		src += 256;
	}


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

//s must be a 256x240 array of 16bit words
void video_setscreen(u16 *s)
{
	screen = s;
}
