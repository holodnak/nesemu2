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
#include "emu/emu.h"
#include "emu/commands.h"
#include "misc/log.h"
#include "misc/config.h"
#include "misc/paths.h"
#include "misc/memutil.h"
#include "nes/nes.h"
#include "system/main.h"
#include "system/system.h"
#include "system/video.h"
#include "system/input.h"
#include "palette/palette.h"
#include "palette/generator.h"
#include "system/sdl/console/console.h"

//required
char configfilename[1024] = CONFIG_FILENAME;
char exepath[1024] = "";

int main(int argc,char *argv[])
{
	int i,ret;
	char *p;
	char tmp[1024];

	//clear the tmp string and configfile string
	memset(tmp,0,1024);
	memset(configfilename,0,1024);

	//make the exe path variable
	strcpy(exepath,argv[0]);
	if((p = strrchr(exepath,PATH_SEPERATOR)) != 0) {
		*p = 0;
	}

	//process the command line
	for(i=1;i<argc;i++) {
		if(strcmp("--mappers",argv[i]) == 0) {
			command_execute("mappers");
			return(0);
		}
		else if(strcmp("--config",argv[i]) == 0) {
			strcpy(configfilename,argv[++i]);
		}
		else
			strcpy(tmp,argv[i]);
	}

	//add extra subsystems
	emu_addsubsystem("console",console_init,console_kill);

	//initialize the emulator
	if(emu_init() != 0) {
        log_printf("main:  emu_init() failed\n");
        return(2);
	}

	if(strcmp(tmp,"") != 0) {
		//load file into the nes
		if(nes_load(tmp) == 0) {
			nes_set_inputdev(0,I_JOYPAD0);
			nes_set_inputdev(1,I_JOYPAD1);
			nes_reset(1);
			running = 1;
		}
	}

	//begin the main loop
	ret = emu_mainloop();

	//destroy emulator
	emu_kill();

	//return to os
	return(emu_exit(ret));
}
