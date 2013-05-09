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
#include "resource.h"
#include "misc/log.h"
#include "misc/config.h"
#include "misc/emu.h"
#include "palette/palette.h"
#include "palette/generator.h"
#include "system/video.h"
#include "system/input.h"
#include "system/win32/mainwnd.h"
#include "nes/nes.h"

static palette_t *pal = 0;

// Global Variables:
int quit = 0;
int running = 0;

void video_resize();

__inline void checkmessages()
{
	MSG msg;

	while(PeekMessage(&msg,hWnd,0,0,PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

int mainloop()
{
//	HACCEL hAccelTable;

//	hAccelTable = LoadAccelerators(hInstance,(LPCTSTR)IDC_CRAP);
	while(quit == 0) {
		checkmessages();
		if(running) {
			nes_frame();
		}
		video_startframe();
		video_endframe();
		input_poll();
	}
	return(0);
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	int ret;

	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	if(emu_init() != 0)
		return(FALSE);

//this palette crap could be made common to all system targets...palette_init() maybe?
	if(strcmp(config_get_string("palette.source","generator"),"file") == 0) {
		pal = palette_load(config_get_string("palette.filename","roni.pal"));
	}
	if(pal == 0) {
		pal = palette_generate(config_get_int("palette.generator.hue",-15),config_get_int("palette.generator.saturation",45));
	}
	video_setpalette(pal);

	log_printf("starting main loop...\n");
	ret = mainloop();

	palette_destroy(pal);

	emu_kill();

	log_printf("done!\n");
	return(ret);
}
