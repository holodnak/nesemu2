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

#ifndef __events_h__
#define __events_h__

#include "types.h"

//event id's
enum events_e {
	E_UNKNOWN = 0,
	E_QUIT,
	E_LOADROM,
	E_LOADPATCH,
	E_UNLOAD,
	E_SOFTRESET,
	E_HARDRESET,
	E_SAVESTATE,
	E_LOADSTATE,
	E_RECORDMOVIE,
	E_PLAYMOVIE,
	E_STOPMOVIE,
	E_LOADMOVIE,
	E_SAVEMOVIE,
	E_FLIPDISK,
	E_DUMPDISK,
	E_AUTOTEST,

	//change running state
	E_TOGGLERUNNING,
	E_PAUSE,
	E_UNPAUSE,

	//change from fullscreen/windowed
	E_TOGGLEFULLSCREEN,
	E_FULLSCREEN,
	E_WINDOWED,

	//last valid event id
	E_LASTEVENT,

	//clones!
	E_RESET = E_SOFTRESET,
};

int emu_event(int id,void *data);

#endif
