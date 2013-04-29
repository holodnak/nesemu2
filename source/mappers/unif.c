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

	//DxROM boards
	UNIF_BOARD("NES-DEROM",						B_DxROM)
	UNIF_BOARD("NES-DE1ROM",					B_DxROM)
	UNIF_BOARD("NES-DRROM",						B_DxROM)

	//irem
	UNIF_BOARD("IREM-TAM-S1",					B_IREM_TAM_S1)

	//taito
	UNIF_BOARD("TAITO-X1-005",					B_TAITO_X1_005)

	//jaleco
	UNIF_BOARD("JALECO-JF-23",					B_JALECO_SS88006)
	UNIF_BOARD("JALECO-JF-24",					B_JALECO_SS88006)
	UNIF_BOARD("JALECO-JF-25",					B_JALECO_SS88006)
	UNIF_BOARD("JALECO-JF-27",					B_JALECO_SS88006)
	UNIF_BOARD("JALECO-JF-29",					B_JALECO_SS88006)
	UNIF_BOARD("JALECO-JF-30",					B_JALECO_SS88006)
	UNIF_BOARD("JALECO-JF-31",					B_JALECO_SS88006)
	UNIF_BOARD("JALECO-JF-32",					B_JALECO_SS88006)
	UNIF_BOARD("JALECO-JF-33",					B_JALECO_SS88006)
	UNIF_BOARD("JALECO-JF-34",					B_JALECO_SS88006)
	UNIF_BOARD("JALECO-JF-35",					B_JALECO_SS88006)
	UNIF_BOARD("JALECO-JF-36",					B_JALECO_SS88006)
	UNIF_BOARD("JALECO-JF-37",					B_JALECO_SS88006)
	UNIF_BOARD("JALECO-JF-38",					B_JALECO_SS88006)
	UNIF_BOARD("JALECO-JF-40",					B_JALECO_SS88006)
	UNIF_BOARD("JALECO-JF-41",					B_JALECO_SS88006)

	//konami vrc boards
	UNIF_BOARD("KONAMI-VRC-1",					B_VRC1)
	UNIF_BOARD("KONAMI-VRC-2",					B_VRC2B)
	UNIF_BOARD("KONAMI-VRC-2A",				B_VRC2A)
	UNIF_BOARD("KONAMI-VRC-2B",				B_VRC2B)
	UNIF_BOARD("KONAMI-VRC-3",					B_VRC3)
	UNIF_BOARD("KONAMI-VRC-4",					B_VRC4B)
	UNIF_BOARD("KONAMI-VRC-4A",				B_VRC4A)
	UNIF_BOARD("KONAMI-VRC-4B",				B_VRC4B)
	UNIF_BOARD("KONAMI-VRC-4C",				B_VRC4C)
	UNIF_BOARD("KONAMI-VRC-4D",				B_VRC4D)
	UNIF_BOARD("KONAMI-VRC-4E",				B_VRC4E)
	UNIF_BOARD("KONAMI-VRC-6",					B_VRC6B)
	UNIF_BOARD("KONAMI-VRC-6A",				B_VRC6A)
	UNIF_BOARD("KONAMI-VRC-6B",				B_VRC6B)
	UNIF_BOARD("KONAMI-VRC-7",					B_VRC7B)
	UNIF_BOARD("KONAMI-VRC-7A",				B_VRC7A)
	UNIF_BOARD("KONAMI-VRC-7B",				B_VRC7B)

	//namcot
	UNIF_BOARD("NAMCOT-3433",					B_NAMCOT_34x3)
	UNIF_BOARD("NAMCOT-3443",					B_NAMCOT_34x3)
	UNIF_BOARD("NAMCOT-3453",					B_NAMCOT_3453)

	//unlicensed
	//camerica
	UNIF_BOARD("CAMERICA-BF9093",				B_CAMERICA_BF9093)
	UNIF_BOARD("CAMERICA-ALGN",				B_CAMERICA_BF9093)
	UNIF_BOARD("CODEMASTERS-NR8N",			B_CAMERICA_BF9093)
	UNIF_BOARD("CAMERICA-BF9096",				B_CAMERICA_BF9096)
	UNIF_BOARD("CAMERICA-BF9097",				B_CAMERICA_BF9097)

	//tengen
	UNIF_BOARD("TENGEN-800002",				B_TENGEN_MIMIC_1)
	UNIF_BOARD("TENGEN-800004",				B_TENGEN_MIMIC_1)
	UNIF_BOARD("TENGEN-800030",				B_TENGEN_MIMIC_1)
	UNIF_BOARD("TENGEN-800032",				B_TENGEN_RAMBO_1)
	UNIF_BOARD("TENGEN-800037",				B_TENGEN_RAMBO_1)

	//txc
	UNIF_BOARD("UNL-22211",						B_TXC_22211A)
	UNIF_BOARD("UNL-22211-A",					B_TXC_22211A)
	UNIF_BOARD("UNL-22211-B",					B_TXC_22211B)
	UNIF_BOARD("UNL-22211-C",					B_TXC_22211C)

	//sachen
	UNIF_BOARD("UNL-SACHEN-74LS374N",		B_SACHEN_74LS374N)
	UNIF_BOARD("UNL-SA-72007",					B_SA_72007)
	UNIF_BOARD("UNL-SA-72008",					B_SA_72008)
	UNIF_BOARD("UNL-SA-0036",					B_SA_0036)
	UNIF_BOARD("UNL-SA-0037",					B_SA_0037)

	//other (unknown)
	UNIF_BOARD("BMC-70IN1",						B_70IN1)
	UNIF_BOARD("BMC-70IN1B",					B_70IN1B)
	UNIF_BOARD("BTL-MARIO1-MALEE2",			B_MARIO1_MALEE2)
	UNIF_BOARD("BTL-BIOMIRACLEA",				B_BIOMIRACLEA)
	UNIF_BOARD("MLT-CALTRON6IN1",				B_CALTRON6IN1)
	UNIF_BOARD("UNL-RACERMATE",				B_RACERMATE)
	UNIF_BOARD("UNL-H2288",						B_H2288)

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
