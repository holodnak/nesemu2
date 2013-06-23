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
#include "system/system.h"
#include "system/video.h"
#include "system/input.h"
#include "system/win32/mainwnd.h"
#include "nes/nes.h"

static palette_t *pal = 0;

// Global Variables:
char configfilename[1024] = CONFIG_FILENAME;
char exepath[1024] = "";

void video_resize();

void checkmessages();

static void console_addline(char *str)
{
	HWND hCtrl;
	int index;

	hCtrl = GetDlgItem(hConsole,IDC_CONSOLEEDIT);
	index = GetWindowTextLength(hCtrl);
	Edit_SetSel(hCtrl,index,index);
	Edit_ReplaceSel(hCtrl,str);
}

static void console_loghook(char *str)
{
	if(hConsole) {
		char newline[3] = "\r\n";
		char *tmp = mem_strdup(str);
		char *p,*p2 = tmp;

		//break up all lines
		for(p=tmp;*p;p++) {
			if(*p == '\n') {
				*p = 0;
				console_addline(p2);
				console_addline(newline);
				p2 = p + 1;
			}
		}

		//output text after the last newline
		if(*p2) {
			console_addline(p2);
		}

		//free the temp string
		mem_free(tmp);
	}
}

void loadrom(char *filename);
void resizeclient(HWND hwnd,int w,int h);

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	int ret;
	char *p,*p2,*cmdline;

	memset(configfilename,0,1024);
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

	if(emu_init() != 0)
		return(FALSE);

	resizeclient(hWnd,config_get_int("video.scale") * 256,config_get_int("video.scale") * 240);
   ShowWindow(hWnd,nCmdShow);
   UpdateWindow(hWnd);

	p = cmdline = mem_strdup(lpCmdLine);
	if(*p == '\"') {
		p++;
		if((p2 = strchr(p,'\"')) != 0)
			*p2 = 0;
	}

	if(strcmp(p,"") != 0) {
		char *filename;

		//it is a relative path, make full path name
		if(p[1] != ':') {
			char *cwd = system_getcwd();
			int len = strlen(p) + strlen(cwd) + 2;

			filename = (char*)mem_alloc(len);
			sprintf(filename,"%s\\%s",cwd,p);
		}

		//already full path name
		else {
			filename = mem_strdup(p);
		}

		log_printf("trying lpcmdline as filename (%s)...\n",filename);
		loadrom(filename);
		mem_free(filename);
	}

	mem_free(cmdline);

	ret = (emu_mainloop() == 0) ? TRUE : FALSE;

	emu_kill();

	return(emu_exit(ret));
}
