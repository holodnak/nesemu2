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

#include "system/common/filters.h"
#include "system/common/filters/draw/draw.h"
#include "system/common/filters/interpolate/interpolate.h"
#include "system/common/filters/scale2x/scalebit.h"

#define FILTER_START(name,width,height)	filter_t filter_ ## name = { #name,width,height, {
#define FILTER_MODE(scale,func)				{scale,func},
#define FILTER_END()								{0,0}}};

FILTER_START(draw,256,240)
	FILTER_MODE(1,draw1x)
	FILTER_MODE(2,draw2x)
	FILTER_MODE(3,draw3x)
	FILTER_MODE(4,draw4x)
FILTER_END()

FILTER_START(interpolate,256,240)
	FILTER_MODE(2,interpolate2x)
	FILTER_MODE(3,interpolate3x)
	FILTER_MODE(4,interpolate4x)
FILTER_END()

FILTER_START(scale,256,240)
	FILTER_MODE(2,scale2x)
	FILTER_MODE(3,scale3x)
	FILTER_MODE(4,scale4x)
FILTER_END()

//FILTER_START(ntsc,301,240)
//	FILTER_MODE(2,ntsc2x)
//	FILTER_MODE(3,ntsc3x)
//	FILTER_MODE(4,ntsc4x)
//FILTER_END()
