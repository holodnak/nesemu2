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

//system related variables
static SDL_Surface *surface = 0;
static int flags = SDL_DOUBLEBUF | SDL_HWSURFACE;
static int screenw,screenh,screenbpp;
static int screenscale;

//palette with emphasis applied
static u8 palette[8][64 * 3];

//palette data fed to video system
static u16 palette16[8][256];		//16 bit color
static u32 palette32[8][256];		//32 bit color

//caches of all available colors
static u16 palettecache16[256];
static u32 palettecache32[256];

//actual values written to nes palette ram
static u8 palettecache[32];

//for frame limiting
static double interval = 0;
static u64 lasttime = 0;

//pointer to scree and copy of the nes screen
static u16 *screen16 = 0;
static u32 *screen32 = 0;
static u8 *nesscreen = 0;

//draw function pointer and pointer to current video filter
static void (*drawfunc)(void*,u32,void*,u32,u32,u32);		//dest,destpitch,src,srcpitch,width,height
static filter_t *filter;

//for correct colors
static int rshift,gshift,bshift;
static int rloss,gloss,bloss;

static int find_drawfunc(int scale,int bpp)
{
	int i;

	for(i=0;filter->modes[i].scale;i++) {
		if(filter->modes[i].scale == scale) {
			switch(bpp) {
				case 32:
					drawfunc = filter->modes[i].draw32;
					return(0);
				case 16:
				case 15:
					drawfunc = filter->modes[i].draw16;
					return(0);
				default:
					log_printf("find_drawfunc:  unsupported bit depth (%d)\n",bpp);
					return(2);
			}
		}
	}
	return(1);
}

static void get_surface_info(SDL_Surface *s)
{
	SDL_PixelFormat *pf = s->format;

	log_printf("get_surface_info:  sdl surface info:\n");
	log_printf("  bits per pixel:  %d\n",pf->BitsPerPixel);
	log_printf("    red:    mask:  %08X    shift:  %d    loss: %d\n",pf->Rmask,pf->Rshift,pf->Rloss);
	log_printf("    green:  mask:  %08X    shift:  %d    loss: %d\n",pf->Gmask,pf->Gshift,pf->Gloss);
	log_printf("    blue:   mask:  %08X    shift:  %d    loss: %d\n",pf->Bmask,pf->Bshift,pf->Bloss);

	rshift = pf->Rshift;
	gshift = pf->Gshift;
	bshift = pf->Bshift;
	rloss = pf->Rloss;
	gloss = pf->Gloss;
	bloss = pf->Bloss;
}

//return absolute value
static int absolute_value(int v)
{
	return((v < 0) ? (0 - v) : v);
}

int find_video_mode(int wantw,int wanth,int flags,int *w,int *h)
{
	SDL_Rect **modes,*mode;
	int i,diffw[2],diffh[2];

	//get list of modes from sdl
	modes = SDL_ListModes(NULL,flags);
	*w = *h = 0;

	//if nothing returned
	if(modes == (SDL_Rect**)0) {
		log_printf("find_video_mode:  fatal error:  no modes available\n");
		return(1);
	}

	//see if any mode is available (windowed mode)
	if(modes == (SDL_Rect**)-1) {
		log_printf("find_video_mode:  all resolutions available\n");
		*w = wantw;
		*h = wanth;
		return(0);
	}

	//output modes
	log_printf("find_video_mode:  available modes:\n");
	for(i=0;modes[i];i++) {
		log_printf("find_video_mode:    %d x %d\n",modes[i]->w,modes[i]->h);
	}

	//search for closest video mode
	for(mode=0,i=0;modes[i];i++) {
		if(modes[i]->w >= wantw && modes[i]->h >= wanth) {
			if(mode == 0) {
				mode = modes[i];
			}
			else {
				diffw[0] = absolute_value(mode->w - wantw);
				diffh[0] = absolute_value(mode->h - wanth);
				diffw[1] = absolute_value(modes[i]->w - wantw);
				diffh[1] = absolute_value(modes[i]->h - wanth);
				if((diffw[1] + diffh[1]) < (diffw[0] + diffh[0])) {
					mode = modes[i];
				}
			}
		}
	}

	//if a mode was found set the return variables
	if(mode) {
		*w = mode->w;
		*h = mode->h;
	}

	return(0);
}

static int get_desktop_bpp()
{
	const SDL_VideoInfo *vi = SDL_GetVideoInfo();

	log_printf("get_desktop_bpp:  current display mode is %d x %d, %d bpp\n",vi->current_w,vi->current_h,vi->vfmt->BitsPerPixel);
	return(vi->vfmt->BitsPerPixel);
}

