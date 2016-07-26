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

extern "C" {
	#include "misc/log.h"
	#include "misc/config.h"
	#include "misc/memutil.h"
	#include "palette/palette.h"
	#include "system/system.h"
	#include "system/video.h"
	#include "system/win32/mainwnd.h"
	#include "system/common/filters.h"
}

#undef _MSC_VER

#if (_MSC_VER >= 1400)
typedef HRESULT (WINAPI *LPDIRECTDRAWCREATEEX)(GUID FAR *, LPVOID *, REFIID,IUnknown FAR *);
HINSTANCE ddrawdll;
LPDIRECTDRAWCREATEEX DirectDrawCreateEx;
#endif

static u8 *nesscreen = 0;
static u16 *screen16;
static u32 *screen32;
static void *screen;
static int screenw,screenh,screenbpp;
static int screenscale;
static u8 palette[8][64 * 3];
static u16 palette15[8][256];
static u16 palette16[8][256];
static u32 palette32[8][256];
static u8 palettecache[32];
static u16 palettecache16[256];
static u32 palettecache32[256];
static double interval = 0;
static u64 lasttime = 0;

static int pitch;
static int surfoffset;

static HINSTANCE ddrawdll;
static LPDIRECTDRAW7 lpDD;
static LPDIRECTDRAWSURFACE7 lpPrimaryDDS,lpSecondaryDDS;
static LPDIRECTDRAWCLIPPER lpDirectDrawClipper;
static DDSURFACEDESC2 ddsd;
static HMENU hMenu = 0;
//static RECT rect;

static void (*updatepalette)(u32,u8) = 0;

//draw function pointer and pointer to current video filter
static void (*drawfunc)(void*,u32,void*,u32,u32,u32);		//dest,destpitch,src,srcpitch,width,height
static filter_t *filter;

static void updatepalette15(u32 addr,u8 data)
{
	palettecache16[addr+0x00] = palette15[0][data];
	palettecache16[addr+0x20] = palette15[1][data];
	palettecache16[addr+0x40] = palette15[2][data];
	palettecache16[addr+0x60] = palette15[3][data];
	palettecache16[addr+0x80] = palette15[4][data];
	palettecache16[addr+0xA0] = palette15[5][data];
	palettecache16[addr+0xC0] = palette15[6][data];
	palettecache16[addr+0xE0] = palette15[7][data];
}

static void updatepalette16(u32 addr,u8 data)
{
	palettecache16[addr+0x00] = palette16[0][data];
	palettecache16[addr+0x20] = palette16[1][data];
	palettecache16[addr+0x40] = palette16[2][data];
	palettecache16[addr+0x60] = palette16[3][data];
	palettecache16[addr+0x80] = palette16[4][data];
	palettecache16[addr+0xA0] = palette16[5][data];
	palettecache16[addr+0xC0] = palette16[6][data];
	palettecache16[addr+0xE0] = palette16[7][data];
}

static void updatepalette32(u32 addr,u8 data)
{
	palettecache32[addr+0x00] = palette32[0][data];
	palettecache32[addr+0x20] = palette32[1][data];
	palettecache32[addr+0x40] = palette32[2][data];
	palettecache32[addr+0x60] = palette32[3][data];
	palettecache32[addr+0x80] = palette32[4][data];
	palettecache32[addr+0xA0] = palette32[5][data];
	palettecache32[addr+0xC0] = palette32[6][data];
	palettecache32[addr+0xE0] = palette32[7][data];
}

static void resizeclient(HWND hwnd,int w,int h)
{
	RECT rc,rw;

	GetWindowRect(hwnd,&rw);
	GetClientRect(hwnd,&rc);
	SetWindowPos(hwnd,0,0,0,((rw.right - rw.left) - rc.right) + w,((rw.bottom - rw.top) - rc.bottom) + h,SWP_NOZORDER | SWP_NOMOVE);
}

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
				//	drawfunc = filter->modes[i].draw16;
				//	return(0);
				default:
					log_printf("find_drawfunc:  unsupported bit depth (%d)\n",bpp);
					return(2);
			}
		}
	}
	return(1);
}

