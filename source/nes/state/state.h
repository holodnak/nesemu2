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

#ifndef __state_h__
#define __state_h__

#include <stdio.h>
#include "nes/state/block.h"

#define STATE_LOAD		0	//load state
#define STATE_SAVE		1	//save state
#define STATE_SIZE		2	//get state size
#define CFG_LOAD			4	//load config
#define CFG_SAVE			5	//save config
#define CFG_SIZE			6	//get config size

#define CFG_U8(dat)				\
	if(mode == CFG_LOAD)			\
		{(dat) = *data++;}		\
	else if(mode == CFG_SAVE)	\
		{*data++ = (dat);}

#define STATE_U8(dat)			\
	if(mode == STATE_LOAD)		\
		{(dat) = *data++;}		\
	else if(mode == STATE_SAVE)	\
		{*data++ = (dat);}			\
	else if(mode == STATE_SIZE)	\
		{*((u32*)data) += 1;}

#define STATE_U16(dat)			\
	if(mode == STATE_LOAD) {		\
		(dat) = *data++;					\
		(dat) |= *data++ << 8;				\
	}													\
	else if(mode == STATE_SAVE) {					\
		*data++ = (u8)((dat) & 0xFF);				\
		*data++ = (u8)(((dat) >> 8) & 0xFF);	\
	}														\
	else if(mode == STATE_SIZE)					\
		{*((u32*)data) += 2;}

#define STATE_U32(dat)			\
	if(mode == STATE_LOAD) {	\
		(dat) = *data++;			\
		(dat) |= *data++ << 8;		\
		(dat) |= *data++ << 16;			\
		(dat) |= *data++ << 24;				\
	}													\
	else if(mode == STATE_SAVE) {					\
		*data++ = (u8)((dat) & 0xFF);				\
		*data++ = (u8)(((dat) >> 8) & 0xFF);	\
		*data++ = (u8)(((dat) >> 16) & 0xFF);	\
		*data++ = (u8)(((dat) >> 24) & 0xFF);	\
	}														\
	else if(mode == STATE_SIZE)					\
		{*((u32*)data) += 4;}

#define STATE_U64(dat)			\
	if(mode == STATE_LOAD) {	\
		(dat) = (u64)*data++;			\
		(dat) |= (u64)*data++ << 8;	\
		(dat) |= (u64)*data++ << 16;	\
		(dat) |= (u64)*data++ << 24;	\
		(dat) |= (u64)*data++ << 32;	\
		(dat) |= (u64)*data++ << 40;		\
		(dat) |= (u64)*data++ << 48;			\
		(dat) |= (u64)*data++ << 56;				\
	}													\
	else if(mode == STATE_SAVE) {					\
		*data++ = (u8)((dat) & 0xFF);				\
		*data++ = (u8)(((dat) >> 8) & 0xFF);	\
		*data++ = (u8)(((dat) >> 16) & 0xFF);	\
		*data++ = (u8)(((dat) >> 24) & 0xFF);	\
		*data++ = (u8)(((dat) >> 32) & 0xFF);	\
		*data++ = (u8)(((dat) >> 40) & 0xFF);	\
		*data++ = (u8)(((dat) >> 48) & 0xFF);	\
		*data++ = (u8)(((dat) >> 56) & 0xFF);	\
	}														\
	else if(mode == STATE_SIZE)					\
		{*((u32*)data) += 8;}

#define STATE_INT(dat)				\
	if(mode == STATE_LOAD) {		\
		u32 _tmpu32;					\
		_tmpu32 = *data++;			\
		_tmpu32 |= *data++ << 8;	\
		_tmpu32 |= *data++ << 16;	\
		_tmpu32 |= *data++ << 24;	\
		dat = (int)_tmpu32;			\
	}										\
	else if(mode == STATE_SAVE) {				\
		*data++ = (u8)(u32)(dat);				\
		*data++ = (u8)((u32)(dat) >> 8);	\
		*data++ = (u8)((u32)(dat) >> 16);	\
		*data++ = (u8)((u32)(dat) >> 24);	\
	}														\
	else if(mode == STATE_SIZE)					\
		*((u32*)data) += 4;

#define STATE_ARRAY_U8(arr,siz) { \
	u32 i; \
	for(i=0;i<(siz);i++)	\
		STATE_U8((arr)[i]); \
	}

#define STATE_ARRAY_U16(arr,siz) { \
	u32 i; \
	for(i=0;i<(siz);i++)	\
		STATE_U16((arr)[i]); \
	}

//state block types
enum blocktype_e {
	B_UNKNOWN	= 0,
	B_NES			= MAKEID('N', 'E', 'S', '\0'),
	B_CPU			= MAKEID('C', 'P', 'U', '\0'),
	B_PPU			= MAKEID('P', 'P', 'U', '\0'),
	B_APU			= MAKEID('A', 'P', 'U', '\0'),
	B_MAPR		= MAKEID('M', 'A', 'P', 'R' ),
	B_VRAM		= MAKEID('V', 'R', 'A', 'M' ),
	B_SVRAM		= MAKEID('B', 'R', 'A', 'M' ),
	B_WRAM		= MAKEID('W', 'R', 'A', 'M' ),
	B_SRAM		= MAKEID('S', 'R', 'A', 'M' ),
	B_DISK		= MAKEID('D', 'I', 'S', 'K' ),
	B_GG			= MAKEID('G', 'G', '\0','\0'),
};

//state header as stored on disk
typedef struct stateheader_s {
	u32	ident;			//state ident
	u16	version;			//state version
	u16	flags;			//flags
	u32	usize,csize;	//uncompressed/compressed size
	u32	crc32;			//crc32 of uncompressed data
} stateheader_t;

typedef void (*statefunc_t)(int,u8*);

int state_init();
void state_kill();
void state_register(u32 type,statefunc_t func);
void state_unregister(u32 type);
statefunc_t state_getfunc(u32 type);
int state_load(memfile_t *file);
int state_save(memfile_t *file);

#endif
