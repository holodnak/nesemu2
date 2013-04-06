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

#ifndef __config_h__
#define __config_h__

#include "types.h"

typedef struct configitem_s {
	struct configitem_s *next;
	char	*name;
	u8		*data;
	int	size;
} configitem_t;

typedef struct config_s {
	configitem_t	*head,*tail;
} config_t;

int config_init();
void config_kill();
config_t *config_load(char *filename);
int config_save(char *filename,config_t *cfg);
void config_set(char *name,char *data,int size);
char *config_get(char *name,int *size);

#endif
