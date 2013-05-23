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
#include "mappers/fds/fds.h"

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

HLECALL(checkdiskheader)
{
	log_printf("checkdiskheader:  not implemented\n");
}

HLECALL(getnumfiles)
{
	fds_disk_header_t *disk_header;
	int pos;

	//position of disk data start
	pos = diskside * 65500;

	//pointer to disk header
	disk_header = (fds_disk_header_t*)(nes.cart->disk.data + pos);

	cpu_write(6,disk_header->numfiles);

	log_printf("getnumfiles:  %d files on disk %d side %d\n",disk_header->numfiles,diskside >> 1,diskside & 1);
}

HLECALL(checkblocktype)
{
	log_printf("checkblocktype:  not implemented\n");
}

HLECALL(filematchtest)
{
	log_printf("filematchtest:  not implemented\n");
}

HLECALL(loaddata)
{
	log_printf("loaddata:  not implemented\n");
}
