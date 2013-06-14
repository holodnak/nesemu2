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

#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include "system/win32/resource.h"
#include "mappers/mappers.h"

static void add_ines_mappers(HWND hListBox,HWND hStatic)
{
	int i,n;
	char str[32];

	for(n=0,i=0;i<256;i++) {
		if(mapper_get_mapperid_ines(i) >= 0) {
			ListBox_AddString(hListBox,itoa(i,str,10));
			n++;
		}
	}
	sprintf(str,"iNES:  %d",n);
	Static_SetText(hStatic,str);
}

static void add_ines20_mappers(HWND hListBox,HWND hStatic)
{
	int i,j,n;
	char str[32];

	for(n=0,i=0;i<(256 * 1);i++) {
		for(j=1;j<16;j++) {
			if(mapper_get_mapperid_ines20(i,j) >= 0) {
				sprintf(str,"%d.%d",i,j);
				ListBox_AddString(hListBox,str);
				n++;
			}
		}
	}
	sprintf(str,"iNES 2.0:  %d",n);
	Static_SetText(hStatic,str);
}

static void add_unif_mappers(HWND hListBox,HWND hStatic)
{
	int n;
	const char *p;
	char str[32];

	for(n=0;;n++) {
		if((p = mapper_get_unif_boardname(n)) == 0)
			break;
		ListBox_AddString(hListBox,p);
	}
	sprintf(str,"UNIF:  %d",n);
	Static_SetText(hStatic,str);
}

LRESULT CALLBACK MappersDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {

		case WM_INITDIALOG:
			add_ines_mappers(GetDlgItem(hDlg,IDC_INESLIST),GetDlgItem(hDlg,IDC_INESSTATIC));
			add_ines20_mappers(GetDlgItem(hDlg,IDC_INES20LIST),GetDlgItem(hDlg,IDC_INES20STATIC));
			add_unif_mappers(GetDlgItem(hDlg,IDC_UNIFLIST),GetDlgItem(hDlg,IDC_UNIFSTATIC));
			return(TRUE);

		case WM_COMMAND:
			if(LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg,LOWORD(wParam));
				return(TRUE);
			}
			break;

	}
	return(FALSE);
}
