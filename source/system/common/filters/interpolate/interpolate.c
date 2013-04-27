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

#include "interpolate.h"

#define INTERPOLATE(p1,p2)		((((p1) >> 1) & 0x7F7F7F7F) + (((p2) >> 1) & 0x7F7F7F7F))

void interpolate2x(void *void_dst,u32 dst_slice,void *void_src,u32 src_slice,u32 width,u32 height)
{
	u32 *dest = (u32*)void_dst;
	u32 *src = (u32*)void_src;
	u32 *p;
	u32 x,y;

	dst_slice /= 4;
	src_slice /= 4;
	for(y=0;y<height;y++) {
		p = dest + dst_slice;
		for(x=0;x<width;x++) {
			dest[x*2+0] = src[x];
			dest[x*2+1] = INTERPOLATE(src[x],src[x+1]);
			p[x*2+0] = INTERPOLATE(src[x],src[x+src_slice]);
			p[x*2+1] = INTERPOLATE(src[x+1],src[x+src_slice]);
		}
		src += src_slice;
		dest += dst_slice * 2;
	}
}

//TODO: this one doesnt look quite right
void interpolate3x(void *void_dst,u32 dst_slice,void *void_src,u32 src_slice,u32 width,u32 height)
{
	u32 *dest = (u32*)void_dst;
	u32 *src = (u32*)void_src;
	u32 *p1,*p2,*p3;
	u32 x,y;

	dst_slice /= 4;
	src_slice /= 4;
	for(y=1;y<height;y++) {
		p1 = dest;
		p2 = dest + dst_slice;
		p3 = dest + dst_slice * 2;
		for(x=0;x<width;x++) {
			p1[x*3+0] = INTERPOLATE(src[x],src[x-src_slice]);
			p1[x*3+1] = INTERPOLATE(src[x],src[x-src_slice]);
			p1[x*3+2] = INTERPOLATE(src[x],src[x-src_slice]);

			p2[x*3+0] = INTERPOLATE(src[x],src[x-1]);
			p2[x*3+1] = src[x];
			p2[x*3+2] = INTERPOLATE(src[x],src[x+1]);

			p3[x*3+0] = INTERPOLATE(src[x],src[x+src_slice]);
			p3[x*3+1] = INTERPOLATE(src[x],src[x+src_slice]);
			p3[x*3+2] = INTERPOLATE(src[x],src[x+src_slice]);
		}
		src += src_slice;
		dest += dst_slice * 3;
	}
}

void interpolate4x(void *dst,u32 dst_slice,void *src,u32 src_slice,u32 width,u32 height)
{
}
