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

struct ines20_boardid_s {
	int num,sub;
	int boardid;
};

#define INES20_BOARD_START()	static struct ines20_boardid_s boards[] = {
#define INES20_BOARD_END()		{0,0,-1}};
#define INES20_BOARD(n,s,b)	{n,s,b},

INES20_BOARD_START()
	INES20_BOARD(0,	1,		B_NINTENDO_NROM_SRAM)			//nrom +	save ram
	INES20_BOARD(4,	1,		B_NINTENDO_HxROM)
	INES20_BOARD(21,	9,		B_KONAMI_VRC4A)
	INES20_BOARD(21,	14,	B_KONAMI_VRC4C)
	INES20_BOARD(23,	10,	B_KONAMI_VRC4E)
	INES20_BOARD(25,	1,		B_KONAMI_VRC4B)
	INES20_BOARD(25,	3,		B_KONAMI_VRC4D)
	INES20_BOARD(34,	1,		B_AVE_NINA_001)
//	INES20_BOARD(34,	2,		B_UNION_BOND)
	INES20_BOARD(71,	1,		B_CAMERICA_BF9097)
	INES20_BOARD(78,	3,		B_IREM_HOLYDIVER)
INES20_BOARD_END()

int mapper_get_mapperid_ines20(int num,int sub)
{
	int i;
	
	//no submapper, just use the ines function
	if(sub == 0) {
		return(mapper_get_mapperid_ines(num));
	}
	for(i=0;boards[i].boardid != -1;i++) {
		if((num == boards[i].num) && (sub == boards[i].sub))
			return(boards[i].boardid);
	}
	return(B_UNSUPPORTED);
}
