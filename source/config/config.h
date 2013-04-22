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

typedef struct configvar_s {
	struct configvar_s *next;
	char *name;
	char *data;
} configvar_t;

int config_init();
void config_kill();

//load configuration
int config_load(char *filename);

//save configuration
int config_save(char *filename);

//add config var
void config_add_var(char *name,char *data);

//delete config var
void config_delete_var(char *name);

//get config var
char *config_get_string(char *name,char *def);
int config_get_int(char *name,int def);
double config_get_double(char *name,double def);

//set config var
void config_set_string(char *name,char *data);
void config_set_int(char *name,int data);
void config_set_double(char *name,double data);

#endif
