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

#ifndef __config_h__
#define __config_h__

#include "misc/vars.h"

/*typedef struct config_s {

	struct {
		int framelimit;
		int fullscreen;
		int scale;
		char filter[64];
	} video;

	struct {
		char port0[64],port1[64];
		char expansion[64];
		struct {
			int a,b;
			int select,start;
			int up,down,left,right;
		} joypad0,joypad1;
	} input;

	struct {
		int enabled;
	} sound;

	struct {
		char data[1024];
		char roms[1024];
		char bios[1024];
		char save[1024];
		char state[1024];
		char patch[1024];
		char palette[1024];
		char cheat[1024];
	} path;

	struct {
		char source[64];
		int hue,saturation;
		char filename[1024];
	} palette;

	struct {

		struct {
			char bios[1024];
			int enabled;
		} gamegenie;

		struct {
			char bios[1024];
			int hle;
		} fds;

		int log_unhandled_io;
		int pause_on_load;
	} nes;

} config_t;

extern config_t *config;*/

int config_init();
void config_kill();
void config_load();
void config_save();

//gets config string variable with variables expanded
char *config_get_eval_string(char *name);

//get config var (wraps the vars_get_*() functions)
char *config_get_string(char *name);
int config_get_int(char *name);
int config_get_bool(char *name);
double config_get_double(char *name);

//set config var (wraps the vars_get_*() functions)
void config_set_string(char *name,char *data);
void config_set_int(char *name,int data);
void config_set_bool(char *name,int data);
void config_set_double(char *name,double data);

//get var
#define var_get_eval_string	config_get_eval_string
#define var_get_string			config_get_string
#define var_get_int				config_get_int
#define var_get_bool				config_get_bool
#define var_get_double			config_get_double

//set var (wraps the vars_get_*() functions)
void var_set_string(char *name,char *data);
void var_set_int(char *name,int data);
void var_set_bool(char *name,int data);
void var_set_double(char *name,double data);

#endif
