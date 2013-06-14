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

#include <windows.h>
#include "system/input.h"
#include "system/system.h"
#include "misc/config.h"
#include "system/common/SDL_keysym.h"

//these global variables provide information for the device input code
int joyx,joyy;			//x and y coords for paddle/mouse
u8 joyzap;				//zapper trigger
u8 joykeys[370];		//keyboard state
int joyconfig[4][8];	//joypad button configuration

// this will map joystick axises/buttons to unused keyboard buttons
#define FIRSTJOYSTATEKEY (350) // ideally should be SDLK_LAST
u8 joystate[32];	// dpad + 8 buttons is enuff' for me but let's be sure :-)

int input_init()
{
	int i;

//	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);
	for(i=0;i<20;i++) {
		joystate[i] = 0;
	}
	input_update_config();
	return(0);
}

void input_kill()
{
}

void input_poll()
{
/*	Uint8 *keystate = SDL_GetKeyState(NULL);
	int i,x,y;

//	joyx = joyy = 0;

	//need to update mousex/mousey/mousebuttons here

	//now update key/mouse state, the input device logic will
	//decode the key/mouse data into the correct input for the nes
	for(i=0;i<300;i++)
		joykeys[i] = keystate[i];
	joyzap = (SDL_GetMouseState(&x,&y) & 1) << 4;
	
	for (i=0; i < 20; i++)
	{
		joykeys[FIRSTJOYSTATEKEY + i] = joystate[i];
	}*/
	int i;

	for(i=0;i<8;i++) {
		joykeys[joyconfig[0][i]] = (u8)(GetAsyncKeyState(joyconfig[0][i]) >> 8);
		joykeys[joyconfig[1][i]] = (u8)(GetAsyncKeyState(joyconfig[1][i]) >> 8);
	}
}

typedef struct keymapentry_s {

	//sdl key id
	int sdlkey;

	//os-depenedent key id
	int oskey;

} keymapentry_t;

