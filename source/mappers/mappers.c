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

#ifdef MAPPER
	#undef MAPPER
#endif

#define MAPPER(n) { \
	extern mapper_t mapper##n; \
	if(mapperid == n) \
		return(&mapper##n); \
	}

static mapper_t *get_mapper(int mapperid)
{
	//licensed
	//nintendo boards
	MAPPER(B_NINTENDO_AxROM);
	MAPPER(B_NINTENDO_BxROM);
	MAPPER(B_NINTENDO_CNROM);
	MAPPER(B_NINTENDO_CNROM_CP);
	MAPPER(B_NINTENDO_CPROM);
	MAPPER(B_NINTENDO_DxROM);
	MAPPER(B_NINTENDO_ExROM);
	MAPPER(B_NINTENDO_FxROM);
	MAPPER(B_NINTENDO_GxROM_MxROM);
	MAPPER(B_NINTENDO_HxROM);
	MAPPER(B_NINTENDO_NROM);
	MAPPER(B_NINTENDO_NROM_SRAM);
	MAPPER(B_NINTENDO_PxROM);
	MAPPER(B_NINTENDO_SxROM_MMC1A);
	MAPPER(B_NINTENDO_SxROM_MMC1B);
	MAPPER(B_NINTENDO_SxROM_MMC1C);
	MAPPER(B_NINTENDO_TxROM);
	MAPPER(B_NINTENDO_TQROM);
	MAPPER(B_NINTENDO_UxROM);
	MAPPER(B_NINTENDO_UN1ROM);
	MAPPER(B_NINTENDO_UNROM_74HC08);
	MAPPER(B_NINTENDO_EVENT);
	MAPPER(B_NINTENDO_QJ);
	MAPPER(B_NINTENDO_PAL_ZZ);

	//konami
	MAPPER(B_KONAMI_VRC1);
	MAPPER(B_KONAMI_VRC2A);
	MAPPER(B_KONAMI_VRC2B);
	MAPPER(B_KONAMI_VRC2C);
	MAPPER(B_KONAMI_VRC3);
	MAPPER(B_KONAMI_VRC4A);
	MAPPER(B_KONAMI_VRC4B);
	MAPPER(B_KONAMI_VRC4C);
	MAPPER(B_KONAMI_VRC4D);
	MAPPER(B_KONAMI_VRC4E);
	MAPPER(B_KONAMI_VRC6A);
	MAPPER(B_KONAMI_VRC6B);
	MAPPER(B_KONAMI_VRC7A);
	MAPPER(B_KONAMI_VRC7B);

	//taito
	MAPPER(B_TAITO_X1_017);
	MAPPER(B_TAITO_X1_005);
	MAPPER(B_TAITO_X1_005A);
	MAPPER(B_TAITO_TC0190FMC);

	//irem
	MAPPER(B_IREM_H3001);
	MAPPER(B_IREM_TAM_S1);
	MAPPER(B_IREM_HOLYDIVER);

	//jaleco
	MAPPER(B_JALECO_SS88006);
	MAPPER(B_JALECO_JF11);
	MAPPER(B_JALECO_JF16);

	//namcot
	MAPPER(B_NAMCOT_34x3);
	MAPPER(B_NAMCOT_3453);
	MAPPER(B_NAMCOT_3425);
//	MAPPER(B_NAMCOT_129);
	MAPPER(B_NAMCOT_163);
//	MAPPER(B_NAMCOT_175);
//	MAPPER(B_NAMCOT_340);

	//sunsoft
	MAPPER(B_SUNSOFT_1);
	MAPPER(B_SUNSOFT_2);
	MAPPER(B_SUNSOFT_3);
	MAPPER(B_SUNSOFT_4);
	MAPPER(B_SUNSOFT_5B);

	//unlicensed
	//tengen
	MAPPER(B_TENGEN_RAMBO_1);
	MAPPER(B_TENGEN_MIMIC_1);

	//camerica
	MAPPER(B_CAMERICA_BF9093);
	MAPPER(B_CAMERICA_BF9096);
	MAPPER(B_CAMERICA_BF9097);
	MAPPER(B_CAMERICA_GOLDENFIVE);

	//txc
	MAPPER(B_TXC_22211A);
	MAPPER(B_TXC_22211B);
	MAPPER(B_TXC_22211C);
	MAPPER(B_TXC_TW);
	MAPPER(B_TXC_MXMDHTWO);
	MAPPER(B_TXC_STRIKEWOLF);

	//sachen
	MAPPER(B_SACHEN_TCA01);
	MAPPER(B_SACHEN_TCU01);
	MAPPER(B_SACHEN_74LS374N);
//	MAPPER(B_SACHEN_74LS374NA);
	MAPPER(B_SACHEN_8259A);
	MAPPER(B_SACHEN_8259B);
	MAPPER(B_SACHEN_8259C);
	MAPPER(B_SACHEN_8259D);
	MAPPER(B_SA_72007);
	MAPPER(B_SA_72008);
	MAPPER(B_SA_0036);
	MAPPER(B_SA_0037);
	MAPPER(B_SA_0161M);

	//ave
	MAPPER(B_AVE_NINA_001);
	MAPPER(B_AVE_NINA_006);
	MAPPER(B_AVE_D1012);

	//cne
	MAPPER(B_CNE_FSB);
	MAPPER(B_CNE_SHLZ);
	MAPPER(B_CNE_DECATHLON);

	//kaiser
	MAPPER(B_KAISER_KS202);

	//waixing
	MAPPER(B_WAIXING_SGZ);
	MAPPER(B_WAIXING_FFV);
	MAPPER(B_WAIXING_SH2);
	MAPPER(B_WAIXING_ZS);
	MAPPER(B_WAIXING_TYPE_D);
	MAPPER(B_WAIXING_TYPE_H);

	//ntdec
	MAPPER(B_NTDEC_TC112);
	MAPPER(B_NTDEC_CALTRON6IN1);
	MAPPER(B_NTDEC_ASDER);

	//rexsoft
	MAPPER(B_REXSOFT_DBZ5);
	MAPPER(B_REXSOFT_SL1632);

	//other (bootleg multicarts)
	MAPPER(B_BMC_15IN1);
	MAPPER(B_BMC_36IN1);
	MAPPER(B_BMC_70IN1);
	MAPPER(B_BMC_70IN1B);
	MAPPER(B_BMC_BIG7IN1);
	MAPPER(B_BMC_FK23C);
	MAPPER(B_BMC_CONTRA_FUNCTION_16);
	MAPPER(B_BMC_MARIO7IN1);
	MAPPER(B_BMC_SUPER700IN1);
	MAPPER(B_BMC_76IN1);
	MAPPER(B_BMC_72IN1);
	MAPPER(B_BMC_21IN1);
	MAPPER(B_BMC_35IN1);
	MAPPER(B_BMC_150IN1);
	MAPPER(B_BMC_SUPERHIK4IN1);
	MAPPER(B_BMC_SUPERHIKXIN1);
	MAPPER(B_BMC_BALLGAMES11IN1);
	MAPPER(B_BMC_SUPER42IN1);
	MAPPER(B_BMC_1200IN1);
	MAPPER(B_BMC_20IN1);
	MAPPER(B_BMC_22IN1);
	MAPPER(B_BMC_31IN1);
	MAPPER(B_BMC_RESET4IN1);
	MAPPER(B_BMC_65IN1);
	MAPPER(B_BMC_GOLDENGAME260IN1);
	MAPPER(B_BMC_SUPERGK);

	//other (bootlegs)
	MAPPER(B_BTL_MARIO1_MALEE2);
	MAPPER(B_BTL_BIOMIRACLEA);
	MAPPER(B_BTL_SMB2B);
	MAPPER(B_BTL_PIKACHUY2K);

	//other (unlicensed)
	MAPPER(B_UNL_H2288);
	MAPPER(B_UNL_RACERMATE);
	MAPPER(B_UNL_UY);
	MAPPER(B_UNL_SUPERLIONKING);
	MAPPER(B_UNL_BITCORP);

	//colordreams
	MAPPER(B_COLORDREAMS);
	MAPPER(B_DEATHRACE);

	//others
	MAPPER(B_MAGICSERIES);
	MAPPER(B_RUMBLESTATION);
	MAPPER(B_HES);
	MAPPER(B_ACTIVE);
	MAPPER(B_OPENCORP);
	MAPPER(B_NITRA);
	MAPPER(B_KASING);
	MAPPER(B_NANJING);
	MAPPER(B_HOSENKAN);
	MAPPER(B_JYCOMPANY);
	MAPPER(B_COOLBOY);

	//homebrew
	MAPPER(B_TEPPLES);
	MAPPER(B_DRIPGAME);
	MAPPER(B_MAGICFLOOR);

	//others
	MAPPER(B_FDS);
	MAPPER(B_NSF);

	return(0);
}

static void null_mapper_cycle()				{}
static void null_mapper_state(int m,u8 *d){}

#define check_null(var,nullfunc)	var = ((var == 0) ? nullfunc : var)

mapper_t *mapper_init(int mapperid)
{
	mapper_t *ret = get_mapper(mapperid);

	if(ret == 0) {
		log_printf("mapper_init:  get_mapper() failed!  mapperid = %d\n",mapperid);
		return(0);
	}
	check_null(ret->ppucycle,	null_mapper_cycle);
	check_null(ret->cpucycle,	null_mapper_cycle);
	check_null(ret->state,		null_mapper_state);
	return(ret);
}
