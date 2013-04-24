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

#ifndef __mmc1_h__
#define __mmc1_h__

#include "types.h"

void mmc1_sync();
void mmc1_init(void (*ssync)());
void mmc1_state(int mode,u8 *spot);
void mmc1_write(u32 addr,u8 value);
void mmc1_syncmirror();
void mmc1_syncprg(int aand,int oor);
void mmc1_syncchr(int aand,int oor);
void mmc1_syncvram(int aand,int oor);
void mmc1_syncsram();
int mmc1_getlowprg();
int mmc1_gethighprg();
int mmc1_getlowchr();
int mmc1_gethighchr();

#endif
