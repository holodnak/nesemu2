/***************************************************************************
 *   Copyright (C) 2006-2009 by Dead_Body   *
 *   jamesholodnak@gmail.com   *
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
#include "system/config.h"

extern config_t defaults;
extern char path_config[];
extern char curdir[];

void config_defaults()
{
	int i;

	sprintf(defaults.path_rom,"%s/",curdir);
	sprintf(defaults.path_state,"%s/state/",curdir);
	sprintf(defaults.path_cheat,"%s/cheat/",curdir);
	sprintf(defaults.path_shots,"%s/shots/",curdir);
	defaults.fdsbios = 1;
	defaults.showinfo = 1;
	defaults.filter = 0;
	defaults.windowed = 1;
	defaults.windowscale = 2;
	defaults.fullscreen = 3;
	defaults.shotformat = 1;
	defaults.soundenabled = 1;
	defaults.soundchannels = 0xFF;
	defaults.devices[0] = 1;
	defaults.devices[1] = 0;
	defaults.expdevice = 0;
	defaults.hue = (u32)(int)-15;
	defaults.sat = (u32)(int)45;
	defaults.gui_keys[0] = SDLK_ESCAPE;
	defaults.gui_keys[1] = SDLK_F8;
	defaults.gui_keys[2] = SDLK_F5;
	defaults.gui_keys[3] = SDLK_F1;
	defaults.gui_keys[4] = SDLK_F12;
	defaults.gui_keys[5] = SDLK_1;
	defaults.joy_keys[0][0] = SDLK_x;
	defaults.joy_keys[0][1] = SDLK_z;
	defaults.joy_keys[0][2] = SDLK_a;
	defaults.joy_keys[0][3] = SDLK_s;
	defaults.joy_keys[0][4] = SDLK_UP;
	defaults.joy_keys[0][5] = SDLK_DOWN;
	defaults.joy_keys[0][6] = SDLK_LEFT;
	defaults.joy_keys[0][7] = SDLK_RIGHT;
	defaults.joy_keys[0][8] = SDLK_v;
	defaults.joy_keys[0][9] = SDLK_c;
	defaults.joy_keys[1][0] = SDLK_h;
	defaults.joy_keys[1][1] = SDLK_g;
	defaults.joy_keys[1][2] = SDLK_t;
	defaults.joy_keys[1][3] = SDLK_y;
	defaults.joy_keys[1][4] = SDLK_i;
	defaults.joy_keys[1][5] = SDLK_k;
	defaults.joy_keys[1][6] = SDLK_j;
	defaults.joy_keys[1][7] = SDLK_l;
	defaults.pad_keys[0] = SDLK_r;
	defaults.pad_keys[1] = SDLK_t;
	defaults.pad_keys[2] = SDLK_y;
	defaults.pad_keys[3] = SDLK_u;
	defaults.pad_keys[4] = SDLK_f;
	defaults.pad_keys[5] = SDLK_g;
	defaults.pad_keys[6] = SDLK_h;
	defaults.pad_keys[7] = SDLK_j;
	defaults.pad_keys[8] = SDLK_v;
	defaults.pad_keys[9] = SDLK_b;
	defaults.pad_keys[10] = SDLK_n;
	defaults.pad_keys[11] = SDLK_m;
	for(i=0;i<10;i++)
		strcpy(defaults.recent[i],"");
	memcpy(&config,&defaults,sizeof(config_t));
}
