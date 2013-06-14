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

#include <string.h>
#include "emu/emu.h"
#include "misc/log.h"
#include "misc/memutil.h"
#include "misc/config.h"
#include "misc/crc32.h"
#include "cartdb/cartdb.h"
#include "system/system.h"
#include "system/video.h"
#include "system/input.h"
#include "system/sound.h"
#include "nes/nes.h"

#define SUBSYSTEM_START	static subsystem_t subsystems[32] = {
#define SUBSYSTEM(n)				{50,"" #n "", n ## _init, n ## _kill},
#define SUBSYSTEM_NOINIT(n)	{50,"" #n "",          0, n ## _kill},
#define SUBSYSTEM_NOKILL(n)	{50,"" #n "", n ## _init,          0},
#define SUBSYSTEM_END	{-1,"",0,0}};

typedef struct subsystem_s {
	int			id;
	char			name[32];
	initfunc_t	init;
	killfunc_t	kill;
} subsystem_t;

//static subsystem table
SUBSYSTEM_START
	SUBSYSTEM(memutil)
	SUBSYSTEM(config)
	SUBSYSTEM_NOKILL(log)
	SUBSYSTEM(cartdb)
	SUBSYSTEM(system)
	SUBSYSTEM(video)
	SUBSYSTEM(input)
	SUBSYSTEM(sound)
	SUBSYSTEM(palette)
	SUBSYSTEM(nes)
SUBSYSTEM_END

int quit,running;

int emu_init()
{
	int i;

	//initialize the control variables
	quit = 0;
	running = 0;

	//generate crc32 table
	crc32_gentab();

	//loop thru the subsystem function pointers and init
	for(i=0;subsystems[i].id != -1;i++) {
		if(subsystems[i].init) {
			log_printf("initing '%s'\n",subsystems[i].name);
			if(subsystems[i].init() != 0) {
				emu_kill();
				return(1);
			}
		}
	}
	return(0);
}

void emu_kill()
{
	int i;

	//find the end of the pointer list ('i' will equal last one + 1)
	for(i=0;subsystems[i].id != -1;i++);

	//loop thru the subsystem function pointers backwards and kill
	for(i--;i>=0;i--) {
		if(subsystems[i].kill != 0) {
			log_printf("killing '%s'\n",subsystems[i].name);
			subsystems[i].kill();
		}
	}
}

int emu_exit(int ret)
{
	//kludge alert!  need list here or something atexit() style...
	log_kill();
	return(ret);
}

int emu_addsubsystem(char *name,initfunc_t init,killfunc_t kill)
{
	int i;

	log_printf("emu_addsubsystem:  adding subsystem '%s'\n",name);
	for(i=0;subsystems[i].id != -1;i++);
	subsystems[i].id = 50;
	strcpy(subsystems[i].name,name);
	subsystems[i].init = init;
	subsystems[i].kill = kill;
	subsystems[i+1].id = -1;
	strcpy(subsystems[i+1].name,"");
	subsystems[i+1].init = 0;
	subsystems[i+1].kill = 0;
	return(0);
}

int emu_mainloop()
{
	u8 *line = (u8*)mem_alloc(512);
	int i;
	u64 t,total,frames;

	//initialize the palette in case the rom isnt loaded first
	for(i=0;i<512;i++) {
		if(i < 32 && running == 0)
			video_updatepalette(i,i);
		line[i] = i;
	}

	//begin the main loop
	log_printf("emu_mainloop:  starting main loop...\n");
	total = 0;
	frames = 0;
	while(quit == 0) {
		t = system_gettick();
		system_checkevents();
		input_poll();
		video_startframe();
		if(running && nes->cart) {
			nes_frame();
		}
		else {
			for(i=0;i<240;i++) {
				video_updateline(i,line);
			}
		}
		video_endframe();
		total += system_gettick() - t;
		frames++;
	}
	log_printf("fps:  %f (%d frames)\n",(double)frames / (double)total * system_getfrequency(),frames);
	mem_free(line);
	return(0);
}
