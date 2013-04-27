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

#define FRAMELIMIT

#ifdef WIN32
	#include <windows.h>
	#include "../win32/resource.h"
	#include <SDL/SDL_syswm.h>
#endif
#include <SDL/SDL.h>
#include "misc/log.h"
#include "palette/palette.h"
#include "nes/nes.h"
#include "system/video.h"
#include "system/win32/resource.h"
#include "system/sdl/console/console.h"
#include "misc/memutil.h"
#include "misc/config.h"

//video filters
#include "system/common/filters/draw/draw.h"
#include "system/common/filters/interpolate/interpolate.h"
#include "system/common/filters/scale2x/scalebit.h"
#include "system/common/filters/hq2x/hq2x.h"
//#include "system/common/filters/hq2x/hq3x.h"

static SDL_Surface *surface = 0;
static int flags = SDL_DOUBLEBUF | SDL_HWSURFACE;// | SDL_NOFRAME;
static int screenw,screenh,screenbpp;
static int screenscale;
static u32 palette32[256];
static u32 palettecache[32];
static u32 interval = 1000 / 60;
static u32 lasttime = 0;
static palette_t *palette = 0;
static u32 *screen = 0;
static void (*drawfunc)(void*,u32,void*,u32,u32,u32);		//dest,destpitch,src,srcpitch,width,height

enum filters_e {
	F_NONE,
	F_INTERPOLATE,
	F_SCALE,
	F_HQ,
};

static int get_filter_int(char *str)
{
	if(stricmp("interpolate",str) == 0)	return(F_INTERPOLATE);
	if(stricmp("scale",str) == 0)			return(F_SCALE);
	if(stricmp("hq",str) == 0)				return(F_HQ);
	return(F_NONE);
}

int video_reinit()
{
	int filter = get_filter_int(config_get_string("video.filter","none"));

	//set screen info
	flags &= ~SDL_FULLSCREEN;
	flags |= config_get_int("video.fullscreen",0) ? SDL_FULLSCREEN : 0;
	screenscale = config_get_int("video.scale",1);
	screenw = 256 * screenscale;
	screenh = 240 * screenscale;
	screenbpp = 32;

	//initialize surface/window
	surface = SDL_SetVideoMode(screenw,screenh,screenbpp,flags);
	SDL_WM_SetCaption("nesemu2",NULL);
	SDL_ShowCursor(0);

	//allocate memory for temp screen buffer
	screen = mem_realloc(screen,256 * (240 + 16) * (screenbpp / 8) * 4);
	drawfunc = draw1x;
	if(screenscale == 2) {
		if(filter == F_NONE)				drawfunc = draw2x;
		if(filter == F_INTERPOLATE)	drawfunc = interpolate2x;
		if(filter == F_SCALE)			drawfunc = scale2x;
		if(filter == F_HQ)				drawfunc = hq2x_32;
	}
	if(screenscale == 3) {
		if(filter == F_NONE)				drawfunc = draw3x;
		if(filter == F_INTERPOLATE)	drawfunc = interpolate3x;
		if(filter == F_SCALE)			drawfunc = scale3x;
	}
	if(screenscale == 4) {
		if(filter == F_NONE)				drawfunc = draw4x;
		if(filter == F_INTERPOLATE)	drawfunc = interpolate4x;
		if(filter == F_SCALE)			drawfunc = scale4x;
	}
	//print information
	log_printf("video initialized:  %dx%dx%d %s\n",screenw,screenh,screenbpp,(flags & SDL_FULLSCREEN) ? "fullscreen" : "windowed");

	return(0);
}

int video_init()
{
	int ret = video_reinit();

	ret += hq2x_InitLUTs();
	return(ret);
}

void video_kill()
{
	SDL_ShowCursor(1);
	if(screen)
		mem_free(screen);
	screen = 0;
	hq2x_Kill();
}

void video_startframe()
{
	//lock sdl surface
	SDL_LockSurface(surface);
}

void video_endframe()
{
	u32 t;
/*	u32 *ptr1 = screen;
	u32 *ptr2 = screen + 232 * 256;

	for(t=0;t<256*8;t++) {
		ptr1[t] = palettecache[0];
		ptr2[t] = palettecache[0];
	}*/

	drawfunc(surface->pixels,surface->pitch,screen,256*4,256,240);

	console_draw(surface->pixels,surface->pitch,screenh);

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
	u32 *dest = screen + (line * 256);
	int i;

	for(i=0;i<256;i++) {
		*dest++ = palettecache[*s++];
	}
}

//this handles palette changes from the nes engine
void video_updatepalette(u8 addr,u8 data)
{
	palettecache[addr] = palette32[data & 0x3F];
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

int video_getwidth()
{
	return(screenw);
}

int video_getheight()
{
	return(screenh);
}

int video_getbpp()
{
	return(screenbpp);
}

void *video_getscreen()
{
	return(screen);
}

