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
#include "emu/commands.h"
#include "misc/log.h"
#include "misc/memutil.h"
#include "palette/palette.h"
#include "system/input.h"
#include "system/video.h"
#include "system/sdl/console/console.h"
#include "system/sdl/console/font.h"
#include "system/sdl/console/linebuffer.h"

#define YPOS_STEP 13

#define CONSOLE_BGCOLOR		0x0F0FC0
#define CONSOLE_BORDER		0x0F0F80

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

//last message outputted
static char statusmsg[1024];
static int statustime = 0;

//input buffer
static char inputbuf[1024];
static int inputbuflen,inputbufpos;
static int cursorblink;

void console_loghook(char *str)
{
	linebuffer_add(str);
	strncpy(statusmsg,str,1024);
	statustime = 60 * 3;
}

int console_init()
{
	int ret = 0;

	memset(inputbuf,0,1024);	
	inputbuflen = inputbufpos = 0;
	cursorblink = 0;
	showing = 0;
	width = video_getwidth();
	height = video_getheight() * 3 / 5 + 4;
	screen = mem_alloc(width * height * sizeof(u32));
	ret += linebuffer_init((width - 8) / 8);
	log_sethook(console_loghook);
	log_printf("console init'd:  width, height = %d, %d\n",width,height);
	return(ret);
}

void console_kill()
{
	log_sethook(0);
	linebuffer_kill();
	if(screen)
		mem_free(screen);
}

void console_draw(u32 *dest,int w,int h)
{
	int x,y,i,n;
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

	//draw faded line at the bottom
	for(y=0;y<4;y++) {
		pixel = CONSOLE_BGCOLOR - (0x10 * y);
		for(x=0;x<width;x++) {
			src[x] = pixel;
		}
		src += width;
	}

	//draw the seperator
	src = screen + (height - 11) * width;
	for(y=0;y<1;y++) {
		pixel = 0xbbbbbb;
		for(x=3;x<(width-3);x++) {
			src[x] = pixel;
		}
		src += width;
	}

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

	//output the text
	n = MIN((height / 8) - 1,linebuffer_count());
	y = (height / 8) - 2;
	for(i=0;i<n;i++) {
		char *str = linebuffer_getrelative(i);

		if(str)
			font_drawstr(str,screen + 4 + (y * width * 8),width);
		y--;
	}

	//draw the prompt
	font_drawchar(']',screen + 2 + ((height - 9) * width),width);

	//draw the input buffer
	font_drawstr(inputbuf,screen + 10 + ((height - 9) * width),width);

	//draw the blinking cursor
	if(cursorblink & 0x20)
		font_drawchar('_',screen + 10 + ((height - 9) * width) + (inputbufpos * 8),width);

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
	cursorblink = (cursorblink + 1) & 0x3F;
//	cursorblink++;
}

static int isignored(int ch)
{
	if (ch == '`' || ch == '~')
		return(1);
	return(0);
}

static int isletter(int ch)
{
	if (ch >= SDLK_a && ch <= SDLK_z)
		return(1);
	return(0);
}

static int isnumber(int ch)
{
	if (ch >= SDLK_0 && ch <= SDLK_9)
		return(1);
	return(0);
}

void console_keyevent(int state,int sym)
{
	int modstate = SDL_GetModState();
	int isshift = modstate & (KMOD_LSHIFT | KMOD_RSHIFT);
	int iscaps = modstate & KMOD_CAPS;
	int i,ch = -1;

	//ignore keyup events
	if(state == SDL_KEYUP)
		return;

	//if shift is pressed invert caps lock state
	if(isshift)
		iscaps ^= KMOD_CAPS;

	//cursor left
	if (sym == SDLK_LEFT) {
		if (inputbufpos)
			inputbufpos--;
		return;
	}

	//cursor right
	else if (sym == SDLK_RIGHT) {
		if (inputbufpos < inputbuflen)
			inputbufpos++;
		return;
	}

	//if this is a letter
	else if(isletter(sym) || isnumber(sym)) {
		ch = sym - SDLK_a;
		if(iscaps)
			ch += 'A';
		else
			ch += 'a';
	}

	//if this is a printable character and we are not ignoring it
//	else if(isprint(sym) && isignored(sym) == 0) {
//		ch = sym;
//	}

	//backspace
	else if(sym == SDLK_BACKSPACE) {
		if(inputbufpos) {
			inputbuflen--;
			inputbufpos--;
			for(i=inputbufpos;i<inputbuflen;i++) {
				inputbuf[i] = inputbuf[i+1];
			}
			inputbuf[i] = 0;
		}
		return;
	}

	else if(sym == SDLK_RETURN) {
		command_execute(inputbuf);
		inputbufpos = inputbuflen = 0;
		memset(inputbuf,0,1024);
		return;
	}

	//unknown key, ignore it
	else {
//		log_printf("ignored key $%02X (%c)",sym,sym);
		return;
	}

	//add key to input buffer
	inputbuf[inputbufpos] = (char)ch;

	//see if it is at the end of the input buffer
	if(inputbufpos == inputbuflen) {
		inputbufpos++;
		inputbuflen++;
		inputbuf[inputbufpos] = 0;
	}

	else if (inputbufpos < inputbuflen) {
		inputbufpos++;
	}

	else {
		log_printf("console_keyevent:  bug!");
	}

//	log_printf("console_keyevent:  pos = %d, len = %d",inputbufpos,inputbuflen);

}
