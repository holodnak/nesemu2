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
#include "misc/log.h"
#include "misc/emu.h"
#include "misc/config.h"
#include "nes/nes.h"
#include "nes/state/state.h"
#include "system/system.h"
#include "system/video.h"
#include "system/input.h"
#include "palette/palette.h"
#include "palette/generator.h"
#include "mappers/mapperid.h"
#include "system/sdl/console/console.h"

int quit = 0;
int running = 0;
char romfilename[1024];
char statefilename[1024];
static palette_t *pal = 0;

static int keydown = 0;

int mainloop()
{
	u32 t,total = 0,frames = 0;

	console_init();

//this palette crap could be made common to all system targets...palette_init() maybe?
	if(strcmp(config_get_string("palette.source","generator"),"file") == 0) {
		pal = palette_load(config_get_string("palette.filename","roni.pal"));
	}
	if(pal == 0) {
		pal = palette_generate(config_get_int("palette.generator.hue",-15),config_get_int("palette.generator.saturation",45));
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

		if(nes.cart && (nes.cart->mapperid & B_TYPEMASK) == B_FDS) {
			if(joykeys[SDLK_F9] && (keydown & 0x10) == 0) {
				u8 data[4];

				keydown |= 0x10;
				data[0] = 0;
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

static void showmappers()
{
	int i,n;
	const char *str;

	log_printf("supported ines mappers:  0");
	for(n=1,i=1;i<256;i++) {
		if(mapper_get_mapperid_ines(i) >= 0) {
			log_printf(", %d",i);
			n++;
		}
	}

	log_printf("\nsupported unif mappers:\n");
	for(i=0;;i++) {
		if((str = mapper_get_unif_boardname(i)) == 0)
			break;
		log_printf("   %s\n",str);
	}
	log_printf("\n%d ines mappers, %d unif mappers, %d internal boards supported\n",n,i,B_BOARDEND);
}

int main(int argc,char *argv[])
{
	int ret;

//	if(argc < 2) {
//		log_printf("usage:  %s file.rom\n",argv[0]);
//		return(1);
//	}

	//process command line arguments
	if(argc > 1) {
		if(strcmp("--mappers",argv[1]) == 0) {
			showmappers();
			return(0);
		}

		//set rom filename
		strncpy(romfilename,argv[1],1024);
	}

	//initialize the emulator
	if(emu_init() != 0) {
        log_printf("main:  emu_init() failed\n");
        return(2);
	}

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

	//destroy emulator
	emu_kill();

	//return to os
#if defined(WIN32) && defined(DEBUG)
	system("pause");
#endif
	return(ret);
}
