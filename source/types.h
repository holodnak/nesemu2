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

#ifndef __types_h__
#define __types_h__

#if defined(_MSC_VER)
	#define int64 __int64
	#define INLINE __inline
#elif defined(__GNUC__)
	#define int64 long long
	#define INLINE inline
#else
	#error unknown compiler.  please #define int64.
#endif

typedef signed char		s8;
typedef signed short		s16;
typedef signed int		s32;
typedef signed int64		s64;

typedef unsigned char	u8;
typedef unsigned short	u16;
typedef unsigned int		u32;
typedef unsigned int64	u64;

typedef u8 (*readfunc_t)(u32);
typedef void (*writefunc_t)(u32,u8);

#endif
