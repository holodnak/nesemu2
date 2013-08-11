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

#ifndef __filter__ntsc_h__
#define __filter__ntsc_h__

#include "types.h"

int ntsc_init();
void ntsc_kill();
void ntsc_palette_changed();
void ntsc2x(void *dst,u32 dst_slice,void *src,u32 src_slice,u32 width,u32 height);
void ntsc3x(void *dst,u32 dst_slice,void *src,u32 src_slice,u32 width,u32 height);
void ntsc4x(void *dst,u32 dst_slice,void *src,u32 src_slice,u32 width,u32 height);

#endif
