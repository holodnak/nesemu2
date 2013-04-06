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

#ifndef __mapperinc_h__
#define __mapperinc_h__

#define MAPPER(boardid,reset,tile,line,state) \
	mapper_t mapper##boardid = {boardid,reset,tile,line,state}

#define CFG_U8(dat)		\
	if(mode == STATE_LOADCFG)	\
		{(dat) = *data++;}	\
	else if(mode == STATE_SAVECFG) \
		{*data++ = (dat);}

#define STATE_U8(dat)		\
	if(mode == STATE_LOAD)	\
		{(dat) = *data++;}	\
	else if(mode == STATE_SAVE) \
		{*data++ = (dat);}

#define STATE_U16(dat)			\
	if(mode == STATE_LOAD) {	\
		(dat) = *data++;			\
		(dat) |= *data++ << 8;	\
	}									\
	else if(mode == STATE_SAVE) {					\
		*data++ = (u8)((dat) & 0xFF);				\
		*data++ = (u8)(((dat) >> 8) & 0xFF);	\
	}

#define STATE_U32(dat)			\
	if(mode == STATE_LOAD) {	\
		(dat) = *data++;			\
		(dat) |= *data++ << 8;	\
		(dat) |= *data++ << 16;	\
		(dat) |= *data++ << 24;	\
	}									\
	else if(mode == STATE_SAVE) {					\
		*data++ = (u8)((dat) & 0xFF);				\
		*data++ = (u8)(((dat) >> 8) & 0xFF);	\
		*data++ = (u8)(((dat) >> 16) & 0xFF);	\
		*data++ = (u8)(((dat) >> 24) & 0xFF);	\
	}

#define STATE_INT(dat)				\
	if(mode == STATE_LOAD) {		\
		u32 _tmpu32;					\
		_tmpu32 = *data++;			\
		_tmpu32 |= *data++ << 8;	\
		_tmpu32 |= *data++ << 16;	\
		_tmpu32 |= *data++ << 24;	\
		dat = (int)_tmpu32;			\
	}										\
	else if(mode == STATE_SAVE) {							\
		*data++ = (u8)((u32)(dat) & 0xFF);				\
		*data++ = (u8)(((u32)(dat) >> 8) & 0xFF);		\
		*data++ = (u8)(((u32)(dat) >> 16) & 0xFF);	\
		*data++ = (u8)(((u32)(dat) >> 24) & 0xFF);	\
	}

#define STATE_ARRAY_U8(arr,siz) { \
	int i; \
	for(i=0;i<(siz);i++)	\
		STATE_U8((arr)[i]); \
	}

#include "mappers.h"
#include "mapperid.h"
#include "nes/nes.h"
#include "nes/memory.h"

#endif
