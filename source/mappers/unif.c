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

#include <string.h>
#include "mapperinc.h"

struct unif_board_s {
	const char *name;
	int boardid;
};

#define UNIF_BOARD_START()		static struct unif_board_s boards[] = {
#define UNIF_BOARD_END()		{0,-1}};
#define UNIF_BOARD(s,b)			{s,b},

UNIF_BOARD_START()

	//licensed
	//NROM boards
	UNIF_BOARD("NES-NROM",						B_NROM)
	UNIF_BOARD("NES-NROM-128",					B_NROM)
	UNIF_BOARD("NES-NROM-256",					B_NROM)
	UNIF_BOARD("HVC-NROM-128",					B_NROM)
	UNIF_BOARD("HVC-NROM-256",					B_NROM)
	UNIF_BOARD("IREM-NROM-128",				B_NROM)
	UNIF_BOARD("IREM-NROM-256",				B_NROM)
	UNIF_BOARD("TAITO-NROM-128",				B_NROM)
	UNIF_BOARD("TAITO-NROM-256",				B_NROM)
	UNIF_BOARD("KONAMI-NROM-128",				B_NROM)
	UNIF_BOARD("KONAMI-NROM-256",				B_NROM)
	UNIF_BOARD("JALECO-JF-01",					B_NROM)
	UNIF_BOARD("JALECO-JF-02",					B_NROM)
	UNIF_BOARD("JALECO-JF-03",					B_NROM)
	UNIF_BOARD("JALECO-JF-04",					B_NROM)

	//SxROM boards
	UNIF_BOARD("NES-SAROM",						B_SxROM)
	UNIF_BOARD("NES-SBROM",						B_SxROM)
	UNIF_BOARD("NES-SCROM",						B_SxROM)
	UNIF_BOARD("NES-SEROM",						B_SxROM)
	UNIF_BOARD("NES-SFROM",						B_SxROM)
	UNIF_BOARD("NES-SGROM",						B_SxROM)
	UNIF_BOARD("NES-SHROM",						B_SxROM)
	UNIF_BOARD("NES-SJROM",						B_SxROM)
	UNIF_BOARD("NES-SKROM",						B_SxROM)
	UNIF_BOARD("NES-SLROM",						B_SxROM)
	UNIF_BOARD("NES-SNROM",						B_SxROM)
	UNIF_BOARD("NES-SOROM",						B_SxROM)
	UNIF_BOARD("NES-SUROM",						B_SxROM)
	UNIF_BOARD("NES-SXROM",						B_SxROM)

UNIF_BOARD_END()

int mapper_get_mapperid_unif(char *str)
{
	int i;

	for(i=0;boards[i].boardid != -1;i++) {
		if(stricmp(str,boards[i].name) == 0)
			return(boards[i].boardid);
	}
	return(B_UNSUPPORTED);
}

const char *mapper_get_unif_boardname(int idx)
{
	if(boards[idx].boardid == -1 || boards[idx].name == 0)
		return(0);
	return(boards[idx].name);
}
