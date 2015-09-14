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
#include "system/common/filters.h"
#include "system/common/filters/draw/draw.h"
#include "system/common/filters/interpolate/interpolate.h"
#include "system/common/filters/scale2x/scalebit.h"
#include "system/common/filters/ntsc/ntsc.h"

#define FILTER_START(name,minw,minh,mins)	filter_t filter_ ## name = { #name,minw,minh,mins, {
#define FILTER_MODE(scale,func16,func32)		{scale,func16,func32},
#define FILTER16_MODE(scale,func)				{scale,func,0},
#define FILTER32_MODE(scale,func)				{scale,0,func},
#define FILTER_END()								{0,0}}};

FILTER_START(draw,256,240,1)
	FILTER_MODE(1,draw1x)
	FILTER_MODE(2,draw2x)
	FILTER_MODE(3,draw3x)
	FILTER_MODE(4,draw4x)
	FILTER_MODE(5,draw5x)
	FILTER_MODE(6,draw6x)
FILTER_END()

FILTER_START(interpolate,512,480,2)
	FILTER32_MODE(2,interpolate2x)
	FILTER32_MODE(3,interpolate3x)
	FILTER32_MODE(4,interpolate4x)
FILTER_END()

FILTER_START(scale,512,480,2)
	FILTER32_MODE(2,scale2x)
	FILTER32_MODE(3,scale3x)
	FILTER32_MODE(4,scale4x)
FILTER_END()

FILTER_START(ntsc,602,480,2)
	FILTER32_MODE(2,ntsc2x)
	FILTER32_MODE(3,ntsc3x)
	FILTER32_MODE(4,ntsc4x)
FILTER_END()

int filter_init()
{
	ntsc_init();
	return(0);
}

void filter_kill()
{
	ntsc_kill();
}

void filter_palette_changed()
{
	ntsc_palette_changed();
}

int filter_get_int(char *str)
{
	if(stricmp("interpolate",str) == 0)	return(F_INTERPOLATE);
	if(stricmp("scale",str) == 0)			return(F_SCALE);
	if(stricmp("ntsc",str) == 0)			return(F_NTSC);
	return(F_NONE);
}

filter_t *filter_get(int flt)
{
	filter_t *ret = 0;

	switch(flt) {
		default:
		case F_NONE:			ret = &filter_draw;				break;
		case F_INTERPOLATE:	ret = &filter_interpolate;		break;
		case F_SCALE:			ret = &filter_scale;				break;
		case F_NTSC:			ret = &filter_ntsc;				break;
	}
	return(ret);
}
