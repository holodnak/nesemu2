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

static u8 portdata;
static u8 counter,buttons,strobe;

static u8 read()
{
	return(portdata | 0x40);
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
	buttons = 0;
}

static void state(int mode,u8 *data)
{
	STATE_U8(portdata);
	STATE_U8(counter);
	STATE_U8(strobe);
	STATE_U8(buttons);
}

INPUTDEV(I_ZAPPER,read,write,update,0,state);
