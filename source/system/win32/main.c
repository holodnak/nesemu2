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
#include <windowsx.h>
#include "resource.h"
#include "emu/emu.h"
#include "misc/log.h"
#include "misc/config.h"
#include "misc/paths.h"
#include "palette/palette.h"
#include "palette/generator.h"
#include "system/main.h"
#include "system/video.h"
#include "system/input.h"
#include "system/win32/mainwnd.h"
#include "nes/nes.h"

static palette_t *pal = 0;

// Global Variables:
int quit = 0;
int running = 0;
char configfilename[1024] = CONFIG_FILENAME;
char exepath[1024] = "";

void video_resize();

__inline void checkmessages()
{
	MSG msg;

	while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
		if(IsDialogMessage(hConsole,&msg) == FALSE &&
			IsDialogMessage(hDebugger,&msg) == FALSE) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

int mainloop()
{
	while(quit == 0) {
		checkmessages();
		if(running && nes.cart) {
			nes_frame();
		}
		video_startframe();
		video_endframe();
		input_poll();
	}
	return(0);
}

static void console_loghook(char *str)
{
	HWND hCtrl = GetDlgItem(hConsole,IDC_CONSOLEEDIT);
	int index = GetWindowTextLength(hCtrl);

	Edit_SetSel(hCtrl,index,index);
	Edit_ReplaceSel(hCtrl,str);
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	int ret;
	char *p;

	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	log_sethook(console_loghook);

	//make the exe path variable
	GetModuleFileName(hInstance,exepath,1024);
	if((p = strrchr(exepath,PATH_SEPERATOR)) != 0) {
		*p = 0;
	}

	if(emu_init() != 0)
		return(FALSE);

	//this is temporary
	nes_set_inputdev(0,I_JOYPAD0);

//this palette crap could be made common to all system targets...palette_init() maybe?
	if(strcmp(config->palette.source,"file") == 0) {
		pal = palette_load(config->palette.filename);
	}
	if(pal == 0) {
		pal = palette_generate(config->palette.hue,config->palette.saturation);
	}
	video_setpalette(pal);

	log_printf("trying lpcmdline as filename (%s)...\n",lpCmdLine);
	if(nes_load(lpCmdLine) == 0) {
		nes_reset(1);
		running = 1;
	}

	log_printf("starting main loop...\n");
	ret = mainloop();

	palette_destroy(pal);

	emu_kill();

	log_printf("done!\n");
	return(ret);
}
