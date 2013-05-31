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

#ifndef __namcot_163_h__
#define __namcot_163_h__

#include "types.h"

#define NAMCOT_HAS_SOUND		0x01			//has sound registers
#define NAMCOT_HAS_IRQ			0x02			//has irq registers
#define NAMCOT_HAS_NTSELECT	0x04			//has nt selection registers
#define NAMCOT_HAS_CHRDISABLE	0x08			//has chr disable register
#define NAMCOT_HAS_WRITEPROT	0x10			//has wram write protection
#define NAMCOT_HAS_RAMENABLE	0x20			//has prg ram enable register
#define NAMCOT_HAS_MIRRCTRL	0x40			//has mirroring control register

#define MAKENAMCOT(n,f)	(((n) << 8) | (f))

#define NAMCOT_129	MAKENAMCOT(0,NAMCOT_HAS_SOUND | NAMCOT_HAS_IRQ | NAMCOT_HAS_NTSELECT | NAMCOT_HAS_CHRDISABLE | NAMCOT_HAS_WRITEPROT)
#define NAMCOT_163	MAKENAMCOT(1,NAMCOT_HAS_SOUND | NAMCOT_HAS_IRQ | NAMCOT_HAS_NTSELECT | NAMCOT_HAS_CHRDISABLE | NAMCOT_HAS_WRITEPROT)
#define NAMCOT_175	MAKENAMCOT(2,NAMCOT_HAS_RAMENABLE)
#define NAMCOT_340	MAKENAMCOT(3,NAMCOT_HAS_NTSELECT | NAMCOT_HAS_MIRRCTRL)

void namcot163_sync();
void namcot163_write(u32 addr,u8 data);
void namcot163_reset(void (*syncfunc)(),int hard);
void namcot163_cpucycle();
void namcot163_state(int mode,u8 *data);

#endif
