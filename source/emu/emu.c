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

#include "emu/emu.h"
#include "log/log.h"
#include "config/config.h"
#include "system/system.h"
#include "system/video.h"

#define SUBSYSTEM_START	static subsystem_t subsystems[] = {
#define SUBSYSTEM(n)		{n ## _init,n ## _kill},
#define SUBSYSTEM_END	{0,0}};

typedef int (*initfunc_t)();
typedef void (*killfunc_t)();

typedef struct subsystem_s {
	initfunc_t	init;
	killfunc_t	kill;
} subsystem_t;

SUBSYSTEM_START
	SUBSYSTEM(config)
	SUBSYSTEM(log)
	SUBSYSTEM(system)
	SUBSYSTEM(video)
//	SUBSYSTEM(input)
//	SUBSYSTEM(sound)
SUBSYSTEM_END

int emu_init()
{
	int i;

	//loop thru the subsystem function pointers and init
	for(i=0;subsystems[i].init;i++) {
		if(subsystems[i].init() != 0) {
			emu_kill();
			return(1);
		}
	}
	return(0);
}

void emu_kill()
{
	int i;

	//find the end of the pointer list ('i' will equal last one + 1)
	for(i=0;subsystems[i].kill;i++);

	//loop thru the subsystem function pointers backwards and kill
	for(i--;i>=0;i--) {
		subsystems[i].kill();
	}
}
