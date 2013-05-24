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

HLECALL(enpf)
{
	nes->cpu.a = cpu_read(0xFE) | 0x08;
	cpu_write(0xFE,nes->cpu.a);
	cpu_write(0x2001,nes->cpu.a);
}

HLECALL(dispf)
{
	nes->cpu.a = cpu_read(0xFE) & 0xF7;
	cpu_write(0xFE,nes->cpu.a);
	cpu_write(0x2001,nes->cpu.a);
}

HLECALL(enobj)
{
	nes->cpu.a = cpu_read(0xFE) | 0x10;
	cpu_write(0xFE,nes->cpu.a);
	cpu_write(0x2001,nes->cpu.a);
}

HLECALL(disobj)
{
	nes->cpu.a = cpu_read(0xFE) & 0xEF;
	cpu_write(0xFE,nes->cpu.a);
	cpu_write(0x2001,nes->cpu.a);
}

HLECALL(enpfobj)
{
	hle_enpf();
	hle_enobj();
}

HLECALL(dispfobj)
{
	hle_dispf();
	hle_disobj();
}

HLECALL(setscroll)
{
	cpu_read(0x2002);
	cpu_write(0x2005,cpu_read(0xFD));
	cpu_write(0x2005,cpu_read(0xFC));
	cpu_write(0x2000,cpu_read(0xFF));
}

HLECALL(spritedma)
{
	cpu_write(0x2003,0);		//sprite address
	cpu_write(0x4014,2);		//sprite dma
}
