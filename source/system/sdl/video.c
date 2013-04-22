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

//#define DOUBLESIZE
//#define FRAMELIMIT

#ifdef WIN32
	#include <windows.h>
#endif
#include <SDL/SDL.h>
#include "log/log.h"
#include "palette/palette.h"
#include "system/video.h"
#include "nes/nes.h"
#include "system/win32/resource.h"

static SDL_Surface *surface = 0;
static int flags = SDL_DOUBLEBUF | SDL_HWSURFACE;// | SDL_NOFRAME;
static int screenw,screenh,screenbpp;
static int screenscale;
static u32 palette32[256];
static u32 palettecache[32];
static u32 interval = 1000 / 60;
static u32 lasttime = 0;
static palette_t *palette = 0;

int video_reinit()
{
	//set screen info
	flags &= ~SDL_FULLSCREEN;
#ifdef DOUBLESIZE
	screenscale = 2;
#else
	screenscale = 1;
#endif
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

void video_startframe()
{
	//lock sdl surface
	SDL_LockSurface(surface);
}

void video_endframe()
{
	u32 t;

	//flip buffers and unlock surface
	SDL_Flip(surface);
	SDL_UnlockSurface(surface);

	//simple frame limiter
	t = SDL_GetTicks();
#ifdef FRAMELIMIT
	while((t - lasttime) < interval) {
#ifdef WIN32
//		Sleep(interval - (t - lasttime) + 0);
#endif
		t = SDL_GetTicks();
	}
#endif
	lasttime = t;
}

//this handles lines coming directly from the nes engine
void video_updateline(int line,u8 *s)
{
	int i;
#ifdef DOUBLESIZE
	u32 *dest1 = (u32*)((u8*)surface->pixels + (((line * 2) + 0) * surface->pitch));
	u32 *dest2 = (u32*)((u8*)surface->pixels + (((line * 2) + 1) * surface->pitch));

	for(i=0;i<256;i++) {
		u32 pixel = palettecache[*s++];
		*dest1++ = pixel;
		*dest1++ = pixel;
		*dest2++ = pixel;
		*dest2++ = pixel;
	}
#else
	u32 *dest = (u32*)((u8*)surface->pixels + (line * surface->pitch));

	for(i=0;i<256;i++) {
		*dest++ = palettecache[*s++];
	}
#endif
}


//this handles palette changes from the nes engine
void video_updatepalette(u8 addr,u8 data)
{
	palettecache[addr & 0x1F] = palette32[data & 0x3F];
}

void video_setpalette(palette_t *p)
{
	int i;
	palentry_t *e;

	palette = p;
	for(i=0;i<256;i++) {
		e = &p->pal[0][i & 0x3F];
		palette32[i] = (e->r << 16) | (e->g << 8) | (e->b << 0);
	}
}

