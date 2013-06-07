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
#include "misc/memutil.h"
#include "misc/log.h"
#include "emu/commands.h"
#include "system/win32/mainwnd.h"
#include "system/win32/resource.h"

typedef struct line_s {
	struct line_s *prev,*next;
	char *str;
} line_t;

typedef struct history_s {
	line_t *lines;
	line_t *cur;
} history_t;

static history_t history = {0,0};

void history_init()
{
	history.lines = 0;
	history.cur = 0;
}

void history_kill()
{
	line_t *p,*line = history.lines;

	while(line) {
		p = line;
		line = line->next;
		free(p->str);
		free(p);
	}
}

void history_add(char *str)
{
	line_t *p = (line_t*)malloc(sizeof(line_t));

	p->prev = 0;
	p->next = history.lines;
	p->str = strdup(str);
	if(history.lines == 0)
		history.lines = p;
	else
		history.lines->prev = p;
	history.lines = p;
	history.cur = 0;
}

char *history_getcur()
{
	return(history.cur ? history.cur->str : 0);
}

char *history_getnext()
{
	char *ret = 0;

	if(history.cur == 0)
		history.cur = history.lines;
	else {
		if(history.cur->next)
			history.cur = history.cur->next;
	}
	return(history_getcur());
}

char *history_getprev()
{
	char *ret = 0;

	if(history.cur) {
		if(history.cur->prev)
			history.cur = history.cur->prev;
		ret = history.cur->str;
	}
	return(ret);
}

#define PROP_ORIGINAL_PROC		TEXT("_NewEdit_Original_Proc_")
#define PROP_STATIC_NEWEDIT	TEXT("_NewEdit_")

LRESULT CALLBACK NewEditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WNDPROC pfnOrigProc = (WNDPROC)GetProp(hwnd,PROP_ORIGINAL_PROC);
	char tmp[123];

	switch (message) {
		case WM_DESTROY:
			SetWindowLongPtr(hwnd, GWL_WNDPROC, (LONG)(LONG_PTR) pfnOrigProc);
			RemoveProp(hwnd, PROP_ORIGINAL_PROC);
			break;
		case WM_KEYDOWN:
			if(wParam == VK_UP) {
				SetWindowText(hwnd,history_getnext());
				return(TRUE);
			}
			if(wParam == VK_DOWN) {
				SetWindowText(hwnd,history_getprev());
				return(TRUE);
			}
			break;
	}
	return(CallWindowProc(pfnOrigProc,hwnd,message,wParam,lParam));
}

BOOL ConvertEditToNewEdit(HWND hwndCtl)
{
	DWORD dwStyle;
	WNDPROC pfnOrigProc;

	// Make sure the control will send notifications.
	dwStyle = GetWindowLong(hwndCtl, GWL_STYLE);
	SetWindowLong(hwndCtl, GWL_STYLE, dwStyle | SS_NOTIFY);

	// Subclass the existing control.
	pfnOrigProc = (WNDPROC)(LONG_PTR)GetWindowLongPtr(hwndCtl, GWL_WNDPROC);
	SetProp(hwndCtl, PROP_ORIGINAL_PROC, (HANDLE) pfnOrigProc);
	SetWindowLongPtr(hwndCtl, GWL_WNDPROC, (LONG)(LONG_PTR)(WNDPROC)NewEditProc);

	// Set a flag on the control so we know what color it should be.
	SetProp(hwndCtl, PROP_STATIC_NEWEDIT, (HANDLE) 1);

	return(TRUE);
}

//TODO: create history and put it in GWL_USERDATA

LRESULT CALLBACK ConsoleProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	char str2[123];
	char *str;
	int len,tmp;
	HWND hEdit;

	switch(message) {
		case WM_INITDIALOG:
			hEdit = GetDlgItem(hwnd,IDC_COMMANDEDIT);
			ConvertEditToNewEdit(hEdit);
			SetFocus(hEdit);
			history_init();
			return(TRUE);

		case WM_SYSCOMMAND:
			switch(LOWORD(wParam)) {
				case SC_CLOSE:
					SendMessage(hWnd,WM_COMMAND,ID_VIEW_CONSOLE,0);
					return(TRUE);
			}
			break;

		case WM_COMMAND:
		    switch(LOWORD(wParam)) {
				case IDC_EXECUTEBUTTON:
					hEdit = GetDlgItem(hwnd,IDC_COMMANDEDIT);
					if((len = SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0))) {
						str = (char*)mem_alloc(len + 1);
						memset(str,0,len + 1);
						SendMessage(hEdit,WM_GETTEXT,len + 1,(LPARAM)str);
						log_printf("> %s\n",str);
						history_add(str);
						SetWindowText(hEdit,"");
						command_execute(str);
						mem_free(str);
					}
					return(TRUE);
				 case IDOK:
				 case IDCANCEL:
					 return(TRUE);
			 }
			break;

		case WM_DESTROY:
			history_kill();
			break;

	}
	return(FALSE);
}
