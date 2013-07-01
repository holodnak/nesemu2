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
#include "misc/slre/slre.h"

struct unif_board_s {
	const char *name;
	int boardid;
};

#define UNIF_BOARD_START()		static struct unif_board_s boards[] = {
#define UNIF_BOARD_END()		{0,-1}};
#define UNIF_BOARD(s,b)			{s,b},

UNIF_BOARD_START()

	//licensed
	//nintendo boards
	UNIF_BOARD(".+?(-)A[MNO](|1)ROM",						B_NINTENDO_AxROM)
	UNIF_BOARD(".+?(-)CNROM",									B_NINTENDO_CNROM)
	UNIF_BOARD(".+?(-)CPROM",									B_NINTENDO_CPROM)
	UNIF_BOARD(".+?(-)D[ER](|1)ROM",							B_NINTENDO_DxROM)
	UNIF_BOARD(".+?(-)E[KLTW]ROM",							B_NINTENDO_ExROM)
	UNIF_BOARD(".+?(-)F[JK]ROM",								B_NINTENDO_FxROM)
	UNIF_BOARD(".+?(-)(GN|MH)ROM",							B_NINTENDO_GxROM_MxROM)
	UNIF_BOARD(".+?(-)HKROM",									B_NINTENDO_HxROM)
	UNIF_BOARD(".+?(-)(N|R)ROM(|-.*?)",						B_NINTENDO_NROM)
	UNIF_BOARD(".+?(-)P(EEO|N)ROM",							B_NINTENDO_PxROM)
	UNIF_BOARD(".+?(-)S[ABCEFGHIJKLNOU](|W)(|1|EP)ROM",B_NINTENDO_SxROM)
	UNIF_BOARD(".+?(-)T[^Q](|1|EP)ROM",						B_NINTENDO_TxROM)
	UNIF_BOARD(".+?(-)TQ(|EP)ROM",							B_NINTENDO_TQROM)
	UNIF_BOARD(".+?(-)U[NO](|EP)ROM",						B_NINTENDO_UxROM)
	UNIF_BOARD(".+?(-)UN1ROM",									B_NINTENDO_UN1ROM)
	UNIF_BOARD("NES-EVENT",										B_NINTENDO_EVENT)
	UNIF_BOARD("NES-QJ",											B_NINTENDO_QJ)

	//jaleco
	UNIF_BOARD("JALECO-JF-0[1234]",							B_NINTENDO_NROM)
	UNIF_BOARD("JALECO-JF-11",									B_JALECO_JF11)
	UNIF_BOARD("JALECO-JF-(15|18|39)",						B_NINTENDO_UxROM)			//verify JALECO-JF-39
	UNIF_BOARD("JALECO-JF-16",									B_JALECO_JF16)
	UNIF_BOARD("JALECO-JF-2[34579]",							B_JALECO_SS88006)
	UNIF_BOARD("JALECO-JF-3[012345678]",					B_JALECO_SS88006)
	UNIF_BOARD("JALECO-JF-4[01]",								B_JALECO_SS88006)

	//konami boards
	UNIF_BOARD("KONAMI-VRC-1",									B_KONAMI_VRC1)
	UNIF_BOARD("KONAMI-VRC-2",									B_KONAMI_VRC2)
	UNIF_BOARD("KONAMI-VRC-2A",								B_KONAMI_VRC2A)
	UNIF_BOARD("KONAMI-VRC-2B",								B_KONAMI_VRC2B)
	UNIF_BOARD("KONAMI-VRC-3",									B_KONAMI_VRC3)
	UNIF_BOARD("KONAMI-VRC-4",									B_KONAMI_VRC4B)
	UNIF_BOARD("KONAMI-VRC-4A",								B_KONAMI_VRC4A)
	UNIF_BOARD("KONAMI-VRC-4B",								B_KONAMI_VRC4B)
	UNIF_BOARD("KONAMI-VRC-4C",								B_KONAMI_VRC4C)
	UNIF_BOARD("KONAMI-VRC-4D",								B_KONAMI_VRC4D)
	UNIF_BOARD("KONAMI-VRC-4E",								B_KONAMI_VRC4E)
	UNIF_BOARD("KONAMI-VRC-6",									B_KONAMI_VRC6B)
	UNIF_BOARD("KONAMI-VRC-6A",								B_KONAMI_VRC6A)
	UNIF_BOARD("KONAMI-VRC-6B",								B_KONAMI_VRC6B)
	UNIF_BOARD("KONAMI-VRC-7",									B_KONAMI_VRC7B)
	UNIF_BOARD("KONAMI-VRC-7A",								B_KONAMI_VRC7A)
	UNIF_BOARD("KONAMI-VRC-7B",								B_KONAMI_VRC7B)
	UNIF_BOARD("KONAMI-74*139/74",							B_NINTENDO_CNROM)			//appears to be CNROM

	//namcot
	UNIF_BOARD("NAMCOT-3414",									B_NAMCOT_34x3)
	UNIF_BOARD("NAMCOT-34[345]3",								B_NAMCOT_34x3)
	UNIF_BOARD("NAMCOT-3425",									B_NAMCOT_3425)

	//sunsoft
	UNIF_BOARD("SUNSOFT-1",										B_SUNSOFT_1)
	UNIF_BOARD("SUNSOFT-2",										B_SUNSOFT_2)
	UNIF_BOARD("SUNSOFT-3",										B_SUNSOFT_3)
	UNIF_BOARD("SUNSOFT-4",										B_SUNSOFT_4)
	UNIF_BOARD("SUNSOFT-5[AB]",								B_SUNSOFT_5B)

	//irem
	UNIF_BOARD("IREM-TAM-S1",									B_IREM_TAM_S1)
	UNIF_BOARD("IREM-HOLYDIVER",								B_IREM_HOLYDIVER)
	UNIF_BOARD("IREM-H3001",									B_IREM_H3001)

	//taito
	UNIF_BOARD("TAITO-X1-005",									B_TAITO_X1_005)
	UNIF_BOARD("TAITO-X1-017",									B_TAITO_X1_017)

	//unlicensed
	//camerica
	UNIF_BOARD("CAMERICA-BF9093",								B_CAMERICA_BF9093)
	UNIF_BOARD("CAMERICA-ALGN",								B_CAMERICA_BF9093)
	UNIF_BOARD("CODEMASTERS-NR8N",							B_CAMERICA_BF9093)
	UNIF_BOARD("CAMERICA-BF9096",								B_CAMERICA_BF9096)
	UNIF_BOARD("CAMERICA-BF9097",								B_CAMERICA_BF9097)

	//tengen
	UNIF_BOARD("TENGEN-8000(02|04|30)",						B_TENGEN_MIMIC_1)
	UNIF_BOARD("TENGEN-800003",								B_NINTENDO_NROM)
	UNIF_BOARD("TENGEN-80003[27]",							B_TENGEN_RAMBO_1)
	UNIF_BOARD("TENGEN-800042",								B_SUNSOFT_4)				//if it works, let it

	//txc
	UNIF_BOARD("UNL-22211(|-A)",								B_TXC_22211A)
	UNIF_BOARD("UNL-22211-B",									B_TXC_22211B)
	UNIF_BOARD("UNL-22211-C",									B_TXC_22211C)

	//ave
	UNIF_BOARD("AVE-NINA-01",									B_AVE_NINA_001)
	UNIF_BOARD("AVE-NINA-0[36]",								B_AVE_NINA_006)

	//sachen
	UNIF_BOARD("UNL-SACHEN-74LS374N",						B_SACHEN_74LS374N)
	UNIF_BOARD("UNL-SACHEN-8259",								B_SACHEN_8259A)
	UNIF_BOARD("UNL-SACHEN-8259A",							B_SACHEN_8259A)
	UNIF_BOARD("UNL-SACHEN-8259B",							B_SACHEN_8259B)
	UNIF_BOARD("UNL-SACHEN-8259C",							B_SACHEN_8259C)
	UNIF_BOARD("UNL-SACHEN-8259D",							B_SACHEN_8259D)
	UNIF_BOARD("UNL-SA-72007",									B_SA_72007)
	UNIF_BOARD("UNL-SA-72008",									B_SA_72008)
	UNIF_BOARD("UNL-SA-0036",									B_SA_0036)
	UNIF_BOARD("UNL-SA-0037",									B_SA_0037)
	UNIF_BOARD("UNL-SA-016-1M",								B_SA_0161M)

	//bootleg multicarts
	UNIF_BOARD("BMC-70IN1",										B_BMC_70IN1)
	UNIF_BOARD("BMC-70IN1B",									B_BMC_70IN1B)
	UNIF_BOARD("BMC-SUPER700IN1",								B_BMC_SUPER700IN1)
	UNIF_BOARD("BMC-SUPER42IN1",								B_BMC_76IN1)
	UNIF_BOARD("BMC-FK23C",										B_BMC_FK23C)
	UNIF_BOARD("BMC-260IN1",									B_BMC_GOLDENGAME260IN1)

	//bootlegs
	UNIF_BOARD("BTL-MARIO1-MALEE2",							B_BTL_MARIO1_MALEE2)
	UNIF_BOARD("BTL-BIOMIRACLEA",								B_BTL_BIOMIRACLEA)

	//multicarts
	UNIF_BOARD("MLT-CALTRON6IN1",								B_NTDEC_CALTRON6IN1)
	UNIF_BOARD("MLT-ACTION52",									B_ACTIVE)

	//other
	UNIF_BOARD("UNL-RACERMATE",								B_UNL_RACERMATE)
	UNIF_BOARD("UNL-H2288",										B_UNL_H2288)
	UNIF_BOARD("UNL-KASING",									B_KASING)
	UNIF_BOARD("UNL-SUPERLIONKING",							B_UNL_SUPERLIONKING)

	//homebrew
	UNIF_BOARD("UNL-DRIPGAME",									B_DRIPGAME)

	//unofficial, need verification or real board name
	UNIF_BOARD("BMC-65IN1",										B_BMC_65IN1)

