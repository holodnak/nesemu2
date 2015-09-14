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
#include "version.h"
#include "emu/emu.h"
#include "emu/commands.h"
#include "emu/events.h"
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

//required
char configfilename[1024] = CONFIG_FILENAME;
char exepath[1024] = "";

static void usage(char *argv0)
{
	printf("\nnesemu2 v%s-pi - Copyright 2013-2015 James Holodnak\n\n",VERSION);
	printf("Usage:  %s [options] filename\n\n",argv0);
	printf("Supported ROM formats:  iNES, NES 2.0, UNIF, FDS, NSF\n\n");
	printf("Options:\n\n");
	printf("  --help          : Show this message and exit.\n");
	printf("  --mappers       : Show supported mappers and exit.\n");
	printf("  --config <file> : Use 'file' as configuration file.\n");
	printf("  --patch <file>  : Specify patch file for ROM.\n");
	printf("\n");
}

//todo:  this is getting ugly
int main(int argc,char *argv[])
{
	int i,ret;
	char *p;
	char romfilename[1024];
	char patchfilename[1024];

	//clear the tmp strings and configfile string
	memset(romfilename,0,1024);
	memset(patchfilename,0,1024);
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
		else if(strcmp("--help",argv[i]) == 0) {
			usage(argv[0]);
			return(0);
		}
		else if(strcmp("--config",argv[i]) == 0) {
			strcpy(configfilename,argv[++i]);
		}
		else if(strcmp("--patch",argv[i]) == 0) {
			strcpy(patchfilename,argv[++i]);
		}
		else
			strcpy(romfilename,argv[i]);
	}

	//initialize the emulator
	if(emu_init() != 0) {
        log_printf("main:  emu_init() failed\n");
        return(2);
	}

	//load rom specified by arguments
	if(strcmp(romfilename,"") != 0) {
		emu_event(E_LOADROM,(void*)romfilename);
	}

	//load patch
	if(strcmp(patchfilename,"") != 0) {
		emu_event(E_LOADPATCH,(void*)patchfilename);
	}

	//begin emulation
	ret = emu_mainloop();

	//destroy emulator
	emu_kill();

	//return to os
	return(emu_exit(ret));
}
