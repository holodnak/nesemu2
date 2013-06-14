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
}

typedef struct filter_s {
	int (*init);
	void (*kill);
	void (*draw)(void*,void*,int);
	int (*getminwidth)();
	int (*getminheight)();
} filter_t;

#undef _MSC_VER

#if (_MSC_VER >= 1400)
typedef HRESULT (WINAPI *LPDIRECTDRAWCREATEEX)(GUID FAR *, LPVOID *, REFIID,IUnknown FAR *);
HINSTANCE ddrawdll;
LPDIRECTDRAWCREATEEX DirectDrawCreateEx;
#endif

static u8 *screen = 0;
static int screenw,screenh,screenbpp;
static int screenscale;
static u16 palette15[8][256];
static u16 palette16[8][256];
static u32 palette32[8][256];
static u8 palettecache[32];
static u16 palettecache16[256];
static u32 palettecache32[256];
static palette_t *palette = 0;
static double interval = 0;
static u64 lasttime = 0;
static int filter = 0;

static int pitch;
static int surfoffset;

static HINSTANCE ddrawdll;
static LPDIRECTDRAW7 lpDD;
static LPDIRECTDRAWSURFACE7 lpPrimaryDDS,lpSecondaryDDS;
static LPDIRECTDRAWCLIPPER lpDirectDrawClipper;
static DDSURFACEDESC2 ddsd;
static HMENU hMenu = 0;
static RECT rect;

static void (*drawline)(int,u8*) = 0;
static void (*blankline)(int) = 0;
static void (*updatepalette)(u32,u8) = 0;

static void drawline1x_16(int line,u8 *src)
{
	u16 *d = (u16*)((u8*)ddsd.lpSurface + (line * pitch) + surfoffset);
	int i;

	for(i=0;i<256;i++) {
		*d++ = palettecache16[src[i]];
	}
}

static void drawline1x_32(int line,u8 *src)
{
	u32 *d = (u32*)((u8*)ddsd.lpSurface + (line * pitch) + surfoffset);
	int i;

	for(i=0;i<256;i++) {
		*d++ = palettecache32[src[i]];
	}
}

static void drawline2x_16(int line,u8 *src)
{
	u16 *d1 = (u16*)((u8*)ddsd.lpSurface + (line * 2 * pitch) + surfoffset);
	u16 *d2 = (u16*)((u8*)d1 + pitch);
	u16 pixel;
	int i;

	for(i=0;i<256;i++) {
		pixel = palettecache16[src[i]];
		*d1++ = pixel;
		*d1++ = pixel;
		*d2++ = pixel;
		*d2++ = pixel;
	}
}

static void drawline2x_32(int line,u8 *src)
{
	u32 *d1 = (u32*)((u8*)ddsd.lpSurface + (line * 2 * pitch) + surfoffset);
	u32 *d2 = (u32*)((u8*)d1 + pitch);
	u32 pixel;
	int i;

	for(i=0;i<256;i++) {
		pixel = palettecache32[src[i]];
		*d1++ = pixel;
		*d1++ = pixel;
		*d2++ = pixel;
		*d2++ = pixel;
	}
}

static void blankline1x_16(int line)
{
	u16 *d = (u16*)((u8*)ddsd.lpSurface + (line * pitch) + surfoffset);
	int i;

	for(i=0;i<256;i++) {
		*d++ = 0;
	}
}

static void blankline1x_32(int line)
{
	u32 *d = (u32*)((u8*)ddsd.lpSurface + (line * pitch) + surfoffset);
	int i;

	for(i=0;i<256;i++) {
		*d++ = 0;
	}
}

static void blankline2x_16(int line)
{
	u16 *d1 = (u16*)((u8*)ddsd.lpSurface + (line * 2 * pitch) + surfoffset);
	u16 *d2 = (u16*)((u8*)d1 + pitch);
	int i;

	for(i=0;i<256;i++) {
		*d1++ = 0;
		*d1++ = 0;
		*d2++ = 0;
		*d2++ = 0;
	}
}

static void blankline2x_32(int line)
{
	u32 *d1 = (u32*)((u8*)ddsd.lpSurface + (line * 2 * pitch) + surfoffset);
	u32 *d2 = (u32*)((u8*)d1 + pitch);
	int i;

	for(i=0;i<256;i++) {
		*d1++ = 0;
		*d1++ = 0;
		*d2++ = 0;
		*d2++ = 0;
	}
}

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
	GetWindowRect(hWnd,&rect);
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

