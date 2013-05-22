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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "misc/memutil.h"
#include "misc/vars.h"
#include "misc/log.h"

vars_t *vars = 0;

//check if a char is whitespace
static int iswhitespace(char ch)
{
	if(ch == ' ' || ch == '\t' || ch == '\n')
		return(1);
	return(0);
}

//eat whitespace from beginning and end of the string
static char *eatwhitespace(char *str)
{
	char *p,*ret = str;

	while(iswhitespace(*ret))
		ret++;
	p = ret + strlen(ret) - 1;
	while(iswhitespace(*p))
		*p-- = 0;
	return(ret);
}

int vars_init()
{
	vars = vars_create();
	return(0);
}

void vars_kill()
{
	vars_destroy(vars);
}

vars_t *vars_create()
{
	vars_t *ret = 0;

	ret = (vars_t*)mem_alloc(sizeof(vars_t));
	memset(ret,0,sizeof(vars_t));
	return(ret);
}

void vars_destroy(vars_t *vs)
{
	vars_clear(vs);
	mem_free(vs);
}

vars_t *vars_load(char *filename)
{
	vars_t *ret = 0;
	FILE *fp;
	char line[1024],*p,*oldp;

	if((fp = fopen(filename,"rt")) == 0) {
		log_printf("vars_load:  error opening file '%s'\n",filename);
		return(0);
	}

	ret = vars_create();
	while(feof(fp) == 0) {

		//read line from file
		if(fgets(line,1024,fp) == NULL)
			break;

		//skip past any whitespace
		p = eatwhitespace(line);

		//comment or empty string, do nothing
		if(*p == '#' || *p == 0)
			continue;

		//find where to split the string
		if((oldp = strchr(p,'=')) == 0) {
			log_printf("vars_load:  malformed line ('%s')\n",p);
			continue;
		}

		//parse out the name/data pair
		*oldp++ = 0;
		p = eatwhitespace(p);
		oldp = eatwhitespace(oldp);
		vars_add_var(ret,p,oldp);
	}

	log_printf("vars_load:  loaded file '%s'\n",filename);
	fclose(fp);
	return(ret);
}

int vars_save(vars_t *vs,char *filename)
{
	FILE *fp;
	var_t *v = vs->vars;

	if((fp = fopen(filename,"wt")) == 0) {
		log_printf("vars_save:  error opening '%s'\n",filename);
		return(1);
	}
	while(v) {
		fprintf(fp,"%s = %s\n",v->name,v->data);
		v = v->next;
	}
	fclose(fp);
	vs->changed = 0;
	return(0);
}

void vars_clear(vars_t *vs)
{
	var_t *v,*v2;

	v = vs->vars;
	while(v) {
		v2 = v;
		v = v->next;
		mem_free(v2->name);
		mem_free(v2->data);
		mem_free(v2);
	}
	vs->vars = 0;
}

void vars_add_var(vars_t *vs,char *name,char *data)
{
	var_t *v,*var;

	var = (var_t*)mem_alloc(sizeof(var_t));
	memset(var,0,sizeof(var_t));
	var->name = mem_strdup(name);
	var->data = mem_strdup(data);
	if(vs->vars == 0)
		vs->vars = var;
	else {
		v = vs->vars;
		while(v->next)
			v = v->next;
		v->next = var;
	}
	vs->changed++;
}

void vars_delete_var(vars_t *vs,char *name)
{
	var_t *v,*prev;

	prev = 0;
	v = vs->vars;
	while(v) {
		if(strcmp(name,v->name) == 0) {
			if(prev == 0) {
				vs->vars = v->next;
			}
			else {
				prev->next = v->next;
			}
			mem_free(v->name);
			mem_free(v->data);
			mem_free(v);
			return;
		}
		prev = v;
		v = v->next;
	}
	vs->changed++;
}

static char tmpstr[64];

char *vars_get_string(vars_t *vs,char *name,char *def)
{
	var_t *v = vs->vars;

	while(v) {
		if(strcmp(name,v->name) == 0) {
			return(v->data);
		}
		v = v->next;
	}
	return(def);
}

int vars_get_int(vars_t *vs,char *name,int def)
{
	sprintf(tmpstr,"%d",def);
	return(atoi(vars_get_string(vs,name,tmpstr)));
}

double vars_get_double(vars_t *vs,char *name,double def)
{
	sprintf(tmpstr,"%f",def);
	return(atof(vars_get_string(vs,name,tmpstr)));
}

void vars_set_string(vars_t *vs,char *name,char *data)
{
	var_t *v = vs->vars;

	while(v) {
		if(strcmp(name,v->name) == 0) {
			mem_free(v->data);
			v->data = mem_strdup(data);
			vs->changed++;
			return;
		}
		v = v->next;
	}
	if(v == 0)
		vars_add_var(vs,name,data);
}

void vars_set_int(vars_t *vs,char *name,int data)
{
	sprintf(tmpstr,"%d",data);
	vars_set_string(vs,name,tmpstr);
}

void vars_set_double(vars_t *vs,char *name,double data)
{
	sprintf(tmpstr,"%f",data);
	vars_set_string(vs,name,tmpstr);
}