#define SAFE_RELEASE(vv)	\
	if(vv) {						\
		vv->Release();			\
		vv = NULL;				\
	}

static void killddraw()
{
	SAFE_RELEASE(lpDD);
#if(_MSC_VER >= 1400)
	DirectDrawCreateEx = NULL;
	if(ddrawdll) {
		FreeLibrary(ddrawdll);
		ddrawdll = NULL;
	}
#endif
}

#define _T(s) s

static int initddraw()
{
	if(ddrawdll)
		return(0);
#if(_MSC_VER >= 1400)
	if((ddrawdll = LoadLibrary("ddraw.dll")) == 0) {
		MessageBox(hWnd,"Error loading DirectDraw DLL","nesemu2",MB_OK | MB_ICONERROR);
		return(1);
	}
	if((DirectDrawCreateEx = (LPDIRECTDRAWCREATEEX)GetProcAddress(ddrawdll,"DirectDrawCreateEx")) == 0) {
		MessageBox(hWnd,"Error in GetProcAddress of function DirectDrawCreateEx","nesemu2",MB_OK | MB_ICONERROR);
		return(1);
	}
#endif
	if(FAILED(DirectDrawCreateEx(NULL, (LPVOID *)&lpDD, IID_IDirectDraw7, NULL))) {
		MessageBox(hWnd,"Error initializing DirectDraw","nesemu2",MB_OK | MB_ICONERROR);
		return(1);
	}
	return(0);
}

