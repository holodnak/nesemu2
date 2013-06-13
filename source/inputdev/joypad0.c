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
#include "system/input.h"

static u32 portdata;
static u8 counter,strobe,buttons;

static u8 read()
{
	if(strobe) {
		portdata = 0xFFFFFF00 | buttons;
		counter = 0;
		return((u8)portdata & 1);
	}
	return(((portdata >> counter++) & 1) | 0x40);
}

static void write(u8 data)
{
	data &= 1;
	if(data || strobe) {
		strobe = data;
		portdata = 0xFFFFFF00 | buttons;
		counter = 0;
	}
}

static void update()
{
	buttons = 0;
	if(joykeys[joyconfig[0][0]]) buttons |= INPUT_A;
	if(joykeys[joyconfig[0][1]]) buttons |= INPUT_B;
	if(joykeys[joyconfig[0][2]]) buttons |= INPUT_SELECT;
	if(joykeys[joyconfig[0][3]]) buttons |= INPUT_START;
	if(joykeys[joyconfig[0][4]]) buttons |= INPUT_UP;
	if(joykeys[joyconfig[0][5]]) buttons |= INPUT_DOWN;
	if(joykeys[joyconfig[0][6]]) buttons |= INPUT_LEFT;
	if(joykeys[joyconfig[0][7]]) buttons |= INPUT_RIGHT;
}

static int movie(int mode)
{
	if(mode & MOVIE_PLAY) {
		buttons = nes_movie_read_u8();
	}
	else if(mode & MOVIE_RECORD) {
		nes_movie_write_u8(buttons);
	}
	return(1);
}

static void state(int mode,u8 *data)
{
	STATE_U32(portdata);
	STATE_U8(counter);
	STATE_U8(strobe);
	STATE_U8(buttons);
}

INPUTDEV(I_JOYPAD0,read,write,update,movie,state);
