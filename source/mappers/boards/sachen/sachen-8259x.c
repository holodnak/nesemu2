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

#include "mappers/mapperinc.h"
#include "mappers/chips/sachen-8259.h"

static void reset_8259a(int hard)	{	sachen8259_reset(B_SACHEN_8259A,hard);	}
static void reset_8259b(int hard)	{	sachen8259_reset(B_SACHEN_8259B,hard);	}
static void reset_8259c(int hard)	{	sachen8259_reset(B_SACHEN_8259C,hard);	}
static void reset_8259d(int hard)	{	sachen8259_reset(B_SACHEN_8259D,hard);	}

MAPPER(B_SACHEN_8259A,reset_8259a,0,0,sachen8259_state);
MAPPER(B_SACHEN_8259B,reset_8259b,0,0,sachen8259_state);
MAPPER(B_SACHEN_8259C,reset_8259c,0,0,sachen8259_state);
MAPPER(B_SACHEN_8259D,reset_8259d,0,0,sachen8259_state);
