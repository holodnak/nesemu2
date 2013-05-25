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
#include "misc/config.h"

LRESULT CALLBACK GeneralProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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

LRESULT CALLBACK PathsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg,IDC_DATAPATHEDIT,config->path.data);
		SetDlgItemText(hDlg,IDC_BIOSPATHEDIT,config->path.bios);
		SetDlgItemText(hDlg,IDC_SRAMPATHEDIT,config->path.save);
		SetDlgItemText(hDlg,IDC_STATEPATHEDIT,config->path.state);
		SetDlgItemText(hDlg,IDC_PATCHPATHEDIT,config->path.patch);
		SetDlgItemText(hDlg,IDC_PALETTEPATHEDIT,config->path.palette);
		SetDlgItemText(hDlg,IDC_CHEATPATHEDIT,config->path.cheat);
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

LRESULT CALLBACK NesProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		CheckDlgButton(hDlg,IDC_PAUSEAFTERLOADCHECK,config->nes.pause_on_load ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_LOGUNHANDLEDIOCHECK,config->nes.log_unhandled_io ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_FDSHLECHECK,config->nes.fds.hle ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemText(hDlg,IDC_FDSBIOSEDIT,config->nes.fds.bios);
		CheckDlgButton(hDlg,IDC_GENIECHECK,config->nes.gamegenie.enabled ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemText(hDlg,IDC_GENIEBIOSEDIT,config->nes.gamegenie.bios);
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
	PROPSHEETPAGE psp[3];
	PROPSHEETHEADER psh;
	int i;

	memset(psp,0,sizeof(PROPSHEETPAGE) * 3);
	memset(&psh,0,sizeof(PROPSHEETHEADER));

	for(i=0;i<3;i++) {
		psp[i].dwSize = sizeof(PROPSHEETPAGE);
		psp[i].hInstance = hInst;
	}
	psp[0].pszTemplate = MAKEINTRESOURCE(IDD_CONFIG_GENERAL1);
	psp[0].pfnDlgProc = GeneralProc;

	psp[1].pszTemplate = MAKEINTRESOURCE(IDD_CONFIG_GENERAL2);
	psp[1].pfnDlgProc = PathsProc;

	psp[2].pszTemplate = MAKEINTRESOURCE(IDD_CONFIG_GENERAL3);
	psp[2].pfnDlgProc = NesProc;

	psh.dwSize = sizeof(PROPSHEETHEADER);
	psh.dwFlags = PSH_USEICONID | PSH_PROPSHEETPAGE;
	psh.hwndParent = hWnd;
	psh.hInstance = hInst;
	psh.pszCaption = (LPSTR) "Configuration";
	psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
	psh.ppsp = (LPCPROPSHEETPAGE)&psp;

	PropertySheet(&psh);
}
