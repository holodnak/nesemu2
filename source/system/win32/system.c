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
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include "types.h"
#include "system/main.h"
#include "system/win32/mainwnd.h"

int system_init()
{
	return(0);
}

void system_kill()
{
}

void system_checkevents()
{
	MSG msg;

	while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
		if(IsDialogMessage(hConsole,&msg) || IsDialogMessage(hDebugger,&msg))
			continue;
		if(TranslateAccelerator(msg.hwnd,hAccelTable,&msg))
			continue;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

char *system_getcwd()
{
	static char buf[_MAX_PATH];

	if(getcwd(buf,_MAX_PATH) == NULL)
		memset(buf,0,_MAX_PATH);
	return(buf);
}

u64 system_gettick()
{
	LARGE_INTEGER li;

	QueryPerformanceCounter(&li);
	return(li.QuadPart);
}

u64 system_getfrequency()
{
	LARGE_INTEGER li;

	if(QueryPerformanceFrequency(&li) == 0)
		return(1);
	return(li.QuadPart);
}
