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

#define DIRECTDRAW_VERSION 0x0700

#include <windows.h>
#include <windowsx.h>
#include <ddraw.h>
#include <stdio.h>
#include "misc/log.h"
#include "misc/config.h"
#include "misc/memutil.h"
#include "palette/palette.h"
#include "system/video.h"

static int screenw,screenh,screenbpp;
static int screenscale;
static u32 palette32[256];
static u32 palettecache[32];
static palette_t *palette = 0;
static u32 interval = 1000 / 60;
static u32 lasttime = 0;
static u32 *screen = 0;

static BITMAPINFO bmpinfo;
static RECT rect;
static HDC hDC;
static HBITMAP hBitmap;
static void *surface;

extern HWND hWnd;

int video_reinit()
{
	HWND hdesktopwnd = GetDesktopWindow();
	HDC hdesktopdc = GetDC(hdesktopwnd);
	int bpp;

	screenscale = config_get_int("video.scale",1);
	screenw = 256 * screenscale;
	screenh = 240 * screenscale;
	screenbpp = 32;
	screen = (u32*)mem_alloc(256 * (240 + 16) * sizeof(u32));

	bpp = GetDeviceCaps(hdesktopdc,BITSPIXEL);
	DeleteDC(hdesktopdc);
	memset(&bmpinfo,0,sizeof(bmpinfo));
	bmpinfo.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	bmpinfo.bmiHeader.biBitCount    = (u16)bpp;
	bmpinfo.bmiHeader.biWidth       = screenw;
	bmpinfo.bmiHeader.biHeight      = -(int)screenh;
	bmpinfo.bmiHeader.biPlanes      = 1;	
	bmpinfo.bmiHeader.biCompression = BI_RGB;
	bmpinfo.bmiHeader.biSizeImage   = 0;
	switch(bpp)
		{
		case 32:
			if((hBitmap = CreateDIBSection(0,&bmpinfo,DIB_RGB_COLORS,&surface,0,0)) == 0)
				{
				log_printf("video_init: CreateDIBSection error (bpp = %d)",bpp);
				return(1);
				}
			break;
		default:
			log_printf("video_init: bpp of %d isnt supported yet, try using 32bit color",bpp);
			return(1);
		}
	hDC = GetDC(hWnd);
	GetClientRect(hWnd,&rect);
	SendMessage(hWnd,WM_SIZE,0,((rect.right - rect.left) & 0xffff) | ((rect.bottom - rect.top) << 16));
	return(0);
}

int video_init()
{
	int ret = video_reinit();

	if(ret) {
		log_printf("video_init:  failed\n");
		video_kill();
	}
	return(ret);
}

void video_kill()
{
	if(screen) {
		mem_free(screen);
		screen = 0;
	}
	DeleteObject(surface);
	DeleteObject(hBitmap);
	ReleaseDC(hWnd,hDC);
}

void video_startframe()
{
}

void video_endframe()
{
	u32 *s = screen;
	u32 *d = (u32*)surface;
	int x,y;

	for(y=0;y<screenh;y++) {
		for(x=0;x<screenw;x++) {
			d[x] = s[x];
		}
		d += screenw;
		s += screenw;
	}
	StretchDIBits(hDC,rect.left,rect.top,rect.right,rect.bottom,0,0,screenw,screenh,surface,&bmpinfo,DIB_RGB_COLORS,SRCCOPY);
/*	u32 t;

	//simple frame limiter
	t = GetTickCount();
#ifdef FRAMELIMIT
	while((t - lasttime) < interval) {
		t = GetTickCount();
	}
#endif
	lasttime = t;*/
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

void video_resize()
{
	GetClientRect(hWnd,&rect);
}
