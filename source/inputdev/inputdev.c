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

#include "inputdev.h"

#ifdef INPUTDEV
	#undef INPUTDEV
#endif

#define INPUTDEV(n) { \
	extern inputdev_t inputdev##n; \
	if(inputdevid == n) \
		return(&inputdev##n); \
	}

static inputdev_t *get_inputdev(int inputdevid)
{
	//null device
	INPUTDEV(I_NULL);

	//standard input devices
	INPUTDEV(I_JOYPAD0);
	INPUTDEV(I_JOYPAD1);
	INPUTDEV(I_ZAPPER);
	INPUTDEV(I_POWERPAD);

	return(0);
}

static u8 null_read()				{return(0);}
static void null_write(u8 data)	{}
static void null_strobe()			{}
static void null_update()			{}

inputdev_t *inputdev_get(int inputdevid)
{
	inputdev_t *ret = get_inputdev(inputdevid);

	if(ret == 0) {
		return(get_inputdev(I_NULL));
	}
	ret->read = (ret->read == 0) ? null_read : ret->read;
	ret->write = (ret->write == 0) ? null_write : ret->write;
	ret->strobe = (ret->strobe == 0) ? null_strobe : ret->strobe;
	ret->update = (ret->update == 0) ? null_update : ret->update;
	return(ret);
}
