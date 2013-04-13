#ifndef __config_h__
#define __config_h__

#if 0
#include "defines.h"

typedef struct config_s {
	char path_rom[512];			//path to roms (also last directory browsed)
	char path_state[512];		//path to save states
	char path_cheat[512];		//path to disk change states
	char path_shots[512];		//screenshot path
	u32 soundenabled;		//sound enabled
	u32 soundchannels;	//enabled sound channels
	//stick system dependent config variables
	//elsewhere, do not define them here
	u32 filter;				//video filter
	u32 windowed;			//set to 1 if we are in windowed mode
	u32 windowscale;		//window size for windowed mode
	u32 fullscreen;		//fullscreen mode
	u32 shotformat;		//screenshot format
	u32 devices[4];		//player devices
	u32 expdevice;			//expansion device (famicom)
	u32 hue,sat;			//palette hue/saturation

	u32 fdsbios;			//fds bios selection (0=nintendo,1=hle)
	u32 showinfo;			//show info at top while playing game

	//input configuration
	u32 gui_keys[16];		//gui keys
	u32 joy_keys[4][10];		//key config for joypads
	u32 pad_keys[16];		//powerpad buttons

	char recent[10][1024];	//recent files
	u32 recentfreeze;
} config_t;

//these both are actually defined in nesemu.c
extern char path_config[];		//path to configuration file (set by config_defaults() or
extern config_t config;			//some other system dependent reset call)

//system dependent config functions
void config_defaults();			//set configuration defaults
int config_load();
int config_save();
#endif

#endif