int video_init()
{
	if(nesscreen == 0)
		nesscreen = (u8*)mem_alloc(256 * (240 + 16));

	//setup timer to limit frames
	interval = (double)system_getfrequency() / 60.0f;
	lasttime = system_gettick();

	//clear palette caches
	memset(palettecache16,0,256*sizeof(u16));
	memset(palettecache32,0,256*sizeof(u32));

	//set screen info
	flags &= ~SDL_FULLSCREEN;
	flags |= config_get_bool("video.fullscreen") ? SDL_FULLSCREEN : 0;
	screenscale = config_get_int("video.scale");

	//fullscreen mode
	if(flags & SDL_FULLSCREEN) {
		screenscale = (screenscale < 2) ? 2 : screenscale;
		screenbpp = 16;
	}

	//windowed mode
	else {
		screenbpp = get_desktop_bpp();
	}

	//initialize the video filters
	filter_init();

	//get pointer to video filter
	filter = filter_get((screenscale == 1) ? F_NONE : filter_get_int(config_get_string("video.filter")));

	if(find_drawfunc(screenscale,screenbpp) != 0) {
		log_printf("video_init:  error finding appropriate draw func, using draw1x\n");
		filter = &filter_draw;
		drawfunc = filter->modes[0].draw32;
	}

	//calculate desired screen dimensions
	screenw = filter->minwidth / filter->minscale * screenscale;
	screenh = filter->minheight / filter->minscale * screenscale;

	//fullscreen mode
	if(flags & SDL_FULLSCREEN) {
		int w,h;

		if(find_video_mode(screenw,screenh,flags | SDL_FULLSCREEN,&w,&h) == 0) {
			screenw = w;
			screenh = h;
			log_printf("video_init:  best display mode:  %d x %d\n",w,h);
		}
	}

	//initialize surface/window
	surface = SDL_SetVideoMode(screenw,screenh,screenbpp,flags);
	SDL_WM_SetCaption("nesemu2",NULL);
	SDL_ShowCursor(0);
	get_surface_info(surface);

	//allocate memory for temp screen buffer
	screen16 = (u32*)mem_realloc(screen16,256 * (240 + 16) * (screenbpp / 8) * 2);
	screen32 = (u32*)mem_realloc(screen32,256 * (240 + 16) * (screenbpp / 8) * 4);

	//print information
	log_printf("video initialized:  %dx%dx%d %s\n",surface->w,surface->h,surface->format->BitsPerPixel,(flags & SDL_FULLSCREEN) ? "fullscreen" : "windowed");

	return(0);
}

void video_kill()
{
	filter_kill();
	SDL_ShowCursor(1);
	if(screen16)
		mem_free(screen16);
	if(screen32)
		mem_free(screen32);
	if(nesscreen)
		mem_free(nesscreen);
	screen = 0;
	nesscreen = 0;
}

int video_reinit()
{
	video_kill();
	return(video_init());
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
	drawfunc(surface->pixels,surface->pitch,screen16,256*2,256,240);

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

//this handles pixels coming directly from the nes engine
void video_updatepixel(int line,int pixel,u8 s)
{
	int offset = (line * 256) + pixel;

	nesscreen[offset] = s;
	if(line >= 8 && line < 232) {
		screen16[offset] = palettecache16[s];
	}
	else {
		screen16[offset] = 0;
	}
}

//this handles palette changes from the nes engine
void video_updatepalette(u8 addr,u8 data)
{
	palettecache16[addr+0x00] = palette16[0][data];
	palettecache16[addr+0x20] = palette16[1][data];
	palettecache16[addr+0x40] = palette16[2][data];
	palettecache16[addr+0x60] = palette16[3][data];
	palettecache16[addr+0x80] = palette16[4][data];
	palettecache16[addr+0xA0] = palette16[5][data];
	palettecache16[addr+0xC0] = palette16[6][data];
	palettecache16[addr+0xE0] = palette16[7][data];
	palettecache[addr] = data;
}

//must be called AFTER video_init
void video_setpalette(palette_t *p)
{
	int i,j;
	palentry_t *e;

	for(j=0;j<8;j++) {
		for(i=0;i<64;i++) {
			palette[j][(i * 3) + 0] = p->pal[j][i].r;
			palette[j][(i * 3) + 1] = p->pal[j][i].g;
			palette[j][(i * 3) + 2] = p->pal[j][i].b;
		}
	}

	for(j=0;j<8;j++) {
		for(i=0;i<256;i++) {
			e = &p->pal[j][i & 0x3F];
			palette16[j][i] = ((e->r >> rloss) << rshift) | ((e->g >> gloss) << gshift) | ((e->b >> bloss) << bshift);
			palette32[j][i] = (e->r << rshift) | (e->g << gshift) | (e->b << bshift);
		}
	}

	filter_palette_changed();
}

int video_getwidth()			{	return(screenw);			}
int video_getheight()			{	return(screenh);			}
int video_getbpp()				{	return(screenbpp);		}
u8 *video_getscreen()			{	return(nesscreen);		}
u8 *video_getpalette()			{	return((u8*)palette);			}

int video_zapperhit(int x,int y)
{
	int ret = 0;
	u8 *e;
	u8 color;

	color = palettecache[nesscreen[x + y * 256]];
	e = &palette[(color >> 5) & 7][(color & 0x3F) * 3];
	ret += (int)(e[0] * 0.299f);
	ret += (int)(e[1] * 0.587f);
	ret += (int)(e[2] * 0.114f);
	return((ret >= 0x40) ? 1 : 0);
}

//kludge-city!
int video_getxoffset()	{	return(0);	}
int video_getyoffset()	{	return(0);	}
int video_getscale()	{	return(screenscale);	}
