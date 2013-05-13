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

#include <stdlib.h>
#include <windows.h>
#include <prsht.h>
#include "system/win32/resource.h"
#include "system/win32/mainwnd.h"

LRESULT CALLBACK GeneralProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		CheckDlgButton(hDlg,IDC_PAUSEAFTERLOADCHECK,BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_LOGUNHANDLEDIOCHECK,BST_CHECKED);
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

LRESULT CALLBACK PathsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

VOID ConfigurationPropertySheet(HWND hWnd)
{
    PROPSHEETPAGE psp[2];
    PROPSHEETHEADER psh;

	 memset(psp,0,sizeof(PROPSHEETPAGE) * 2);
	 memset(&psh,0,sizeof(PROPSHEETHEADER));

	 psp[0].dwSize = sizeof(PROPSHEETPAGE);
    psp[0].hInstance = hInst;
    psp[0].pszTemplate = MAKEINTRESOURCE(IDD_CONFIG_GENERAL1);
    psp[0].pfnDlgProc = GeneralProc;

	 psp[1].dwSize = sizeof(PROPSHEETPAGE);
    psp[1].hInstance = hInst;
    psp[1].pszTemplate = MAKEINTRESOURCE(IDD_CONFIG_GENERAL2);
    psp[1].pfnDlgProc = PathsProc;

	 psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_USEICONID | PSH_PROPSHEETPAGE;
    psh.hwndParent = hWnd;
    psh.hInstance = hInst;
    psh.pszCaption = (LPSTR) "Configuration";
    psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.ppsp = (LPCPROPSHEETPAGE)&psp;

	 PropertySheet(&psh);
}
