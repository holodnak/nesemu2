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

HLECALL(xferdone)
{
	log_printf("xferdone:  not implemented\n");
	//no error
	if(nes.cpu.flags.z == 0) {
		nes.cpu.a = (cpu_read(0xFA) & 9) | 0x26;
		cpu_write(0xFA,nes.cpu.a);
		cpu_write(0x4025,nes.cpu.a);
		nes.cpu.a = nes.cpu.x = 0;
		nes.cpu.flags.i = 0;
		return;
	}
	else {

	}
}
