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
#include <stdio.h>
#include "system/win32/resource.h"
#include "system/win32/mainwnd.h"
#include "nes/nes.h"

typedef struct breakpoint_s {
	struct breakpoint_s *next;
	char name[64];
	u8 enabled;
	u8 type;
	u8	opcode;
	u32 startaddr,endaddr;
} breakpoint_t;

static int pc;
static breakpoint_t *breakpoints = 0;

static void update_disasm(HWND hwnd)
{
	SCROLLINFO si;
	int i,highlight = -1;
	char str[128];
	HWND hctrl = GetDlgItem(hwnd,IDC_DISASMLIST);
	u32 p = pc;

	//clear listbox
	ListBox_ResetContent(hctrl);

	//draw lines
	for(i=0;i<29;i++) {
		memset(str,0,128);
		sprintf(str,"%04X:\t",p);
		if(nes->cpu.pc == p)
			highlight = i;
		p = cpu_disassemble(&str[6],p);
		str[14] = '\t';
		str[18] = '\t';
		ListBox_AddString(hctrl,str);
	}
	if(highlight >= 0)
		ListBox_SetCurSel(hctrl,highlight);
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_POS;
	si.nPos = pc;
	SetScrollInfo(GetDlgItem(hwnd,IDC_DISASMSCROLL),SB_CTL,&si,TRUE);
}

static void update_registers(HWND hwnd)
{
	char str[32];

	sprintf(str,"%04X",nes->cpu.pc);	SetWindowText(GetDlgItem(hwnd,IDC_PCEDIT),str);
	sprintf(str,"%02X",nes->cpu.a);	SetWindowText(GetDlgItem(hwnd,IDC_AEDIT),str);
	sprintf(str,"%02X",nes->cpu.x);	SetWindowText(GetDlgItem(hwnd,IDC_XEDIT),str);
	sprintf(str,"%02X",nes->cpu.y);	SetWindowText(GetDlgItem(hwnd,IDC_YEDIT),str);
	sprintf(str,"%02X",nes->cpu.sp);	SetWindowText(GetDlgItem(hwnd,IDC_SPEDIT),str);
}

static void update_timing(HWND hwnd)
{
	char str[32];

	sprintf(str,"%d",nes->ppu.linecycles);	SetWindowText(GetDlgItem(hwnd,IDC_PIXELSTATIC),str);
	sprintf(str,"%d",nes->ppu.scanline);	SetWindowText(GetDlgItem(hwnd,IDC_LINESTATIC),str);
	sprintf(str,"%d",nes->ppu.frames);		SetWindowText(GetDlgItem(hwnd,IDC_FRAMESTATIC),str);
}

static void update(HWND hwnd)
{
	update_disasm(hwnd);
	update_registers(hwnd);
	update_timing(hwnd);
}

LRESULT CALLBACK BreakpointDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {

		case WM_INITDIALOG:
			return(TRUE);

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
				case IDCANCEL:
					EndDialog(hDlg,LOWORD(wParam));
					return(TRUE);
			}
			break;
	}
	return(FALSE);
}

LRESULT CALLBACK DebuggerDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	SCROLLINFO si;
	breakpoint_t *bp;

	switch(message) {

		case WM_INITDIALOG:
			pc = nes->cpu.pc;
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_PAGE | SIF_RANGE;
			si.nMin = 0;
			si.nMax = 0xFFFF;
			si.nPage = 0x400;
			SetScrollInfo(GetDlgItem(hDlg,IDC_DISASMSCROLL),SB_CTL,&si,TRUE);
			update(hDlg);
			return(TRUE);

		case WM_COMMAND:
			switch(LOWORD(wParam)) {

				//control
				case IDC_STEPBUTTON:
					cpu_execute(1);
					pc = nes->cpu.pc;
					update(hDlg);
					return(TRUE);
				case IDC_RUNBUTTON:
					return(TRUE);
				case IDC_SOFTRESETBUTTON:
					nes_reset(0);
					return(TRUE);
				case IDC_HARDRESETBUTTON:
					nes_reset(1);
					return(TRUE);

				//seeking
				case IDC_SEEKCURRENTPCBUTTON:
					pc = nes->cpu.pc;
					update(hDlg);
					return(TRUE);
				case IDC_SEEKADDRESSBUTTON:
					update(hDlg);
					return(TRUE);

				//breakpoint handling
				case IDC_ADDBPBUTTON:
					bp = DialogBoxParam(hInst,(LPCTSTR)IDD_BREAKPOINT,hWnd,BreakpointDlg,(LPARAM)0);
					return(TRUE);
				case IDOK:
				case IDCANCEL:
					EndDialog(hDlg,LOWORD(wParam));
					return(TRUE);
			}
			break;

		case WM_VSCROLL:
			if((HWND)lParam == GetDlgItem(hDlg,IDC_DISASMSCROLL)) {
				log_printf("scrolling\n");
				si.cbSize = sizeof(SCROLLINFO);
				si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
				GetScrollInfo((HWND)lParam,SB_CTL,&si);
				switch(LOWORD(wParam)) {
					case SB_LINEUP:			pc--;						break;
					case SB_LINEDOWN:			pc++;						break;
					case SB_PAGEUP:			pc -= si.nPage;		break;
					case SB_PAGEDOWN:			pc += si.nPage;		break;
					case SB_THUMBPOSITION:	pc = HIWORD(wParam);	break;
				}
				if(pc < si.nMin)
					pc = si.nMin;
				if(pc > si.nMax)
					pc = si.nMax;
				update_disasm(hDlg);
			}
			break;
	}
	return(FALSE);
}
