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
	MAPPER(B_AxROM);
	MAPPER(B_BxROM);
	MAPPER(B_CNROM);
	MAPPER(B_CPROM);
	MAPPER(B_DxROM);
	MAPPER(B_ExROM);
	MAPPER(B_FxROM);
	MAPPER(B_GxROM_MxROM);
	MAPPER(B_HxROM);
	MAPPER(B_NROM);
	MAPPER(B_PxROM);
	MAPPER(B_SxROM);
	MAPPER(B_TxROM);
	MAPPER(B_TQROM);
	MAPPER(B_UxROM);
	MAPPER(B_UN1ROM);
	MAPPER(B_UNROM_74HC08);
	MAPPER(B_EVENT);
	MAPPER(B_FDS);

	//konami
	MAPPER(B_VRC1);
	MAPPER(B_VRC2A);
	MAPPER(B_VRC2B);
	MAPPER(B_VRC3);
	MAPPER(B_VRC4A);
	MAPPER(B_VRC4B);
	MAPPER(B_VRC4C);
	MAPPER(B_VRC4D);
	MAPPER(B_VRC4E);
	MAPPER(B_VRC6A);
	MAPPER(B_VRC6B);
	MAPPER(B_VRC7A);
	MAPPER(B_VRC7B);

	//taito
	MAPPER(B_TAITO_X1_005);
	MAPPER(B_TAITO_X1_005A);

	//irem
	MAPPER(B_IREM_H3001);
	MAPPER(B_IREM_TAM_S1);

	//jaleco
	MAPPER(B_JALECO_SS88006);

	//namcot
	MAPPER(B_NAMCOT_34x3);
	MAPPER(B_NAMCOT_3453);
	MAPPER(B_NAMCOT_3425);

	//unlicensed
	//tengen
	MAPPER(B_TENGEN_RAMBO_1);

	//camerica
	MAPPER(B_CAMERICA_BF9093);
	MAPPER(B_CAMERICA_BF9096);
	MAPPER(B_CAMERICA_BF9097);

	//txc
	MAPPER(B_TXC_22211A);
	MAPPER(B_TXC_22211B);
	MAPPER(B_TXC_22211C);
	MAPPER(B_TXC_TW);
	MAPPER(B_TXC_MXMDHTWO);
	MAPPER(B_TXC_STRIKEWOLF);

	//sachen
	MAPPER(B_SACHEN_74LS374N);
//	MAPPER(B_SACHEN_74LS374NA);
	MAPPER(B_SA_72007);
	MAPPER(B_SA_72008);
	MAPPER(B_SA_0036);
	MAPPER(B_SA_0037);

	//ave
	MAPPER(B_NINA_001);

	//cne
	MAPPER(B_CNE_FSB);
	MAPPER(B_CNE_SHLZ);
	MAPPER(B_CNE_DECATHLON);

	//other (unknown)
	MAPPER(B_MARIO1_MALEE2);
	MAPPER(B_UNION_BOND_FFV);
	MAPPER(B_RACERMATE);
	MAPPER(B_BIOMIRACLEA);
	MAPPER(B_CALTRON6IN1);
	MAPPER(B_MAGICSERIES);
	MAPPER(B_70IN1);
	MAPPER(B_70IN1B);

	return(0);
}

static void null_mapper_tile(int t)			{}
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
	check_null(ret->tile,		null_mapper_tile);
	check_null(ret->ppucycle,	null_mapper_cycle);
	check_null(ret->cpucycle,	null_mapper_cycle);
	check_null(ret->state,		null_mapper_state);
	return(ret);
}
