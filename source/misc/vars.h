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

#ifndef __vars_h__
#define __vars_h__

#define F_VAR		0			//variable that is normal
#define F_CONFIG	1			//variable that is part of the configuration (gets saved)

typedef struct var_s {
	struct var_s *next;
	int flags;
	char *name;
	char *data;
} var_t;

typedef struct vars_s {
	var_t *vars;
	int changed;
} vars_t;

//init vars
int vars_init();

//kill vars
void vars_kill();

//create empty var list
vars_t *vars_create();

//destroy var list
void vars_destroy(vars_t *vs);

//load vars from file
vars_t *vars_load(char *filename);

//save vars to a file
int vars_save(vars_t *vars,char *filename);

//merge all vars from src into dest
void vars_merge(vars_t *dest,vars_t *src);

//delete all vars
void vars_clear(vars_t *vs);

//add var
var_t *vars_add_var(vars_t *vs,int flags,char *name,char *data);

//delete var
void vars_delete_var(vars_t *vs,char *name);

//add var
var_t *vars_get_var(vars_t *vs,char *name);

//get var
char *vars_get_string(vars_t *vars,char *name,char *def);
int vars_get_int(vars_t *vars,char *name,int def);
int vars_get_bool(vars_t *vars,char *name,int def);
double vars_get_double(vars_t *vars,char *name,double def);

//set var
var_t *vars_set_string(vars_t *vars,int flags,char *name,char *data);
var_t *vars_set_int(vars_t *vars,int flags,char *name,int data);
var_t *vars_set_bool(vars_t *vars,int flags,char *name,int data);
var_t *vars_set_double(vars_t *vars,int flags,char *name,double data);

#endif
