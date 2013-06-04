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

#include "mappers/mapperinc.h"
#include "mappers/chips/latch.h"

static void (*sync)();
u8 latch_data;
u32 latch_addr;

void latch_write(u32 addr,u8 data)
{
	latch_addr = addr;
	latch_data = data;
	sync();
}

void latch_reset(void (*s)(),int hard)
{
	int i;

	sync = s;
	for(i=8;i<16;i++)
		mem_setwritefunc(i,latch_write);
	if(hard) {
		latch_data = 0;
		latch_addr = 0;
	}
	sync();
}

void latch_state(int mode,u8 *data)
{
	STATE_U8(latch_data);
	STATE_U16(latch_addr);
	sync();
}
