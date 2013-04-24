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

#ifndef __mappers_h__
#define __mappers_h__

#include "types.h"

typedef struct mapper_s {
	int boardid;					//internal board id
	void (*reset)(int);			//mapper reset function
	void (*tile)(int);			//tile read handler
	void (*ppucycle)();			//ppu cycle handler
	void (*cpucycle)();			//cpu cycle handler
	void (*state)(int,u8*);		//load/save state information
} mapper_t;

//converting from ines|ines20|unif to internal board id
int mapper_get_mapperid_ines(int num);
int mapper_get_mapperid_ines20(int num,int sub);
int mapper_get_mapperid_unif(char *board);

//helper for getting supported mapper list
const char *mapper_get_unif_boardname(int idx);

//initialize mapper and get mapper_t struct
mapper_t *mapper_init(int mapperid);

#endif
