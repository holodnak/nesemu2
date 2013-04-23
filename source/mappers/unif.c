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

	//UxROM boards
	UNIF_BOARD("NES-UNROM",						B_UxROM)
	UNIF_BOARD("HVC-UNROM",						B_UxROM)
	UNIF_BOARD("IREM-UNROM",					B_UxROM)
	UNIF_BOARD("KONAMI-UNROM",					B_UxROM)
	UNIF_BOARD("NES-UOROM",						B_UxROM)
	UNIF_BOARD("HVC-UOROM",						B_UxROM)
	UNIF_BOARD("JALECO-JF-15",					B_UxROM)
	UNIF_BOARD("JALECO-JF-18",					B_UxROM)
	UNIF_BOARD("JALECO-JF-39",					B_UxROM)					//verify this

	//TxROM boards
	UNIF_BOARD("NES-TLROM",						B_TxROM)
	UNIF_BOARD("NES-TKROM",						B_TxROM)
	UNIF_BOARD("NES-TFROM",						B_TxROM)
	UNIF_BOARD("NES-TBROM",						B_TxROM)
	UNIF_BOARD("NES-TKEPROM",					B_TxROM)

	//TQROM
	UNIF_BOARD("NES-TQROM",						B_TQROM)

	//UN1ROM
	UNIF_BOARD("HVC-UN1ROM",					B_UN1ROM)

	//CxROM boards
	UNIF_BOARD("NES-CNROM",						B_CNROM)
	UNIF_BOARD("KONAMI-74*139/74",			B_CNROM)					//appears to be CNROM
	UNIF_BOARD("NES-CPROM",						B_CPROM)

	//PxROM boards
	UNIF_BOARD("NES-PNROM",						B_PxROM)
	UNIF_BOARD("NES-PEEOROM",					B_PxROM)
	UNIF_BOARD("HVC-PEEOROM",					B_PxROM)

	//FxROM boards
	UNIF_BOARD("NES-FJROM",						B_FxROM)
	UNIF_BOARD("NES-FKROM",						B_FxROM)

	//taito
	UNIF_BOARD("TAITO-X1-005",					B_TAITO_X1_005)

	//unlicensed
	//camerica
	UNIF_BOARD("CAMERICA-BF9093",				B_CAMERICA_BF9093)
	UNIF_BOARD("CAMERICA-ALGN",				B_CAMERICA_BF9093)
	UNIF_BOARD("CODEMASTERS-NR8N",			B_CAMERICA_BF9093)
	UNIF_BOARD("CAMERICA-BF9096",				B_CAMERICA_BF9096)
	UNIF_BOARD("CAMERICA-BF9097",				B_CAMERICA_BF9097)

	//tengen
	UNIF_BOARD("TENGEN-800032",				B_TENGEN_800032)

	//txc
	UNIF_BOARD("UNL-22211",						B_TXC_22211A)
	UNIF_BOARD("UNL-22211-A",					B_TXC_22211A)
	UNIF_BOARD("UNL-22211-B",					B_TXC_22211B)
	UNIF_BOARD("UNL-22211-C",					B_TXC_22211C)

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
