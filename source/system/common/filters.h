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

#ifndef __filters_h__
#define __filters_h__

#include "types.h"

#define FILTER_DECL(name)	extern filter_t filter_ ##name ;

typedef struct filter_s {

	//name of filter
	char	*name;

	//minimum required output width/height for filter
	int	minwidth,minheight;

	//minimum scale factor
	int	minscale;

	struct mode_s {
		//scale factor
		int	scale;

		//function to draw from source to dest
//		void	(*draw16)(void*,u32,void*,u32,u32,u32);
		void	(*draw32)(void*,u32,void*,u32,u32,u32);
	} modes[8];

} filter_t;

FILTER_DECL(draw);
FILTER_DECL(interpolate);
FILTER_DECL(scale);
FILTER_DECL(ntsc);

#endif