UNIF_BOARD_END()

int mapper_get_mapperid_unif(char *str)
{
	int ret,i;
	const char *err;
	char tmpstr[4][256];

	ret = B_UNSUPPORTED;
	for(i=0;boards[i].boardid != -1;i++) {
		err = slre_match(SLRE_CASE_INSENSITIVE,boards[i].name,str,strlen(str),
			SLRE_STRING,sizeof(tmpstr[0]),tmpstr[0],
			SLRE_STRING,sizeof(tmpstr[1]),tmpstr[1],
			SLRE_STRING,sizeof(tmpstr[2]),tmpstr[2],
			SLRE_STRING,sizeof(tmpstr[3]),tmpstr[3]);
		if(err == NULL) {
			if(ret != B_UNSUPPORTED)
				log_printf("mapper_get_mapperid_unif:  duplicate match for board '%s'\n",str);
			ret = boards[i].boardid;
		}
		else if(stricmp(err,"no match") != 0)
			log_printf("mapper_get_mapperid_unif:  slre error:  %s ('%s' -- '%s')\n",err,boards[i].name,str);
	}
	return(ret);
}

const char *mapper_get_unif_boardname(int idx)
{
	if(boards[idx].boardid == -1 || boards[idx].name == 0)
		return(0);
	return(boards[idx].name);
}
