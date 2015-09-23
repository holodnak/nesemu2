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

#define C_MMCNUM		0xF0
#define C_MMCREV		0x0F

#define MAPPER(boardid,reset,ppucycle,cpucycle,state) \
	mapper_t mapper##boardid = {boardid,reset,ppucycle,cpucycle,state}

#include "mappers/mappers.h"
#include "mappers/mapperid.h"
#include "nes/nes.h"
#include "nes/memory.h"
#include "nes/state/state.h"
#include "misc/log.h"

#endif
