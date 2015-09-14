/***************************************************************************
 *   Copyright (C) 2015 by James Holodnak                                  *
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

#include "inputdev/inputdev.h"
#include "system/input.h"

static u32 port0, port1;
static u8 counter,strobe,buttons;

static u8 read0()
{
	return(0);
}

static u8 read1()
{
	return(0);
}

static void write(u8 data)
{
}

static void update()
{
}

static int movie(int mode)
{
	return(1);
}

static void state(int mode,u8 *data)
{
//	STATE_U32(portdata);
//	STATE_U8(counter);
//	STATE_U8(strobe);
//	STATE_U8(buttons);
}

INPUTDEV(I_CUSTOM0,read0,write,update,movie,state);
INPUTDEV(I_CUSTOM1,read1,write,update,movie,state);
