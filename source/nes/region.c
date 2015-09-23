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

#include "nes/nes.h"
#include "misc/log.h"

region_t region_ntsc = {
	REGION_NTSC,
	60,
	236250000 / 11,
	241,261
};

region_t region_pal = {
	REGION_PAL,
	50,
	26601712,
	241,311
};

region_t region_dendy = {
	REGION_DENDY,
	50,
	26601712,
	291,311
};

void nes_set_region(int r)
{
	switch(r) {
		default:
			log_printf("nes_set_region:  invalid region, defaulting to ntsc.\n");
			r = REGION_NTSC;
		case REGION_NTSC:
			nes->region = &region_ntsc;
			break;
		case REGION_PAL:
			nes->region = &region_pal;
			break;
		case REGION_DENDY:
			nes->region = &region_dendy;
			break;
	}
	apu_set_region(r);
}
