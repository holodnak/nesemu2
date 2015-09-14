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

#include "draw.h"

void draw1x_16(u16 *dest,u32 destp,u16 *src,u32 srcp,u32 w,u32 h)
{
	u32 x,y;

	destp /= 2;
	srcp /= 2;
	for(y=0;y<h;y++) {
		for(x=0;x<w;x++) {
			dest[x] = src[x];
		}
		src += srcp;
		dest += destp;
	}
}

void draw2x_16(u16 *dest,u32 destp,u16 *src,u32 srcp,u32 w,u32 h)
{
	u32 x,y;
	u16 *dest1,*dest2,pixel;

	destp /= 2;
	srcp /= 2;
	for(y=0;y<h;y++) {
		dest1 = dest;
		dest2 = dest + destp;
		for(x=0;x<w;x++) {
			pixel = src[x];
			*dest1++ = pixel;
			*dest1++ = pixel;
			*dest2++ = pixel;
			*dest2++ = pixel;
		}
		src += srcp;
		dest += destp * 2;
	}
}

void draw3x_16(u16 *dest,u32 destp,u16 *src,u32 srcp,u32 w,u32 h)
{
	u32 x,y;
	u16 *dest1,*dest2,*dest3,pixel;

	destp /= 2;
	srcp /= 2;
	for(y=0;y<h;y++) {
		dest1 = dest;
		dest2 = dest + destp;
		dest3 = dest + destp * 2;
		for(x=0;x<w;x++) {
			pixel = src[x];
			*dest1++ = pixel;
			*dest1++ = pixel;
			*dest1++ = pixel;
			*dest2++ = pixel;
			*dest2++ = pixel;
			*dest2++ = pixel;
			*dest3++ = pixel;
			*dest3++ = pixel;
			*dest3++ = pixel;
		}
		src += srcp;
		dest += destp * 3;
	}
}

void draw4x_16(u16 *dest,u32 destp,u16 *src,u32 srcp,u32 w,u32 h)
{
	u32 x,y;
	u16 *dest1,*dest2,*dest3,*dest4,pixel;

	destp /= 2;
	srcp /= 2;
	for(y=0;y<h;y++) {
		dest1 = dest;
		dest2 = dest + destp;
		dest3 = dest + destp * 2;
		dest4 = dest + destp * 3;
		for(x=0;x<w;x++) {
			pixel = src[x];
			*dest1++ = pixel;
			*dest1++ = pixel;
			*dest1++ = pixel;
			*dest1++ = pixel;
			*dest2++ = pixel;
			*dest2++ = pixel;
			*dest2++ = pixel;
			*dest2++ = pixel;
			*dest3++ = pixel;
			*dest3++ = pixel;
			*dest3++ = pixel;
			*dest3++ = pixel;
			*dest4++ = pixel;
			*dest4++ = pixel;
			*dest4++ = pixel;
			*dest4++ = pixel;
		}
		src += srcp;
		dest += destp * 4;
	}
}

void draw1x(u32 *dest,u32 destp,u32 *src,u32 srcp,u32 w,u32 h)
{
	u32 x,y;

	destp /= 4;
	srcp /= 4;
	for(y=0;y<h;y++) {
		for(x=0;x<w;x++) {
			dest[x] = src[x];
		}
		src += srcp;
		dest += destp;
	}
}

void draw2x(u32 *dest,u32 destp,u32 *src,u32 srcp,u32 w,u32 h)
{
	u32 x,y;
	u32 *dest1,*dest2,pixel;

	destp /= 4;
	srcp /= 4;
	for(y=0;y<h;y++) {
		dest1 = dest;
		dest2 = dest + destp;
		for(x=0;x<w;x++) {
			pixel = src[x];
			*dest1++ = pixel;
			*dest1++ = pixel;
			*dest2++ = pixel;
			*dest2++ = pixel;
		}
		src += srcp;
		dest += destp * 2;
	}
}

void draw3x(u32 *dest,u32 destp,u32 *src,u32 srcp,u32 w,u32 h)
{
	u32 x,y;

	destp /= 4;
	srcp /= 4;
	for(y=0;y<h;y++) {
		for(x=0;x<w;x++) {
			u32 pixel = src[x + (y * srcp)];

			dest[x*3+0 + ((y*3+0) * destp)] = pixel;
			dest[x*3+1 + ((y*3+0) * destp)] = pixel;
			dest[x*3+2 + ((y*3+0) * destp)] = pixel;
			dest[x*3+0 + ((y*3+1) * destp)] = pixel;
			dest[x*3+1 + ((y*3+1) * destp)] = pixel;
			dest[x*3+2 + ((y*3+1) * destp)] = pixel;
			dest[x*3+0 + ((y*3+2) * destp)] = pixel;
			dest[x*3+1 + ((y*3+2) * destp)] = pixel;
			dest[x*3+2 + ((y*3+2) * destp)] = pixel;
		}
	}
}

void draw4x(u32 *dest,u32 destp,u32 *src,u32 srcp,u32 w,u32 h)
{
	u32 x,y,z;

	destp /= 4;
	srcp /= 4;
	for(y=0;y<h;y++) {
		for(x=0;x<w;x++) {
			u32 pixel = src[x + (y * srcp)];

			for(z=0;z<4;z++) {
				dest[x*4+0 + ((y*4+z) * destp)] = pixel;
				dest[x*4+1 + ((y*4+z) * destp)] = pixel;
				dest[x*4+2 + ((y*4+z) * destp)] = pixel;
				dest[x*4+3 + ((y*4+z) * destp)] = pixel;
			}
		}
	}
}

void draw5x(u32 *dest,u32 destp,u32 *src,u32 srcp,u32 w,u32 h)
{
	u32 x,y,z;

	destp /= 4;
	srcp /= 4;
	for(y=0;y<h;y++) {
		for(x=0;x<w;x++) {
			u32 pixel = src[x + (y * srcp)];

			for(z=0;z<5;z++) {
				dest[x*4+0 + ((y*4+z) * destp)] = pixel;
				dest[x*4+1 + ((y*4+z) * destp)] = pixel;
				dest[x*4+2 + ((y*4+z) * destp)] = pixel;
				dest[x*4+3 + ((y*4+z) * destp)] = pixel;
				dest[x*4+4 + ((y*4+z) * destp)] = pixel;
			}
		}
	}
}

void draw6x(u32 *dest,u32 destp,u32 *src,u32 srcp,u32 w,u32 h)
{
	u32 x,y,z;

	destp /= 4;
	srcp /= 4;
	for(y=0;y<h;y++) {
		for(x=0;x<w;x++) {
			u32 pixel = src[x + (y * srcp)];

			for(z=0;z<6;z++) {
				dest[x*4+0 + ((y*4+z) * destp)] = pixel;
				dest[x*4+1 + ((y*4+z) * destp)] = pixel;
				dest[x*4+2 + ((y*4+z) * destp)] = pixel;
				dest[x*4+3 + ((y*4+z) * destp)] = pixel;
				dest[x*4+4 + ((y*4+z) * destp)] = pixel;
				dest[x*4+5 + ((y*4+z) * destp)] = pixel;
			}
		}
	}
}
