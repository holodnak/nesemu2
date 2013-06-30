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
#include "misc/log.h"
#include "palette/palette.h"
#include "nes/nes.h"
#include "system/system.h"
#include "system/video.h"
#include "system/win32/resource.h"
#include "system/sdl/console/console.h"
#include "misc/memutil.h"
#include "misc/config.h"
#include "system/common/filters.h"

static SDL_Surface *surface = 0;
static int flags = SDL_DOUBLEBUF | SDL_HWSURFACE;// | SDL_NOFRAME;
static int screenw,screenh,screenbpp;
static int screenscale;
static u32 palette32[8][256];
static u32 palettecache32[256];
static u8 palettecache[32];
static double interval = 0;
static u64 lasttime = 0;
static palette_t *palette = 0;
static u32 *screen = 0;
static u8 *nesscreen = 0;
static void (*drawfunc)(void*,u32,void*,u32,u32,u32);		//dest,destpitch,src,srcpitch,width,height
static filter_t *filter;

enum filters_e {
	F_NONE,
	F_INTERPOLATE,
	F_SCALE,
	F_NTSC,
};

static int get_filter_int(char *str)
{
	if(stricmp("interpolate",str) == 0)	return(F_INTERPOLATE);
	if(stricmp("scale",str) == 0)			return(F_SCALE);
	if(stricmp("ntsc",str) == 0)			return(F_NTSC);
	return(F_NONE);
}

static filter_t *get_filter(int flt)
{
	filter_t *ret = &filter_draw;

	switch(flt) {
		case F_INTERPOLATE:	ret = &filter_interpolate;		break;
		case F_SCALE:			ret = &filter_scale;				break;
//		case F_NTSC:			ret = &filter_ntsc;				break;
	}
	return(ret);
}

static int find_drawfunc()
{
	int i;

	for(i=0;filter->modes[i].scale;i++) {
		if(filter->modes[i].scale == screenscale) {
//			if(screenbpp == 32)
				drawfunc = filter->modes[i].draw32;
//			else
//				drawfunc = filter->modes[i].draw16;
			return(0);
		}
	}
	return(1);
}

int video_init()
{
	int i;

	if(nesscreen == 0)
		nesscreen = (u8*)mem_alloc(256 * (240 + 16));

	//setup timer to limit frames
	interval = (double)system_getfrequency() / 60.0f;
	lasttime = system_gettick();

	//clear palette cache
	memset(palettecache32,0,256*sizeof(u32));

	//set screen info
	flags &= ~SDL_FULLSCREEN;
	flags |= config_get_bool("video.fullscreen") ? SDL_FULLSCREEN : 0;
	screenscale = config_get_int("video.scale");
	screenbpp = 32;

	if(flags & SDL_FULLSCREEN)
		screenscale = (screenscale < 2) ? 2 : screenscale;

	i = get_filter_int(config_get_string("video.filter"));
	filter = get_filter((screenscale == 1) ? F_NONE : i);

	if(find_drawfunc() != 0) {
		log_printf("video_init:  error finding appropriate draw func, using draw1x\n");
		filter = &filter_draw;
		drawfunc = filter->modes[0].draw32;
	}

	screenw = filter->minwidth / filter->minscale * screenscale;
	screenh = filter->minheight / filter->minscale * screenscale;

	//initialize surface/window
	surface = SDL_SetVideoMode(screenw,screenh,screenbpp,flags);
	SDL_WM_SetCaption("nesemu2",NULL);
	SDL_ShowCursor(0);

	//allocate memory for temp screen buffer
	screen = (u32*)mem_realloc(screen,256 * (240 + 16) * (screenbpp / 8) * 4);

	//print information
	log_printf("video initialized:  %dx%dx%d %s\n",surface->w,surface->h,surface->format->BitsPerPixel,(flags & SDL_FULLSCREEN) ? "fullscreen" : "windowed");

	return(0);
}

void video_kill()
{
	SDL_ShowCursor(1);
	if(screen)
		mem_free(screen);
	if(nesscreen)
		mem_free(nesscreen);
	screen = 0;
	nesscreen = 0;
}

void video_startframe()
{
	//lock sdl surface
	SDL_LockSurface(surface);
}

void video_endframe()
{
	u64 t;

	//draw everything
	drawfunc(surface->pixels,surface->pitch,screen,256*4,256,240);
	console_draw((u32*)surface->pixels,surface->pitch,screenh);

	//flip buffers and unlock surface
	SDL_Flip(surface);
	SDL_UnlockSurface(surface);

	//simple frame limiter
	if(config_get_bool("video.framelimit")) {
		do {
			t = system_gettick();
		} while((double)(t - lasttime) < interval);
		lasttime = t;
	}
}

//this handles lines coming directly from the nes engine
void video_updateline(int line,u8 *s)
{
	u32 *dest = screen + (line * 256);
	int i;

	memcpy(nesscreen + (line * 256),s,256);
	if(line >= 8 && line < 232) {
		for(i=0;i<256;i++) {
			*dest++ = palettecache32[*s++];
		}
	}
	else {
		for(i=0;i<256;i++) {
			*dest++ = 0;
		}
	}
}

//this handles palette changes from the nes engine
void video_updatepalette(u8 addr,u8 data)
{
	palettecache32[addr+0x00] = palette32[0][data];
	palettecache32[addr+0x20] = palette32[1][data];
	palettecache32[addr+0x40] = palette32[2][data];
	palettecache32[addr+0x60] = palette32[3][data];
	palettecache32[addr+0x80] = palette32[4][data];
	palettecache32[addr+0xA0] = palette32[5][data];
	palettecache32[addr+0xC0] = palette32[6][data];
	palettecache32[addr+0xE0] = palette32[7][data];
	palettecache[addr] = data;
}

void video_setpalette(palette_t *p)
{
	int i,j;
	palentry_t *e;

	palette = p;
	for(j=0;j<8;j++) {
		for(i=0;i<256;i++) {
			e = &p->pal[j][i & 0x3F];
			palette32[j][i] = (e->r << 16) | (e->g << 8) | (e->b << 0);
		}
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

u8 *video_getscreen()
{
	return(nesscreen);
}

int video_zapperhit(int x,int y)
{
	int ret = 0;
	palentry_t *e;
	u8 color;

	color = palettecache[nesscreen[x + y * 256]];
	e = &palette->pal[(color >> 5) & 7][color & 0x1F];
	ret += (int)(e->r * 0.299);
	ret += (int)(e->g * 0.587);
	ret += (int)(e->b * 0.114);
	return((ret >= 0x40) ? 1 : 0);
}

//kludge-city!
int video_getxoffset()
{
	return(0);
}

int video_getyoffset()
{
	return(0);
}

int video_getscale()
{
	return(screenscale);
}