//we are missing some keys!  todo!
static keymapentry_t keymap[] = {
	{SDLK_BACKSPACE,		VK_BACK},
	{SDLK_TAB,				VK_TAB},
	{SDLK_CLEAR,			VK_CLEAR},
	{SDLK_RETURN,			VK_RETURN},
	{SDLK_PAUSE,			VK_PAUSE},
	{SDLK_ESCAPE,			VK_ESCAPE},
	{SDLK_SPACE,			VK_SPACE},

	//alphanumeric
	{SDLK_0,					'0'},
	{SDLK_1,					'1'},
	{SDLK_2,					'2'},
	{SDLK_3,					'3'},
	{SDLK_4,					'4'},
	{SDLK_5,					'5'},
	{SDLK_6,					'6'},
	{SDLK_7,					'7'},
	{SDLK_8,					'8'},
	{SDLK_9,					'9'},
	{SDLK_a,					'A'},
	{SDLK_b,					'B'},
	{SDLK_c,					'C'},
	{SDLK_d,					'D'},
	{SDLK_e,					'E'},
	{SDLK_f,					'F'},
	{SDLK_g,					'G'},
	{SDLK_h,					'H'},
	{SDLK_i,					'I'},
	{SDLK_j,					'J'},
	{SDLK_k,					'K'},
	{SDLK_l,					'L'},
	{SDLK_m,					'M'},
	{SDLK_n,					'N'},
	{SDLK_o,					'O'},
	{SDLK_p,					'P'},
	{SDLK_q,					'Q'},
	{SDLK_r,					'R'},
	{SDLK_s,					'S'},
	{SDLK_t,					'T'},
	{SDLK_u,					'U'},
	{SDLK_v,					'V'},
	{SDLK_w,					'W'},
	{SDLK_x,					'X'},
	{SDLK_y,					'Y'},
	{SDLK_z,					'Z'},

	//function keys
	{SDLK_F1,				VK_F1},
	{SDLK_F2,				VK_F2},
	{SDLK_F3,				VK_F3},
	{SDLK_F4,				VK_F4},
	{SDLK_F5,				VK_F5},
	{SDLK_F6,				VK_F6},
	{SDLK_F7,				VK_F7},
	{SDLK_F8,				VK_F8},
	{SDLK_F9,				VK_F9},
	{SDLK_F10,				VK_F10},
	{SDLK_F11,				VK_F11},
	{SDLK_F12,				VK_F12},
	{SDLK_F13,				VK_F13},
	{SDLK_F14,				VK_F14},
	{SDLK_F15,				VK_F15},

	//middle keys
	{SDLK_DELETE,			VK_DELETE},
	{SDLK_INSERT,			VK_INSERT},
	{SDLK_END,				VK_END},
	{SDLK_HOME,				VK_HOME},
	{SDLK_PAGEDOWN,		VK_NEXT},
	{SDLK_PAGEUP,			VK_PRIOR},
	{SDLK_UP,				VK_UP},
	{SDLK_DOWN,				VK_DOWN},
	{SDLK_LEFT,				VK_LEFT},
	{SDLK_RIGHT,			VK_RIGHT},

	//numpad
	{SDLK_KP0,				VK_NUMPAD0},
	{SDLK_KP1,				VK_NUMPAD1},
	{SDLK_KP2,				VK_NUMPAD2},
	{SDLK_KP3,				VK_NUMPAD3},
	{SDLK_KP4,				VK_NUMPAD4},
	{SDLK_KP5,				VK_NUMPAD5},
	{SDLK_KP6,				VK_NUMPAD6},
	{SDLK_KP7,				VK_NUMPAD7},
	{SDLK_KP8,				VK_NUMPAD8},
	{SDLK_KP9,				VK_NUMPAD9},
	{SDLK_KP_PERIOD,		VK_DECIMAL},
	{SDLK_KP_DIVIDE,		VK_DIVIDE},
	{SDLK_KP_MULTIPLY,	VK_MULTIPLY},
	{SDLK_KP_MINUS,		VK_SUBTRACT},
	{SDLK_KP_PLUS,			VK_ADD},
//	{SDLK_KP_ENTER,		VK_DIVIDE},
//	{SDLK_KP_EQUALS,		VK_DIVIDE},

	{-1,-1}
};

//we store sdl key id in the config file, translate to win32
static int translatekey(int key)
{
	int i;

	for(i=0;keymap[i].sdlkey;i++) {
		if(keymap[i].sdlkey == key)
			return(keymap[i].oskey);
	}
	log_printf("translatekey:  key %d is unmapped\n",key);
	return(key);
}

void input_update_config()
{
	int i;

	//get the keys from the configuration
	joyconfig[0][0] = config_get_int("input.joypad0.a");
	joyconfig[0][1] = config_get_int("input.joypad0.b");
	joyconfig[0][2] = config_get_int("input.joypad0.select");
	joyconfig[0][3] = config_get_int("input.joypad0.start");
	joyconfig[0][4] = config_get_int("input.joypad0.up");
	joyconfig[0][5] = config_get_int("input.joypad0.down");
	joyconfig[0][6] = config_get_int("input.joypad0.left");
	joyconfig[0][7] = config_get_int("input.joypad0.right");
	joyconfig[1][0] = config_get_int("input.joypad1.a");
	joyconfig[1][1] = config_get_int("input.joypad1.b");
	joyconfig[1][2] = config_get_int("input.joypad1.select");
	joyconfig[1][3] = config_get_int("input.joypad1.start");
	joyconfig[1][4] = config_get_int("input.joypad1.up");
	joyconfig[1][5] = config_get_int("input.joypad1.down");
	joyconfig[1][6] = config_get_int("input.joypad1.left");
	joyconfig[1][7] = config_get_int("input.joypad1.right");

	//translate keys into something windows understands
	for(i=0;i<8;i++) {
		joyconfig[0][i] = translatekey(joyconfig[0][i]);
		joyconfig[1][i] = translatekey(joyconfig[1][i]);
	}
}
