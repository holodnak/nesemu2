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

#ifndef __mmc3_h__
#define __mmc3_h__

#include "types.h"

#define C_MMC3A	0
#define C_MMC3B	1
#define C_MMC3C	2
#define C_MMC6		3

void mmc3_sync();
u8 mmc3_getprgbank(int n);
u8 mmc3_getchrbank(int n);
u8 mmc3_getchrreg(int n);
u8 mmc3_getcommand();
void mmc3_syncprg(int a,int o);
void mmc3_syncchr(int a,int o);
void mmc3_syncvram(int a,int o);
void mmc3_syncsram();
void mmc3_syncmirror();
void mmc3_write(u32 addr,u8 data);
void mmc3_reset(int t,void (*s)(),int hard);
void mmc3_ppucycle();
void mmc3_state(int mode,u8 *data);

#endif
