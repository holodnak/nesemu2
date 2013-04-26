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
	INES_BOARD(3,		B_CNROM)
	INES_BOARD(4,		B_TxROM)
	INES_BOARD(7,		B_AxROM)
	INES_BOARD(9,		B_PxROM)
	INES_BOARD(10,		B_FxROM)
	INES_BOARD(13,		B_CPROM)
	INES_BOARD(18,		B_JALECO_SS88006)
	INES_BOARD(21,		B_VRC4A)
	INES_BOARD(22,		B_VRC2A)
	INES_BOARD(23,		B_VRC2B)
	INES_BOARD(24,		B_VRC6A)
	INES_BOARD(25,		B_VRC4B)
	INES_BOARD(26,		B_VRC6B)
	INES_BOARD(34,		B_BxROM)
	INES_BOARD(64,		B_TENGEN_800032)
	INES_BOARD(65,		B_IREM_H3001)
	INES_BOARD(66,		B_GxROM_MxROM)
	INES_BOARD(71,		B_CAMERICA_BF9093)
	INES_BOARD(73,		B_VRC3)
	INES_BOARD(75,		B_VRC1)
	INES_BOARD(80,		B_TAITO_X1_005)
	INES_BOARD(85,		B_VRC7A)
	INES_BOARD(88,		B_DxROM)
	INES_BOARD(94,		B_UN1ROM)
	INES_BOARD(105,	B_EVENT)
	INES_BOARD(119,	B_TQROM)
	INES_BOARD(132,	B_TXC_22211A)
	INES_BOARD(133,	B_SA_72008)		//??
	INES_BOARD(145,	B_SA_72007)
	INES_BOARD(148,	B_SA_0037)
	INES_BOARD(149,	B_SA_0036)
//	INES_BOARD(150,	B_SACHEN_74LS374N)
//	INES_BOARD(150,	B_SA_72008)
	INES_BOARD(172,	B_TXC_22211B)
	INES_BOARD(173,	B_TXC_22211C)
	INES_BOARD(189,	B_TXC_TW)
	INES_BOARD(206,	B_DxROM)
	INES_BOARD(207,	B_TAITO_X1_005A)
	INES_BOARD(232,	B_CAMERICA_BF9096)
	INES_BOARD(241,	B_TXC_MXMDHTWO)
	INES_BOARD(243,	B_SACHEN_74LS374N)
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
