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
#include "misc/memutil.h"
#include "palette/palette.h"
#include "system/input.h"
#include "system/video.h"
#include "system/sdl/console/console.h"
#include "system/sdl/console/font.h"

#define YPOS_STEP 13

#define CONSOLE_BUF_SIZE	8192

#define CONSOLE_BGCOLOR		0x0F0FC0
#define CONSOLE_BORDER		0x0F0F80

//console text buffer
static char *buffer = 0;

//showing flag
static int showing;

//console video buffer
static u32 *screen;

//width and height of console
static int width,height;

//y position of bottom of console
static int ypos;

//cursor position
static int cursorpos = 0;

static char statusmsg[1024];
static int statustime = 0;

void console_loghook(char *str)
{
	strncpy(statusmsg,str,1024);
	statustime = 60 * 3;
}

int console_init()
{
	buffer = mem_alloc(CONSOLE_BUF_SIZE);
	showing = 0;
	width = video_getwidth();
	height = video_getheight() * 3 / 5;
	screen = mem_alloc(width * height * sizeof(u32));
	log_sethook(console_loghook);
	printf("console init'd:  width, height = %d, %d\n",width,height);
	return(0);
}

void console_kill()
{
	mem_free(buffer);
	mem_free(screen);
}

void console_draw(u32 *dest,int w,int h)
{
	int x,y;
	u32 *src = screen;
	u32 pixel;

	w /= 4;
	//clear console screen
	for(y=0;y<(height-4);y++) {
		for(x=0;x<width;x++) {
			src[x] = CONSOLE_BGCOLOR;
		}
		src += width;
	}
	for(y=0;y<4;y++) {
		pixel = CONSOLE_BGCOLOR - 0x20 * y;
		for(x=0;x<width;x++) {
			src[x] = pixel;
		}
		src += width;
	}

	font_drawstr("nesemu2!",screen + (width * 100),width);

	src = screen;
	//copy newly rendered screen to dest
	src += (height - ypos) * width;

	//copy the screen
	for(y=0;y<ypos;y++) {
		for(x=0;x<width;x++) {
			pixel = (dest[x] >> 3) & 0x1F1F1F;
			pixel |= src[x] & 0xE0E0E0;
			dest[x] = pixel;
		}
		dest += w;
		src += width;
	}

	//show status message
	if(ypos == 0 && statustime) {
		statusmsg[w / 8] = 0;
		font_drawstr(statusmsg,dest,w);
	}
}

void console_print(char *str)
{
	
}

void console_show()
{
	showing = 1;
}

void console_hide()
{
	showing = 0;
}

//call 60 times a second
void console_update()
{
	static keydown = 0;

	if(joykeys[SDLK_BACKQUOTE] && (keydown & 1) == 0) {
		if(showing == 0)
			console_show();
		else
			console_hide();
		keydown |= 1;
	}
	else if(joykeys[SDLK_BACKQUOTE] == 0)
		keydown &= ~1;
	if(showing) {
		if(ypos < height)	ypos += YPOS_STEP;
		if(ypos > height)	ypos = height;
	}
	else {
		if(ypos > 0)	ypos -= YPOS_STEP;
		if(ypos < 0)	ypos = 0;
	}
	if(statustime)
		statustime--;
}