static int initwindowed()
{
	resizeclient(hWnd,screenw,screenh);
	if(FAILED(lpDD->SetCooperativeLevel(hWnd, DDSCL_NORMAL))) {
		MessageBox(hWnd,"Error setting DirectDraw cooperative level","nesemu2",MB_OK | MB_ICONERROR);
		return(1);
	}
	ZeroMemory(&ddsd,sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	if(FAILED(lpDD->CreateSurface(&ddsd, &lpPrimaryDDS, NULL))) {
		MessageBox(hWnd,"Error creating primary surface","nesemu2",MB_OK | MB_ICONERROR);
		return(1);
	}
	ddsd.dwWidth = screenw;
	ddsd.dwHeight = screenh;
	ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	if(FAILED(lpDD->CreateSurface(&ddsd,&lpSecondaryDDS,NULL))) {
		MessageBox(hWnd,"Error creating secondary surface","nesemu2",MB_OK | MB_ICONERROR);
		return(1);
	}
	if(FAILED(lpDD->CreateClipper(0,&lpDirectDrawClipper,NULL))) {
		MessageBox(hWnd,"Error creating clipper","nesemu2",MB_OK | MB_ICONERROR);
		return(1);
	}
	if(FAILED(lpDirectDrawClipper->SetHWnd(0,hWnd))) {
		MessageBox(hWnd,"Error setting clipper window handle","nesemu2",MB_OK | MB_ICONERROR);
		return(1);
	}
	if(FAILED(lpPrimaryDDS->SetClipper(lpDirectDrawClipper))) {
		MessageBox(hWnd,"Error setting primary surface clipper","nesemu2",MB_OK | MB_ICONERROR);
		return(1);
	}
	surfoffset = 0;
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	return(0);
}

static int initfullscreen()
{
	if(GetWindowLong(hConsole,GWL_USERDATA) != 0)
		ShowWindow(hConsole,SW_MINIMIZE);
	if(hDebugger)
		ShowWindow(hDebugger,SW_MINIMIZE);
	SetWindowLongPtr(hWnd,GWL_STYLE,WS_POPUP);
	SetMenu(hWnd,NULL);
//	GetWindowRect(hWnd,&rect);
	if(FAILED(lpDD->SetCooperativeLevel(hWnd,DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_NOWINDOWCHANGES))) {
		MessageBox(hWnd,"Error setting DirectDraw cooperative level","nesemu2",MB_OK | MB_ICONERROR);
		return(1);
	}

	//need to enumerate display modes and set the correct one for the monitor, until then...
	//kludge!
	screenscale = 2;
	screenw = 640;
	screenh = 480;
	screenbpp = 32;

	if(FAILED(lpDD->SetDisplayMode(screenw,screenh,screenbpp,0,0))) {
		MessageBox(hWnd,"Error setting display mode, trying windowed.","nesemu2",MB_OK | MB_ICONERROR);
		video_kill();

		screenscale = config_get_int("video.scale");
		screenw = 256 * screenscale;
		screenh = 240 * screenscale;
		config_set_bool("video.fullscreen",0);

		return(initwindowed());
	}

	surfoffset = ((screenw - 512) * (screenbpp / 8)) / 2;
	ShowWindow(hWnd,SW_MAXIMIZE);
	ZeroMemory(&ddsd,sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
	ddsd.dwBackBufferCount = 1;
	if(FAILED(lpDD->CreateSurface(&ddsd,&lpPrimaryDDS,NULL))) {
		MessageBox(hWnd,"Error creating primary surface","nesemu2",MB_OK | MB_ICONERROR);
		return(1);
	}
	ddsd.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;
	if(FAILED(lpPrimaryDDS->GetAttachedSurface(&ddsd.ddsCaps,&lpSecondaryDDS))) {
		MessageBox(hWnd,"Error creating secondary surface","nesemu2",MB_OK | MB_ICONERROR);
		return(1);
	}
	ZeroMemory(&ddsd,sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ShowCursor(FALSE);
	return(0);
}

static int initvideo()
{
	RECT rect;
	POINT pt = {0, 0};
	int i,ret;

	//nesscreen is nes screen data (raw palette indexes)
	if(nesscreen == 0)
		nesscreen = (u8*)mem_alloc(256 * (240 + 16));

	//copy of unfiltered screen data (ready for output)
	if(screen == 0)
		screen = (u8*)mem_alloc(256 * (240 + 16) * 4);
	screen16 = (u16*)screen;
	screen32 = (u32*)screen;

	//initialize the video filters
	filter_init();

	//get pointer to video filter
	filter = filter_get((screenscale == 1) ? F_NONE : filter_get_int(config_get_string("video.filter")));

	//calculate desired screen dimensions
	screenscale = config_get_int("video.scale");
	screenw = filter->minwidth / filter->minscale * screenscale;
	screenh = filter->minheight / filter->minscale * screenscale;
	screenbpp = 32;

	//initialize video
	if(config_get_bool("video.fullscreen")) {
		ret = initfullscreen();
	}
	else {
		ret = initwindowed();
		resizeclient(hWnd,screenw,screenh);
	}

	//get draw function
	if(find_drawfunc(screenscale,screenbpp) != 0) {
		log_printf("video_init:  error finding appropriate draw func, using draw1x\n");
		filter = &filter_draw;
		drawfunc = filter->modes[0].draw32;
	}

	//if video initialized ok, finalize the initialization
	if(ret == 0) {

		//get surface information
		if(FAILED(lpSecondaryDDS->GetSurfaceDesc(&ddsd))) {
			MessageBox(hWnd,"Failed to retrieve surface description","nesemu2",MB_OK | MB_ICONERROR);
			return(1);
		}

		//clear the screen
		lpSecondaryDDS->Lock(NULL,&ddsd,DDLOCK_WAIT | DDLOCK_NOSYSLOCK | DDLOCK_WRITEONLY,NULL);
		memset(ddsd.lpSurface,0,ddsd.lPitch * ddsd.dwHeight);
		lpSecondaryDDS->Unlock(NULL);
		GetClientRect(hWnd,&rect);
//		if((rect.right == 0) || (rect.bottom == 0))
//			return;
		ClientToScreen(hWnd,&pt);
		rect.left += pt.x;
		rect.right += pt.x;
		rect.top += pt.y;
		rect.bottom += pt.y;
		lpPrimaryDDS->Blt(&rect,lpSecondaryDDS,NULL,DDBLT_WAIT,NULL);

		//line width
		pitch = ddsd.lPitch;

		//check if bit depth is supported
		switch(ddsd.ddpfPixelFormat.dwRGBBitCount) {
			case 16:
				if(ddsd.ddpfPixelFormat.dwRBitMask == 0xF800) {
					screenbpp = 16;
					updatepalette = updatepalette16;
				}
				else {
					screenbpp = 15;
					updatepalette = updatepalette15;
				}
				break;
			case 32:
				screenbpp = 32;
				updatepalette = updatepalette32;
				break;
			default:
				MessageBox(hWnd,"Error detecting bits per pixel","nesemu2",MB_OK | MB_ICONERROR);
				return(1);
		}

		//update palette
		for(i=0;i<32;i++)
			updatepalette(i,palettecache[i]);

		log_printf("video_reinit:  ddraw video inited, %ix%i %ibpp (scale %d, surfoffset = %d)\n",ddsd.dwWidth,ddsd.dwHeight,screenbpp,screenscale,surfoffset);
	}
	return(ret);
}

int video_init()
{
	int ret;

	//setup timer to limit frames
	interval = (double)system_getfrequency() / 60.0f;
	lasttime = system_gettick();

	hMenu = GetMenu(hWnd);
	if(initddraw() != 0) {
		killddraw();
		return(1);
	}
	if((ret = initvideo())) {
		log_printf("video_init:  failed\n");
		video_kill();
	}
	return(ret);
}

void video_kill()
{
	if(nesscreen) {
		mem_free(nesscreen);
		nesscreen = 0;
	}
	if(screen) {
		mem_free(screen);
		screen = 0;
		screen16 = 0;
		screen32 = 0;
	}
	SAFE_RELEASE(lpDirectDrawClipper);
	SAFE_RELEASE(lpSecondaryDDS);
	SAFE_RELEASE(lpPrimaryDDS);
	if(config_get_bool("video.fullscreen")) {
		ShowCursor(TRUE);
//		SetWindowPos(hWnd,0,rect.left,rect.top,0,0,SWP_NOCOPYBITS | SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER);
		SetWindowPos(hWnd,0,0,0,0,0,SWP_NOCOPYBITS | SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER);
		SetWindowLongPtr(hWnd,GWL_STYLE,WS_OVERLAPPEDWINDOW);
		SetMenu(hWnd,hMenu);
		ShowWindow(hWnd,SW_RESTORE);
		if(GetWindowLong(hConsole,GWL_USERDATA) != 0) {
			ShowWindow(hConsole,SW_RESTORE);
		}
		if(hDebugger)
			ShowWindow(hDebugger,SW_RESTORE);
	}
	killddraw();
}

int video_reinit()
{
	video_kill();
	return(video_init());
}

void video_startframe()
{
	lpSecondaryDDS->Lock(NULL,&ddsd,DDLOCK_WAIT | DDLOCK_NOSYSLOCK | DDLOCK_WRITEONLY,NULL);
}

void video_endframe()
{
	RECT rect;
	POINT pt = {0, 0};
	u64 t;

	//blit screen to surface
	drawfunc(ddsd.lpSurface,ddsd.lPitch,screen,256*4,256,240);

/*	switch(screenbpp) {
		case 8:
		case 24:
		default:
			log_printf("video_endframe:  blitting unsupported bit depth: %d\n",screenbpp);
			break;
		case 15:
		case 16:
			linesize = pitch / 2;
			s16 = screen16;
			d16 = (u16*)((u8*)ddsd.lpSurface + surfoffset);
			for(y=0;y<240;y++) {
				for(x=0;x<256;x++) {
					d16[x] = s16[x];
				}
				s16 += 256;
				d16 += linesize;
			}
			break;
		case 32:
			linesize = pitch / 4;
			s32 = screen32;
			d32 = (u32*)((u8*)ddsd.lpSurface + surfoffset);
			for(y=0;y<240;y++) {
				for(x=0;x<256;x++) {
					d32[x] = s32[x];
				}
				s32 += 256;
				d32 += linesize;
			}
			break;
	}*/
	lpSecondaryDDS->Unlock(NULL);
	GetClientRect(hWnd,&rect);
	if((rect.right == 0) || (rect.bottom == 0))
		return;
	ClientToScreen(hWnd,&pt);
	rect.left += pt.x;
	rect.right += pt.x;
	rect.top += pt.y;
	rect.bottom += pt.y;
//	if(config_get_bool("video.fullscreen")) {
//		IDirectDraw7_WaitForVerticalBlank(lpDD,DDWAITVB_BLOCKBEGIN,0);
//	}
//	lpPrimaryDDS->Blt(&rect,lpSecondaryDDS,NULL,DDBLT_ASYNC,NULL);
	lpPrimaryDDS->Blt(&rect,lpSecondaryDDS,NULL,DDBLT_WAIT,NULL);
	if(config_get_bool("video.framelimit")) {
		do {
			t = system_gettick();
		} while((double)(t - lasttime) < interval);
		lasttime = t;
	}
}

//this handles pixels coming directly from the nes engine
void video_updatepixel(int line, int pixel, u8 s)
{
	int offset = (line * 256) + pixel;

	nesscreen[offset] = s;
	switch (screenbpp) {
	case 15:
	case 16:
		screen16[offset] = palettecache16[s];
		break;
	case 32:
		screen32[offset] = palettecache32[s];
		break;
	}
}

#define MAKERGB555(pp) \
	(((pp) >> (3 + 0)) << 0) | \
	(((pp) >> (3 + 8)) << 5) | \
	(((pp) >> (3 + 16)) << 10);

extern "C" void video_updaterawpixel(int line, int pixel, u32 s)
	{
	int offset = (line * 256) + pixel;

	switch (screenbpp) {
	case 15:
	case 16:
		screen16[offset] = MAKERGB555(s);
		break;
	case 32:
		screen32[offset] = s;
		break;
	}
}

//this handles palette changes from the nes engine
void video_updatepalette(u8 addr,u8 data)
{
	palettecache[addr] = data;
	updatepalette(addr,data);
}

void video_setpalette(palette_t *p)
{
	int i,j;
	palentry_t *e;

	//save palette
	for(j=0;j<8;j++) {
		for(i=0;i<64;i++) {
			e = &p->pal[j][i];
			palette[j][(i * 3) + 0] = e->r;
			palette[j][(i * 3) + 1] = e->g;
			palette[j][(i * 3) + 2] = e->b;
		}
	}

	//update the converted palettes
	for(j=0;j<8;j++) {
		for(i=0;i<256;i++) {
			e = &p->pal[j][i & 0x3F];
			palette15[j][i] = ((e->r >> 3) << 0) | ((e->g >> 3) << 5) | ((e->b >> 3) << 10);
			palette16[j][i] = ((e->r >> 3) << 11) | ((e->g >> 2) << 5) | ((e->b >> 3) << 0);
			palette32[j][i] = (e->r << 16) | (e->g << 8) | (e->b << 0);
		}
	}

	filter_palette_changed();
}

u8 *video_getscreen()
{
	return(nesscreen);
}

u8 *video_getpalette()
{
	return(palette[0]);
}

extern "C" int video_zapperhit(int x,int y)
{
	int ret = 0;
	u8 *e,color,pixel,emphasis;

	pixel = nesscreen[x + y * 256];
	emphasis = pixel >> 5;
	color = palettecache[pixel & 0x1F];
	e = &palette[emphasis][color];
	ret += (int)(e[0] * 0.299f);
	ret += (int)(e[1] * 0.587f);
	ret += (int)(e[2] * 0.114f);
	return((ret >= 0x40) ? 1 : 0);
}

//kludge-city!
extern "C" int video_getxoffset()
{
	return((surfoffset / (screenbpp / 8)) % screenw);
}

extern "C" int video_getyoffset()
{
	return((surfoffset / (screenbpp / 8)) / screenw);
}

extern "C" int video_getscale()
{
	return(screenscale);
}
