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

#include <SDL/SDL.h>
#include <stdio.h>
#include "log/log.h"
#include "emu/emu.h"
#include "nes/nes.h"
#include "system/system.h"

int quit = 0;
char romfilename[_MAX_PATH];

int mainloop()
{
	//load file into the nes
	if(nes_load(romfilename) != 0)
		return(1);

	//reset the nes
	nes_reset(1);

	//main event loop
	while(quit == 0) {
		nes_frame();
		system_check_events();
	}

	return(0);
}

int main(int argc,char *argv[])
{
	int ret;

	if(argc < 2) {
		log_printf("usage:  %s file.rom\n");
		return(1);
	}

	//set rom filename
	strncpy(romfilename,argv[1],_MAX_PATH);

	//initialize the emulator
	emu_init();

	//begin the main loop
	ret = mainloop();

	//destroy emulator
	emu_kill();

	system("pause");

	//return to os
	return(ret);
}
