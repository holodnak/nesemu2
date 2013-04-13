/***************************************************************************
 *   Copyright (C) 2006-2009 by Dead_Body   *
 *   jamesholodnak@gmail.com   *
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

#ifndef __nes__io_h__
#define __nes__io_h__

#include "types.h"

//nes io handlers
u8 nes_read_4000(u32 addr);
void nes_write_4000(u32 addr,u8 data);

//generic read/write function handlers that use the memory ptrs
u8 nes_read_mem(u32 addr);
void nes_write_mem(u32 addr,u8 data);

//nes rom read handler
u8 nes_read_rom(u32 addr);

#endif
