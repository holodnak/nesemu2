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
#include <windows.h>
#include "log/log.h"
#include "emu/emu.h"
#include "nes/nes.h"
#include "system/system.h"
#include "system/video.h"
#include "system/input.h"
#include "palette/palette.h"
#include "palette/generator.h"
#include "inputdev/null.h"
#include "inputdev/joypad0.h"
#include "inputdev/joypad1.h"

int quit = 0;
char romfilename[_MAX_PATH];
static palette_t *pal = 0;

static int keydown = 0;

int mainloop()
{
	u32 t,total = 0;

	//load file into the nes
	if(nes_load(romfilename) != 0) {
		return(1);
	}

	nes_set_inputdev(0,&dev_joypad0);
	nes_set_inputdev(1,&dev_null);
	nes_set_inputdev(2,&dev_null);
	pal = palette_generate(-15,45);
	video_setpalette(pal);

	log_printf("resetting nes...\n");

	//reset the nes
	nes_reset(1);

	log_printf("starting main loop...\n");

	//main event loop
	while(quit == 0) {
		t = GetTickCount();
		nes_frame();
		input_poll();
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
		system_check_events();
		total += GetTickCount() - t;
//		if(total >= 10 * 1000)	quit++;
	}

	log_printf("fps:  %f (%d frames in %f seconds)\n",(double)nes.ppu.frames / (double)total * 1000.0f,nes.ppu.frames,(double)((double)total / 1000.0f));

	palette_destroy(pal);

	return(0);
}

int main(int argc,char *argv[])
{
	int ret;

	if(argc < 2) {
		log_printf("usage:  %s file.rom\n",argv[0]);
		return(1);
	}

	//set rom filename
	strncpy(romfilename,argv[1],_MAX_PATH);

	//initialize the emulator
	if(emu_init() != 0) {
        log_printf("main:  emu_init() failed\n");
        return(2);
	}

	//begin the main loop
	ret = mainloop();

	//destroy emulator
	emu_kill();

	//return to os
	system("pause");
	return(ret);
}
