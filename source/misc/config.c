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
#include "misc/config.h"
#include "misc/vars.h"
#include "misc/memutil.h"
#include "misc/log.h"
#include "system/main.h"

static vars_t *configvars = 0;

config_t *config = 0;

#define GET_VAR_INT(name,def) \
	config-> name = vars_get_int(v,#name,def);

#define GET_VAR_STR(name,def) \
	p = vars_get_string(v,#name,def); \
	strcpy(config-> name,p);

#define SET_VAR_INT(name) \
	vars_set_int(v,#name,config-> name);

#define SET_VAR_STR(name) \
	vars_set_string(v,#name,config-> name);

int config_init()
{
	char *p;
	vars_t *v;

	config = (config_t*)mem_alloc(sizeof(config_t));
	memset(config,0,sizeof(config_t));
	if((v = vars_load(configfilename)) == 0) {
		log_printf("config_init:  unable to load file, using defaults\n");
		v = vars_create();
	}

	GET_VAR_INT(video.framelimit,			1);
	GET_VAR_INT(video.fullscreen,			0);
	GET_VAR_INT(video.scale,				1);
	GET_VAR_STR(video.filter,				"none");

	GET_VAR_STR(input.port0,				"joypad0");
	GET_VAR_STR(input.port1,				"joypad1");
	GET_VAR_STR(input.expansion,			"none");

	GET_VAR_INT(input.joypad0.a,			'x');
	GET_VAR_INT(input.joypad0.b,			'z');
	GET_VAR_INT(input.joypad0.select,	'a');
	GET_VAR_INT(input.joypad0.start,		's');
	GET_VAR_INT(input.joypad0.up,			273);
	GET_VAR_INT(input.joypad0.down,		274);
	GET_VAR_INT(input.joypad0.left,		276);
	GET_VAR_INT(input.joypad0.right,		275);

	GET_VAR_INT(sound.enabled,				1);

#ifdef WIN32
	GET_VAR_STR(path.data,					"%exepath%/data");
#else
	GET_VAR_STR(path.data,					"%home%/.nesemu2/data");
#endif

	GET_VAR_STR(path.roms,					"%config.path.data%/roms");
	GET_VAR_STR(path.bios,					"%config.path.data%/bios");
	GET_VAR_STR(path.save,					"%config.path.data%/save");
	GET_VAR_STR(path.state,					"%config.path.data%/state");
	GET_VAR_STR(path.palette,				"%config.path.data%/palette");

	GET_VAR_STR(palette.source,			"generator");
	GET_VAR_INT(palette.hue,				-15);
	GET_VAR_INT(palette.saturation,		45);
	GET_VAR_STR(palette.filename,			"roni.pal");

	GET_VAR_STR(nes.gamegenie.bios,		"genie.rom");
	GET_VAR_INT(nes.gamegenie.enabled,	0);

	GET_VAR_STR(nes.fds.bios,				"disksys.rom");
	GET_VAR_INT(nes.fds.hle,				1);

	GET_VAR_INT(nes.log_unhandled_io,	0);
	GET_VAR_INT(nes.pause_on_load,		0);

	configvars = v;
	return(0);
}

void config_kill()
{
	vars_t *v = configvars;

	if(v == 0) {
		log_printf("config_kill:  internal error!  configvars = 0.\n");
		return;
	}

	SET_VAR_INT(video.framelimit);
	SET_VAR_INT(video.fullscreen);
	SET_VAR_INT(video.scale);
	SET_VAR_STR(video.filter);

	SET_VAR_STR(input.port0);
	SET_VAR_STR(input.port1);
	SET_VAR_STR(input.expansion);

	SET_VAR_INT(input.joypad0.a);
	SET_VAR_INT(input.joypad0.b);
	SET_VAR_INT(input.joypad0.select);
	SET_VAR_INT(input.joypad0.start);
	SET_VAR_INT(input.joypad0.up);
	SET_VAR_INT(input.joypad0.down);
	SET_VAR_INT(input.joypad0.left);
	SET_VAR_INT(input.joypad0.right);

	SET_VAR_INT(sound.enabled);

	SET_VAR_STR(path.data);
	SET_VAR_STR(path.roms);
	SET_VAR_STR(path.bios);
	SET_VAR_STR(path.save);
	SET_VAR_STR(path.state);
	SET_VAR_STR(path.palette);

	SET_VAR_STR(palette.source);
	SET_VAR_INT(palette.hue);
	SET_VAR_INT(palette.saturation);
	SET_VAR_STR(palette.filename);

	SET_VAR_STR(nes.gamegenie.bios);
	SET_VAR_INT(nes.gamegenie.enabled);

	SET_VAR_STR(nes.fds.bios);
	SET_VAR_INT(nes.fds.hle);

	SET_VAR_INT(nes.log_unhandled_io);
	SET_VAR_INT(nes.pause_on_load);

	vars_save(v,configfilename);
	vars_destroy(v);
	mem_free(config);
}

//update the internal config var list with data from the config struct
//todo: this should only update the var list if the variable changed
void config_update()
{
	vars_t *v = configvars;

	if(v == 0) {
		log_printf("config_update:  internal error!  configvars = 0.\n");
		return;
	}

	SET_VAR_INT(video.framelimit);
	SET_VAR_INT(video.fullscreen);
	SET_VAR_INT(video.scale);
	SET_VAR_STR(video.filter);

	SET_VAR_STR(input.port0);
	SET_VAR_STR(input.port1);
	SET_VAR_STR(input.expansion);

	SET_VAR_INT(input.joypad0.a);
	SET_VAR_INT(input.joypad0.b);
	SET_VAR_INT(input.joypad0.select);
	SET_VAR_INT(input.joypad0.start);
	SET_VAR_INT(input.joypad0.up);
	SET_VAR_INT(input.joypad0.down);
	SET_VAR_INT(input.joypad0.left);
	SET_VAR_INT(input.joypad0.right);

	SET_VAR_INT(sound.enabled);

	SET_VAR_STR(path.data);
	SET_VAR_STR(path.roms);

	SET_VAR_STR(palette.source);
	SET_VAR_INT(palette.hue);
	SET_VAR_INT(palette.saturation);
	SET_VAR_STR(palette.filename);

	SET_VAR_STR(nes.gamegenie.bios);
	SET_VAR_INT(nes.gamegenie.enabled);

	SET_VAR_STR(nes.fds.bios);
	SET_VAR_INT(nes.fds.hle);

	SET_VAR_INT(nes.log_unhandled_io);
	SET_VAR_INT(nes.pause_on_load);
}

//kludge for the path parser
char *config_get_string(char *name,char *def)
{
	return(vars_get_string(configvars,name,def));
}
