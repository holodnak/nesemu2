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
#include "nes/state/state.h"
#include "system/main.h"
#include "system/system.h"
#include "system/video.h"
#include "system/input.h"
#include "palette/palette.h"
#include "palette/generator.h"
#include "mappers/mapperid.h"
#include "system/sdl/console/console.h"

//required
int quit = 0;
int running = 0;
char configfilename[1024] = CONFIG_FILENAME;
char exepath[1024] = "";

char datapath[1024];
char romfilename[1024];
char statefilename[1024];
static palette_t *pal = 0;

static int keydown = 0;

int mainloop()
{
	u32 t,total = 0,frames = 0;

	console_init();

//this palette crap could be made common to all system targets...palette_init() maybe?
	if(strcmp(config->palette.source,"file") == 0) {
		pal = palette_load(config->palette.filename);
	}
	if(pal == 0) {
		pal = palette_generate(config->palette.hue,config->palette.saturation);
	}
	video_setpalette(pal);

	log_printf("starting main loop...\n");

	//main event loop
	while(quit == 0) {
		t = SDL_GetTicks();
		if(running)
			nes_frame();
		video_startframe();
		video_endframe();
		input_poll();
		console_update();
		if(joykeys[SDLK_p]) {
			keydown |= 1;
		}
		else if(keydown & 1) {
			nes_reset(0);
			keydown &= ~1;
		}
		if(joykeys[SDLK_o]) {
			keydown |= 2;
		}
		else if(keydown & 2) {
			nes_reset(1);
			keydown &= ~2;
		}
		if(joykeys[SDLK_F5]) {
			keydown |= 4;
		}
		else if(keydown & 4) {
			nes_savestate(statefilename);
			keydown &= ~4;
		}
		if(joykeys[SDLK_F8]) {
			keydown |= 8;
		}
		else if(keydown & 8) {
			nes_loadstate(statefilename);
			keydown &= ~8;
		}
		if(joykeys[SDLK_F4] && (keydown & 0x80) == 0) {
			keydown |= 0x80;
			running ^= 1;
		}
		else if(joykeys[SDLK_F4] == 0) {
			keydown &= ~0x80;
		}

		if(joykeys[SDLK_F11] && (keydown & 0x8000) == 0) {
			FILE *fp;

			keydown |= 0x8000;
			log_printf("dumping disk as dump.fds\n");
			if((fp = fopen("dump.fds","wb")) != 0) {
				fwrite(nes.cart->disk.data,1,nes.cart->disk.size,fp);
				fclose(fp);
			}

		}
		else if(joykeys[SDLK_F11] == 0) {
			keydown &= ~0x8000;
		}

		if(nes.cart && (nes.cart->mapperid & B_TYPEMASK) == B_FDS) {
			if(joykeys[SDLK_F9] && (keydown & 0x10) == 0) {
				u8 data[4] = {0,0,0,0};

				keydown |= 0x10;
				nes.mapper->state(CFG_SAVE,data);
				if(data[0] == 0xFF)
					data[0] = 0;
				else
					data[0] ^= 1;
				nes.mapper->state(CFG_LOAD,data);
				log_printf("disk inserted!  side = %d\n",data[0]);
			}
			else if(joykeys[SDLK_F9] == 0) {
				keydown &= ~0x10;
			}

		}
		if(joykeys[SDLK_ESCAPE]) {
			quit++;
		}
		system_check_events();
		total += SDL_GetTicks() - t;
		frames++;
	}

	log_printf("fps:  %f (%d frames in %f seconds)\n",(double)frames / (double)total * 1000.0f,frames,(double)((double)total / 1000.0f));

	palette_destroy(pal);

	console_kill();

	return(0);
}

static void mkdirr(char *path)
{
	char *tmp = mem_strdup(path);
	char *p = tmp;
	int num = 0;

	for(p=tmp;*p;p++) {
		if(*p == '/' || *p == '\\')
			*p = PATH_SEPERATOR;
	}
	log_printf("mkdirr:  creating directory '%s'\n",path);
	for(num=0,p=tmp;(p = strchr(p,PATH_SEPERATOR));num++) {
		if(num == 0) {
			p++;
			continue;
		}
		*p = 0;
		mkdir(path);
		*p = PATH_SEPERATOR;
		p++;
	}
	mem_free(tmp);
}

int main(int argc,char *argv[])
{
	int i,ret;
	char *p;
	char tmp[1024];

	//set default configuration filename
	strcpy(configfilename,CONFIG_FILENAME);

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
			strcpy(romfilename,argv[i]);
	}

	//initialize the emulator
	if(emu_init() != 0) {
        log_printf("main:  emu_init() failed\n");
        return(2);
	}

	//make the directories
	paths_parse(config->path.save,tmp,1024);
	mkdirr(tmp);
	paths_parse(config->path.state,tmp,1024);
	mkdirr(tmp);

	if(strcmp(romfilename,"") != 0) {
		//load file into the nes
		if(nes_load(romfilename) == 0) {
			nes_set_inputdev(0,I_JOYPAD0);
			nes_set_inputdev(1,I_JOYPAD1);
			nes_reset(1);
			strncpy(statefilename,romfilename,1024);
			strcat(statefilename,".state");
			running = 1;
		}
	}

	//begin the main loop
	ret = mainloop();

	//check if a cart was loaded
	if(nes.cart) {
		//save sram
		//save disk
		nes_unload();
	}
	
	//destroy emulator
	emu_kill();

	//return to os
	return(ret);
}
