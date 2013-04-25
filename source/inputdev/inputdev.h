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

#ifndef __inputdev_h__
#define __inputdev_h__

#include "types.h"

//nes input keys for joypads
#define INPUT_A		0x01
#define INPUT_B		0x02
#define INPUT_SELECT	0x04
#define INPUT_START	0x08
#define INPUT_UP		0x10
#define INPUT_DOWN	0x20
#define INPUT_LEFT	0x40
#define INPUT_RIGHT	0x80

#define INPUTDEV(id,read,write,strobe,update) \
	inputdev_t inputdev##id = {id,read,write,strobe,update}

typedef struct inputdev_s {
	//device id
	int id;

	//read port
	u8 (*read)();

	//write port
	void (*write)(u8);

	//strobe
	void (*strobe)();

	//update controller info
	void (*update)();
} inputdev_t;

enum inputdevid_e {
	I_NULL,
	I_JOYPAD0,
	I_JOYPAD1,
	I_ZAPPER,
	I_POWERPAD,
};

inputdev_t *inputdev_get(int id);

#endif