static int getfilterint(char *str)
{
	if(stricmp("Scanline",str) == 0)			return(1);
	if(stricmp("Interpolate",str) == 0)		return(2);
	if(stricmp("Scale",str) == 0)				return(3);
	if(stricmp("NTSC",str) == 0)				return(4);
	return(0);
}

static int video_reinit()
{
	int i,ret;

	if(screen == 0)
		screen = (u8*)mem_alloc(256 * (240 + 16));
	filter = getfilterint(config_get_string("video.filter"));
	screenscale = config_get_int("video.scale");
	screenw = 256 * screenscale;
	screenh = 240 * screenscale;
	screenbpp = 32;
	if(config_get_bool("video.fullscreen")) {
		ret = initfullscreen();
	}
	else {
		ret = initwindowed();
		resizeclient(hWnd,screenw,screenh);
	}
	if(ret == 0) {
		if(FAILED(lpSecondaryDDS->GetSurfaceDesc(&ddsd))) {
			MessageBox(hWnd,"Failed to retrieve surface description","nesemu2",MB_OK | MB_ICONERROR);
			return(1);
		}
		video_startframe();
		memset(ddsd.lpSurface,0,ddsd.lPitch * ddsd.dwHeight);
		video_endframe();
		pitch = ddsd.lPitch;
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
				drawline = drawline1x_16;
				blankline = blankline1x_16;
				if(screenscale == 2) {
					drawline = drawline2x_16;
					blankline = blankline2x_16;
				}
				break;
			case 32:
				screenbpp = 32;
				updatepalette = updatepalette32;
				drawline = drawline1x_32;
				blankline = blankline1x_32;
				if(screenscale == 2) {
					drawline = drawline2x_32;
					blankline = blankline2x_32;
				}
				break;
			default:
				MessageBox(hWnd,"Error detecting bits per pixel","nesemu2",MB_OK | MB_ICONERROR);
				return(1);
		}
		for(i=0;i<32;i++)
			updatepalette(i,palettecache[i]);
		log_printf("video_reinit:  ddraw video inited, %ix%i %ibpp\n",ddsd.dwWidth,ddsd.dwHeight,screenbpp);
	}
	return(ret);
}

int video_init()
{
	int ret;

	interval = (double)system_getfrequency() / 60.0f;
	lasttime = system_gettick();
	hMenu = GetMenu(hWnd);
	if(initddraw() != 0) {
		killddraw();
		return(1);
	}
	if((ret = video_reinit())) {
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
	SAFE_RELEASE(lpDirectDrawClipper);
	SAFE_RELEASE(lpSecondaryDDS);
	SAFE_RELEASE(lpPrimaryDDS);
	if(config_get_bool("video.fullscreen")) {
		ShowCursor(TRUE);
		SetWindowPos(hWnd,0,rect.left,rect.top,0,0,SWP_NOCOPYBITS | SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER);
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

void video_startframe()
{
	lpSecondaryDDS->Lock(NULL,&ddsd,DDLOCK_WAIT | DDLOCK_NOSYSLOCK | DDLOCK_WRITEONLY,NULL);
}

void video_endframe()
{
	RECT rect;
	POINT pt = {0, 0};
	u64 t;

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

//this handles lines coming directly from the nes engine
void video_updateline(int line,u8 *s)
{
	memcpy(screen + (line * 256),s,256);
	if(line >= 8 && line < 232)
		drawline(line,s);
	else
		blankline(line);
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

	palette = p;
	for(j=0;j<8;j++) {
		for(i=0;i<256;i++) {
			e = &p->pal[j][i & 0x3F];
			palette15[j][i] = ((e->r >> 3) << 0) | ((e->g >> 3) << 5) | ((e->b >> 3) << 10);
			palette16[j][i] = ((e->r >> 3) << 11) | ((e->g >> 2) << 5) | ((e->b >> 3) << 0);
			palette32[j][i] = (e->r << 16) | (e->g << 8) | (e->b << 0);
		}
	}
}

extern "C" void video_resize()
{
//	GetClientRect(hWnd,&rect);
}

u8 *video_getscreen()
{
	return(screen);
}