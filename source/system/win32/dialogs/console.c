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

BOOL CALLBACK ConsoleProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	char *str;
	int len;
	HWND hEdit;

	switch(message) {
		case WM_INITDIALOG:
			hEdit = GetDlgItem(hwnd,IDC_COMMANDEDIT);
			SetFocus(hEdit);
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

	}
	return(FALSE);
}
