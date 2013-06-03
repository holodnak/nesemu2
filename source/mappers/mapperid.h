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

#ifndef __mapperid_h__
#define __mapperid_h__

enum boardid_e {

	//unsupported board
	B_UNSUPPORTED = -1,

	//licensed
	//nintendo
	B_NINTENDO_NROM = 0,
	B_NINTENDO_AxROM,
	B_NINTENDO_BxROM,
	B_NINTENDO_CNROM,
	B_NINTENDO_CNROM_CP,
	B_NINTENDO_CPROM,
	B_NINTENDO_DxROM,
	B_NINTENDO_ExROM,
	B_NINTENDO_FxROM,
	B_NINTENDO_GxROM_MxROM,
	B_NINTENDO_HxROM,
	B_NINTENDO_PxROM,
	B_NINTENDO_SxROM,
	B_NINTENDO_TxROM,
	B_NINTENDO_UxROM,
	B_NINTENDO_TQROM,
	B_NINTENDO_UN1ROM,
	B_NINTENDO_UNROM_74HC08,
	B_NINTENDO_EVENT,
	B_NINTENDO_QJ,
	B_NINTENDO_NROM_SRAM,

	//konami
	B_KONAMI_VRC1,
	B_KONAMI_VRC2A,
	B_KONAMI_VRC2B,
	B_KONAMI_VRC3,
	B_KONAMI_VRC4A,
	B_KONAMI_VRC4B,
	B_KONAMI_VRC4C,
	B_KONAMI_VRC4D,
	B_KONAMI_VRC4E,
	B_KONAMI_VRC6A,
	B_KONAMI_VRC6B,
	B_KONAMI_VRC7A,
	B_KONAMI_VRC7B,

	//jaleco
	B_JALECO_SS88006,
	B_JALECO_JF11,

	//namcot
	B_NAMCOT_3425,
	B_NAMCOT_34x3,
	B_NAMCOT_3446,
	B_NAMCOT_3453,
	B_NAMCOT_129,
	B_NAMCOT_163,
	B_NAMCOT_175,
	B_NAMCOT_340,

	//bandai
	B_BANDAI_KARAOKE,

	//irem
	B_IREM_H3001,
	B_IREM_TAM_S1,

	//taito
	B_TAITO_X1_017,
	B_TAITO_X1_005,
	B_TAITO_X1_005A,
	B_TAITO_TC0190FMC,
	B_TAITO_TC0190FMC_PAL16R4,

	//sunsoft
	B_SUNSOFT_1,
	B_SUNSOFT_2,
	B_SUNSOFT_3,
	B_SUNSOFT_4,
	B_SUNSOFT_5B,

	//unlicensed
	B_UNL_H2288,
	B_BMC_15IN1,
	B_BMC_36IN1,
	B_BMC_70IN1,
	B_BMC_70IN1B,
	B_BMC_BIG7IN1,
	B_BMC_FK23C,
	B_BMC_CONTRA_FUNCTION_16,
	B_BMC_MARIO7IN1,
	B_BMC_SUPER700IN1,
	B_BMC_76IN1,
	B_BMC_72IN1,
	B_BMC_21IN1,
	B_BMC_35IN1,
	B_BMC_150IN1,
	B_BMC_SUPERHIK4IN1,
	B_BMC_SUPERHIKXIN1,
	B_BMC_BALLGAMES11IN1,
	B_BTL_BIOMIRACLEA,
	B_BTL_MARIO1_MALEE2,
	B_BTL_SMB2B,
	B_MAGICSERIES,
	B_COLORDREAMS,
	B_RUMBLESTATION,
	B_HES,
	B_ACTIVE,

	//c&e
	B_CNE_FSB,
	B_CNE_SHLZ,
	B_CNE_DECATHLON,

	//racermate
	B_UNL_RACERMATE,

	//colordreams + few others
	B_74x377,

	//tengen
	B_TENGEN_MIMIC_1,
	B_TENGEN_RAMBO_1,

	//txc corporation
	B_TXC_22211A,
	B_TXC_22211B,
	B_TXC_22211C,
	B_TXC_MXMDHTWO,
	B_TXC_TW,
	B_TXC_STRIKEWOLF,

	//sachen
	B_SACHEN_TCA01,
	B_SACHEN_74LS374N,
	B_SACHEN_74LS374NA,
	B_SACHEN_8259A,
	B_SACHEN_8259B,
	B_SACHEN_8259C,
	B_SACHEN_8259D,
	B_SA_0036,
	B_SA_0037,
	B_SA_72007,
	B_SA_72008,

	//camerica
	B_CAMERICA_BF9093,
	B_CAMERICA_BF9096,
	B_CAMERICA_BF9097,
	B_CAMERICA_GOLDENFIVE,

	//ffe
	B_FFE_MAPPER6,
	B_FFE_MAPPER8,
	B_FFE_MAPPER17,

	//ave
	B_AVE_NINA_001,
	B_AVE_NINA_006,

	//kaiser
	B_KAISER_KS202,

	//waixing
	B_WAIXING_SGZ,
	B_WAIXING_FFV,
	B_WAIXING_SH2,
	B_WAIXING_TYPE_D,

	//ntdec
	B_NTDEC_TC112,
	B_NTDEC_CALTRON6IN1,

	//rexsoft
	B_REXSOFT_DBZ5,

	//homebrew
	B_TEPPLES,
	B_DRIPGAME,
	B_MAGICFLOOR,

	//for getting a count of supported boards
	B_BOARDEND,

	//extra support
	B_NSF =		0x10000000,
	B_FDS =		0x20000000,
	B_HLEFDS =	0x20000001,
	B_DRPCJR =	0x30000000,
	B_GENIE =	0x40000000,
	B_TYPEMASK =0xF0000000,

};

#endif
