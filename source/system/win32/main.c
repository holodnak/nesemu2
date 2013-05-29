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
#include "misc/memutil.h"
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

void checkmessages();

int mainloop()
{
	while(quit == 0) {
		system_checkevents();
		video_startframe();
		if(running && nes->cart) {
			nes_frame();
		}
		video_endframe();
		input_poll();
	}
	return(0);
}

static void console_loghook(char *str)
{
	HWND hCtrl;
	int index;

	if(hConsole) {
		hCtrl = GetDlgItem(hConsole,IDC_CONSOLEEDIT);
		index = GetWindowTextLength(hCtrl);
		Edit_SetSel(hCtrl,index,index);
		Edit_ReplaceSel(hCtrl,str);
	}
}

void resizeclient(HWND hwnd,int w,int h);

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	int ret;
	char *p,*p2,*cmdline;

	MyRegisterClass(hInstance);
	if(InitInstance(hInstance,nCmdShow) == 0) {
		return(FALSE);
	}

	log_sethook(console_loghook);

	//make the exe path variable
	GetModuleFileName(hInstance,exepath,1024);
	if((p = strrchr(exepath,PATH_SEPERATOR)) != 0) {
		*p = 0;
	}

	sprintf(configfilename,"%s%c%s",exepath,PATH_SEPERATOR,CONFIG_FILENAME);

	if(emu_init() != 0)
		return(FALSE);

	resizeclient(hWnd,config->video.scale * 256,config->video.scale * 240);
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

	//this is temporary
	nes_set_inputdev(0,I_JOYPAD0);

	p = cmdline = mem_strdup(lpCmdLine);
	if(*p == '\"') {
		p++;
		if((p2 = strchr(p,'\"')) != 0)
			*p2 = 0;
	}
	log_printf("trying lpcmdline as filename (%s)...\n",p);
	if(nes_load(p) == 0) {
		nes_reset(1);
		running = 1;
	}

	mem_free(cmdline);

	log_printf("starting main loop...\n");
	ret = (mainloop() == 0) ? TRUE : FALSE;

	emu_kill();

	log_printf("done!\n");
	return(ret);
}
