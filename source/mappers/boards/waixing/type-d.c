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
	int i,bank;

	mmc3_syncprg(0xFF,0x00);
	for(i=0;i<8;i++) {
		bank = mmc3_getchrbank(i);
		if(bank < 2) {
			mem_setvram1(i,bank);
			log_printf("waixing/type-d.c:  sync:  mapping vram!\n");
		}
		else
			mem_setchr1(i,bank);
	}
	mmc3_syncmirror();
	mmc3_syncsram();
}

static void reset(int hard)
{
	mem_setvramsize(2);
	mmc3_reset(C_MMC3B,sync,hard);
}

MAPPER(B_WAIXING_TYPE_D,reset,mmc3_ppucycle,0,mmc3_state);
