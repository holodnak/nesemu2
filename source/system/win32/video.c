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

#include <windows.h>
#include "misc/log.h"
#include "palette/palette.h"
#include "system/video.h"

static u32 palette32[256];
static u32 palettecache[32];
static palette_t *palette = 0;
static u32 interval = 1000 / 60;
static u32 lasttime = 0;

int video_reinit()
{
	return(0);
}

int video_init()
{
	int ret = video_reinit();

	return(ret);
}

void video_kill()
{
}

void video_startframe()
{
}

void video_endframe()
{
	u32 t;

	//simple frame limiter
	t = GetTickCount();
#ifdef FRAMELIMIT
	while((t - lasttime) < interval) {
		t = GetTickCount();
	}
#endif
	lasttime = t;
}

//this handles lines coming directly from the nes engine
void video_updateline(int line,u8 *s)
{
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
