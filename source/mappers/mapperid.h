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
	B_NROM = 0,
	B_AxROM,
	B_BxROM,
	B_CNROM,
	B_CPROM,
	B_DxROM,
	B_ExROM,
	B_FxROM,
	B_GxROM_MxROM,
	B_HxROM,
	B_PxROM,
	B_SxROM,
	B_TxROM,
	B_UxROM,
	B_TQROM,
	B_UN1ROM,
	B_UNROM_74HC08,
	B_EVENT,

	//konami
	B_VRC1,
	B_VRC2A,
	B_VRC2B,
	B_VRC3,
	B_VRC4A,
	B_VRC4B,
	B_VRC4C,
	B_VRC4D,
	B_VRC4E,
	B_VRC6A,
	B_VRC6B,
	B_VRC7A,
	B_VRC7B,

	//jaleco
	B_74x139x74,
	B_JALECO_SS88006,

	//namcot
	B_NAMCOT_163,
	B_NAMCOT_3425,
	B_NAMCOT_34x3,
	B_NAMCOT_3446,
	B_NAMCOT_3453,

	//bandai
	B_BANDAI_KARAOKE,
	B_74x161x161x32,		//also taito

	//irem
	B_IREM_H3001,
	B_IREM_TAM_S1,

	//taito
	B_TAITO_X1_005,
	B_TAITO_X1_005A,

	//unlicensed
	B_H2288,
	B_70IN1,
	B_70IN1B,
	B_CALTRON6IN1,
	B_BIOMIRACLEA,
	B_MARIO1_MALEE2,
	B_MAGICSERIES,

	//union bond ffv
	B_UNION_BOND_FFV,

	//racermate
	B_RACERMATE,

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

	//ffe
	B_FFE_MAPPER6,
	B_FFE_MAPPER8,
	B_FFE_MAPPER17,

	//ave
	B_NINA_001,

	//for getting a count of supported boards
	B_BOARDEND,

	//extra support

	B_NSF =		0x10000000,
	B_FDS =		0x10000001,
	B_HLEFDS =	0x10000002,

};

#endif
