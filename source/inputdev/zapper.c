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

#include "inputdev.h"
#include "system/video.h"
#include "system/input.h"

static u8 portdata;
static u8 counter,buttons,strobe;
static int xpos,ypos;

extern int video_zapperhit(int x,int y);

static u8 read()
{
	int x = xpos,y = ypos;
	int X,Y;
	int hits = 0;
	u8 ret = 0;

	//check for button press
	if(buttons)
		ret = 0x10;

	//see if zapper is even in range
	if((x < 0) || (x >= 256) || (y < 0) || (y >= 240))
		ret |= 8;

	//check for zapper light data
	else {
		for(Y = y - 8; Y < y + 8; Y++) {
			if(Y < 0)
				Y = 0;
			if(Y < nes->ppu.scanline - 32)
				continue;
			if(Y > nes->ppu.scanline)
				break;
			for(X = x - 8; X < x + 8; X++) {
				if(X < 0)
					X = 0;
				if(X > 255)
					break;
				if((Y == nes->ppu.scanline) && (X >= nes->ppu.linecycles))
					break;
				if(video_zapperhit(X,Y))
					hits++;
			}
		}
	}

	//if valid hit data, then set bit state
	if(hits < 64)
		ret |= 8;

	return(ret);
}

static void write(u8 data)
{
	if(((data & 1) == 0) && (strobe & 1)) {
		portdata = buttons;
		counter = 0;
	}
	strobe = data;
}

static void update()
{
	buttons = input_poll_mouse(&xpos,&ypos);
}

static int movie(int mode)
{
	if(mode & MOVIE_PLAY) {
		buttons = movie_read_u8();
		xpos = (int)movie_read_u8();
		ypos = (int)movie_read_u8();
	}
	else if(mode & MOVIE_RECORD) {
		movie_write_u8(buttons);
		movie_write_u8((u8)xpos);
		movie_write_u8((u8)ypos);
	}
	return(1);
}

static void state(int mode,u8 *data)
{
	STATE_U8(portdata);
	STATE_U8(counter);
	STATE_U8(strobe);
	STATE_U8(buttons);
}

INPUTDEV(I_ZAPPER,read,write,update,movie,state);
