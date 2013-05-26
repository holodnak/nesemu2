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
#include "misc/log.h"
#include "misc/config.h"
#include "misc/paths.h"
#include "nes/nes.h"
#include "nes/state/state.h"
#include "system/win32/dialogs.h"
#include "system/win32/resource.h"
#include "system/video.h"

#define MAX_LOADSTRING 100

HINSTANCE hInst;									//current instance
HACCEL hAccelTable;								//accelerators
HWND hWnd;											//main window
HWND hConsole = 0;								//console/debug message window
HWND hDebugger = 0;								//debugger window
CHAR szTitle[MAX_LOADSTRING];					//title bar text
CHAR szWindowClass[MAX_LOADSTRING];			//the main window class name

//defined in main.c
extern int quit,running;

//defined in video.cpp
void video_resize();

void resizeclient(HWND hwnd,int w,int h)
{
	RECT rc,rw;

	GetWindowRect(hwnd,&rw);
	GetClientRect(hwnd,&rc);
	SetWindowPos(hwnd,0,0,0,((rw.right - rw.left) - rc.right) + w,((rw.bottom - rw.top) - rc.bottom) + h,SWP_NOZORDER | SWP_NOMOVE);
}

static int filedialog(HWND parent,int type,char *buffer,char *title,char *filter,char *curdir)
{
	OPENFILENAME dlgdata;

	dlgdata.lStructSize = sizeof(OPENFILENAME);
	dlgdata.hwndOwner = parent;
	dlgdata.hInstance = GetModuleHandle(0);
	dlgdata.lpstrFilter = filter;
	dlgdata.lpstrCustomFilter = 0;
	dlgdata.nMaxCustFilter = 0;
	dlgdata.nFilterIndex = 0;
	dlgdata.lpstrFile = buffer;
	dlgdata.nMaxFile = 512;
	dlgdata.lpstrFileTitle = 0;
	dlgdata.nMaxFileTitle = 0;
	dlgdata.lpstrInitialDir = curdir;
	dlgdata.lpstrTitle = title;
	dlgdata.Flags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY;
	dlgdata.nFileOffset = 0;
	dlgdata.nFileExtension = 0;
	dlgdata.lpstrDefExt = 0;
	dlgdata.lCustData = 0;
	dlgdata.lpfnHook = 0;
	dlgdata.lpTemplateName = 0;
	switch(type) {
		default:
		case 0: //open
			if(GetOpenFileName(&dlgdata) != 0)
				return(0);
			break;
		case 1: //save
			if(GetSaveFileName(&dlgdata) != 0)
				return(0);
			break;
	}
	return(1);
}

static void file_open(HWND hWnd)
{
	char buffer[1024];
	static char filter[] =
		"NES ROMs (*.nes, *.unf, *.unif *.fds)\0*.nes;*.unf;*.unif;*.fds\0"
		"iNES ROMs (*.nes)\0*.nes\0"
		"UNIF ROMs (*.unf, *.unif)\0*.unf;*.unif\0";
		"FDS Images (*.fds)\0*.fds\0";

	memset(buffer,0,1024);
	if(filedialog(hWnd,0,buffer,"Open NES ROM...",filter,0) != 0)
		return;
	log_printf("WndProc:  loading file '%s'\n",buffer);
	if(nes_load(buffer) == 0) {
		log_printf("WndProc:  resetting nes...\n");
		nes_reset(1);
		running = config->nes.pause_on_load ? 0 : 1;
	}
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static char dest[1024];
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	HMENU hMenu;

	switch (message) 
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		// Parse the menu selections:
		switch (wmId)
		{
		case ID_FILE_OPEN:
			file_open(hWnd);
			break;
		case ID_NES_SOFTRESET:
			if(nes->cart)
				nes_reset(0);
			break;
		case ID_NES_HARDRESET:
			if(nes->cart)
				nes_reset(1);
			break;
		case ID_NES_LOADSTATE:
			if(nes->cart) {
				paths_makestatefilename(nes->romfilename,dest,1024);
				nes_loadstate(dest);
			}
			break;
		case ID_NES_SAVESTATE:
			if(nes->cart) {
				paths_makestatefilename(nes->romfilename,dest,1024);
				nes_savestate(dest);
			}
			break;
		case ID_FDS_FLIPDISK:
			if(nes->cart) {
				u8 data[4] = {0,0,0,0};

				nes->mapper->state(CFG_SAVE,data);
				if(data[0] == 0xFF)
					data[0] = 0;
				else
					data[0] ^= 1;
				nes->mapper->state(CFG_LOAD,data);
				log_printf("disk inserted!  side = %d\n",data[0]);
			}
			break;
		case ID_VIEW_CONFIGURATION:
			ConfigurationPropertySheet(hWnd);
			break;
		case ID_VIEW_DEBUGGER:
			DialogBox(hInst,(LPCTSTR)IDD_DEBUGGER,hWnd,(DLGPROC)DebuggerDlg);
			break;
		case ID_VIEW_CONSOLE:
			if(hConsole == 0) {
				hConsole = CreateDialog(hInst,MAKEINTRESOURCE(IDD_CONSOLE),hWnd,ConsoleProc);
				ShowWindow(hConsole,SW_SHOW);
			}
			else {
				DestroyWindow(hConsole);
				hConsole = 0;
			}
			hMenu = GetMenu(hWnd);
			CheckMenuItem(hMenu,ID_VIEW_CONSOLE,(hConsole != 0) ? MF_CHECKED : MF_UNCHECKED);
			break;
		case IDM_ABOUT:
			DialogBox(hInst,(LPCTSTR)IDD_ABOUT,hWnd,(DLGPROC)AboutDlg);
			break;
		case ID_HELP_SUPPORTEDMAPPERS:
			DialogBox(hInst,(LPCTSTR)IDD_MAPPERS,hWnd,(DLGPROC)MappersDlg);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case ID_VIEW_FULLSCREEN:
			video_kill();
			config->video.fullscreen ^= 1;
			video_init();
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		quit++;
		if(hConsole)
			DestroyWindow(hConsole);
		PostQuitMessage(0);
		break;
	case WM_SIZE:
		video_resize();
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MAIN, szWindowClass, MAX_LOADSTRING);

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_MAIN);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCTSTR)IDC_MAIN;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_MAIN);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable
	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
	hAccelTable = LoadAccelerators(hInstance,MAKEINTRESOURCE(IDR_ACCELERATOR));
	return((hWnd == 0) ? FALSE : TRUE);
}

void checkmessages()
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
