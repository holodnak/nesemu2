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

#include "mappers/fds/calls.h"
#include "mappers/fds/hle.h"

HLECALL(delay132)
{
	int i;

	for(i=0;i<132;i++)
		cpu_tick();
}

HLECALL(delayms)
{
	int delay = 1790 * nes.cpu.y + 5;
	int i;

	for(i=0;i<delay;i++)
		cpu_tick();
}

HLECALL(vintwait)
{
	u8 tmp;

	//save a
	cpu_write(nes.cpu.sp-- | 0x100,nes.cpu.a);

	//save old nmi action
	cpu_write(nes.cpu.sp-- | 0x100,cpu_read(0x100));

	//write new nmi action
	cpu_write(0x100,0);

	//enable nmi
	tmp = cpu_read(0xFF) | 0x80;
	cpu_write(0xFF,tmp);
	cpu_write(0x2000,tmp);

	//clear zero flag, set negative flag
	nes.cpu.flags.z = 0;
	nes.cpu.flags.n = 1;

	log_hle("vintwait!\n");
}
