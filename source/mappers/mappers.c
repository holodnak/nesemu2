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
	//nintendo boards
	MAPPER(B_NROM);
	MAPPER(B_SxROM);
	MAPPER(B_UxROM);
	MAPPER(B_AxROM);
	MAPPER(B_PxROM);
	MAPPER(B_FxROM);
	MAPPER(B_TxROM);

	//tengen
	MAPPER(B_TENGEN_800032);

	//camerica
	MAPPER(B_CAMERICA_BF9093);
	MAPPER(B_CAMERICA_BF9096);
	MAPPER(B_CAMERICA_BF9097);

	return(0);
}

static void null_mapper_tile(u8 t,int b)
{
}

static void null_mapper_cycle()
{
}

static void null_mapper_state(int m,u8 *d)
{
}

mapper_t *mapper_init(int mapperid)
{
	mapper_t *ret = get_mapper(mapperid);

	if(ret == 0)
		return(0);
	ret->tile = (ret->tile == 0) ? null_mapper_tile : ret->tile;
	ret->cycle = (ret->cycle == 0) ? null_mapper_cycle : ret->cycle;
	ret->state = (ret->state == 0) ? null_mapper_state : ret->state;
	return(ret);
}
