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

#include "mapperinc.h"

struct ines_boardid_s {
	int num;
	int boardid;
};

#define INES_BOARD_START()		static struct ines_boardid_s boards[] = {
#define INES_BOARD_END()		{0,-1}};
#define INES_BOARD(n,b)			{n,b},

INES_BOARD_START()
	INES_BOARD(0,		B_NROM)
	INES_BOARD(1,		B_SxROM)
	INES_BOARD(2,		B_UxROM)
	INES_BOARD(4,		B_TxROM)
	INES_BOARD(7,		B_AxROM)
	INES_BOARD(9,		B_PxROM)
	INES_BOARD(10,		B_FxROM)
	INES_BOARD(64,		B_TENGEN_800032)
	INES_BOARD(71,		B_CAMERICA_BF9093)
	INES_BOARD(232,	B_CAMERICA_BF9096)
INES_BOARD_END()

int mapper_get_mapperid_ines(int num)
{
	int i;

	for(i=0;boards[i].boardid != -1;i++) {
		if(num == boards[i].num)
			return(boards[i].boardid);
	}
	return(B_UNSUPPORTED);
}
