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

#include "mappers/mapperinc.h"
#include "mappers/chips/mmc3.h"

static void sync()
{
	int i,j;

	mmc3_syncprg(0x3F,0);
	for(i=0;i<8;i++) {
		j = mmc3_getchrbank(i);
		if(j & 0x40)
			mem_setvram1(i,j & 7);
		else
			mem_setchr1(i,j & 0x3F);
	}
	mmc3_syncsram();
	mmc3_syncmirror();
}

static void reset(int hard)
{
	mem_setvramsize(8);
	mmc3_reset(C_MMC3A,sync,hard);
}

MAPPER(B_TQROM,reset,mmc3_ppucycle,0,mmc3_state);
