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

#ifndef __cart_nsf_h__
#define __cart_nsf_h__

typedef struct nsf_s {
	char	ident[5];
	u8		version;
	u8		totalsongs;
	u8		startsong;
	u16	loadaddr;
	u16	initaddr;
	u16	playaddr;
	char	name[32];
	char	artist[32];
	char	copyright[32];
	u16	ntscspeed;
	u8		bankswitch[8];
	u16	palspeed;
	u8		ntscpal;
	u8		chips;
	u8		expansion[4];
} nsf_t;

int cart_load_nsf(cart_t *ret,const char *filename);

#endif
