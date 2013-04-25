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

static u32 data;
static u8 counter;

static u8 read()
{
	return(((data >> counter++) & 1) | 0x40);
}

static void strobe()
{
	data = 0xFFFFFF00;
	if(joykeys[joyconfig[1][0]]) data |= INPUT_A;
	if(joykeys[joyconfig[1][1]]) data |= INPUT_B;
	if(joykeys[joyconfig[1][2]]) data |= INPUT_SELECT;
	if(joykeys[joyconfig[1][3]]) data |= INPUT_START;
	if(joykeys[joyconfig[1][4]]) data |= INPUT_UP;
	if(joykeys[joyconfig[1][5]]) data |= INPUT_DOWN;
	if(joykeys[joyconfig[1][6]]) data |= INPUT_LEFT;
	if(joykeys[joyconfig[1][7]]) data |= INPUT_RIGHT;
	counter = 0;
}

INPUTDEV(I_JOYPAD1,read,0,strobe,0);
