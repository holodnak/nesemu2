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
#include <stdio.h>
#include "system/win32/resource.h"
#include "version.h"

//based on code by Neal Stublen
#define PROP_ORIGINAL_FONT		TEXT("_Hyperlink_Original_Font_")
#define PROP_ORIGINAL_PROC		TEXT("_Hyperlink_Original_Proc_")
#define PROP_STATIC_HYPERLINK	TEXT("_Hyperlink_From_Static_")
#define PROP_UNDERLINE_FONT	TEXT("_Hyperlink_Underline_Font_")

LRESULT CALLBACK HyperlinkParentProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	WNDPROC pfnOrigProc = (WNDPROC)GetProp(hwnd,PROP_ORIGINAL_PROC);
	HDC hdc;
	HWND hctrl;
	LRESULT ret;

	switch(message) {
		case WM_CTLCOLORSTATIC:
			hdc = (HDC)wParam;
			hctrl = (HWND)lParam;
			if(GetProp(hctrl,PROP_STATIC_HYPERLINK) != NULL) {
				ret = CallWindowProc(pfnOrigProc,hwnd,message,wParam,lParam);
				SetTextColor(hdc,RGB(0,0,192));
				return(ret);
			}
			break;
		case WM_DESTROY:
			SetWindowLongPtr(hwnd,GWL_WNDPROC,(LONG)(LONG_PTR)pfnOrigProc);
			RemoveProp(hwnd,PROP_ORIGINAL_PROC);
			break;
	}
	return(CallWindowProc(pfnOrigProc,hwnd,message,wParam,lParam));
}

LRESULT CALLBACK HyperlinkProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WNDPROC pfnOrigProc = (WNDPROC)GetProp(hwnd,PROP_ORIGINAL_PROC);
	HFONT hOrigFont,hFont;
	RECT rect;
	POINT pt;
	HCURSOR hCursor;

	switch(message) {
		case WM_DESTROY:
			SetWindowLongPtr(hwnd,GWL_WNDPROC,(LONG)(LONG_PTR)pfnOrigProc);
			RemoveProp(hwnd,PROP_ORIGINAL_PROC);
			hOrigFont = (HFONT) GetProp(hwnd,PROP_ORIGINAL_FONT);
			SendMessage(hwnd,WM_SETFONT,(WPARAM)hOrigFont,0);
			RemoveProp(hwnd,PROP_ORIGINAL_FONT);
			hFont = (HFONT)GetProp(hwnd,PROP_UNDERLINE_FONT);
			DeleteObject(hFont);
			RemoveProp(hwnd,PROP_UNDERLINE_FONT);
			RemoveProp(hwnd,PROP_STATIC_HYPERLINK);
			break;
		case WM_MOUSEMOVE:
			if(GetCapture() != hwnd) {
				hFont = (HFONT) GetProp(hwnd,PROP_UNDERLINE_FONT);
				SendMessage(hwnd, WM_SETFONT,(WPARAM)hFont,FALSE);
				InvalidateRect(hwnd,NULL,FALSE);
				SetCapture(hwnd);
			}
			else {
				GetWindowRect(hwnd,&rect);
				pt.x = LOWORD(lParam);
				pt.y = HIWORD(lParam);
				ClientToScreen(hwnd,&pt);
				if(PtInRect(&rect,pt) == FALSE) {
					hFont = (HFONT)GetProp(hwnd,PROP_ORIGINAL_FONT);
					SendMessage(hwnd,WM_SETFONT,(WPARAM)hFont,FALSE);
					InvalidateRect(hwnd,NULL,FALSE);
					ReleaseCapture();
				}
			}
			break;
		case WM_SETCURSOR:
			hCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND));
			if(hCursor == NULL) {
				hCursor = LoadCursor(NULL,MAKEINTRESOURCE(IDC_ARROW));
			}
			SetCursor(hCursor);
			return(TRUE);
	}
	return(CallWindowProc(pfnOrigProc,hwnd,message,wParam,lParam));
}

BOOL ConvertStaticToHyperlink(HWND hctrl)
{
	WNDPROC pfnOrigProc;
	HWND hwndParent;
	DWORD dwStyle;
	HFONT hOrigFont,hFont;
	LOGFONT lf;

	if((hwndParent = GetParent(hctrl)) != NULL) {
		pfnOrigProc = (WNDPROC)(LONG_PTR)GetWindowLongPtr(hwndParent,GWL_WNDPROC);
		if(pfnOrigProc != HyperlinkParentProc) {
			SetProp(hwndParent,PROP_ORIGINAL_PROC,(HANDLE)pfnOrigProc);
			SetWindowLongPtr(hwndParent,GWL_WNDPROC,(LONG)(LONG_PTR)(WNDPROC)HyperlinkParentProc);
		}
	}

	// Make sure the control will send notifications.
	dwStyle = GetWindowLong(hctrl,GWL_STYLE);
	SetWindowLong(hctrl,GWL_STYLE,dwStyle | SS_NOTIFY);

	// Subclass the existing control.
	pfnOrigProc = (WNDPROC)(LONG_PTR)GetWindowLongPtr(hctrl, GWL_WNDPROC);
	SetProp(hctrl, PROP_ORIGINAL_PROC,(HANDLE)pfnOrigProc);
	SetWindowLongPtr(hctrl,GWL_WNDPROC,(LONG)(LONG_PTR)(WNDPROC)HyperlinkProc);

	// Create an updated font by adding an underline.
	hOrigFont = (HFONT) SendMessage(hctrl,WM_GETFONT,0,0);
	SetProp(hctrl,PROP_ORIGINAL_FONT,(HANDLE)hOrigFont);
	GetObject(hOrigFont,sizeof(lf),&lf);
	lf.lfUnderline = TRUE;
	hFont = CreateFontIndirect(&lf);
	SetProp(hctrl,PROP_UNDERLINE_FONT,(HANDLE)hFont);
	SetProp(hctrl,PROP_STATIC_HYPERLINK,(HANDLE)1);
	return(TRUE);
}

LRESULT CALLBACK AboutDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char tmp[128];

	switch(message) {

		case WM_INITDIALOG:
			sprintf(tmp,"nesemu2 v%s",VERSION);
			SetWindowText(GetDlgItem(hDlg,IDC_TITLESTATIC),tmp);
			ConvertStaticToHyperlink(GetDlgItem(hDlg,IDC_URLSTATIC));
			return(TRUE);

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_URLSTATIC:
					GetWindowText(GetDlgItem(hDlg,IDC_URLSTATIC),tmp,128);
					ShellExecute(hDlg,"open",tmp,NULL,NULL,SW_SHOWNORMAL);
					return(TRUE);
				case IDOK:
				case IDCANCEL:
					EndDialog(hDlg,LOWORD(wParam));
					return(TRUE);
			}
			break;

	}
	return(FALSE);
}
