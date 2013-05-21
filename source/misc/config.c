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
#include "system/main.h"
#include "misc/memutil.h"
#include "misc/config.h"
#include "misc/log.h"

static configvar_t *config = 0;

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

int config_init()
{
	config_load(configfilename);
	return(0);
}

void config_kill()
{
	configvar_t *v,*v2;

	config_save(configfilename);
	v = config;
	while(v) {
		v2 = v;
		v = v->next;
		mem_free(v2->name);
		mem_free(v2->data);
		mem_free(v2);
	}
	config = 0;
}

int config_load(char *filename)
{
	FILE *fp;
	char line[1024],*p,*oldp;

	if((fp = fopen(filename,"rt")) == 0) {
		log_printf("config_load:  error opening configuration file '%s'\n",filename);
		return(1);
	}

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
			log_printf("config_load:  malformed configuration line ('%s')\n",p);
			continue;
		}

		//parse out the name/data pair
		*oldp++ = 0;
		p = eatwhitespace(p);
		oldp = eatwhitespace(oldp);
		config_add_var(p,oldp);
	}

	log_printf("config_load:  loaded configuration file '%s'\n",filename);
	fclose(fp);
	return(0);
}

int config_save(char *filename)
{
	FILE *fp;
	configvar_t *v = config;

	if((fp = fopen(filename,"wt")) == 0) {
		log_printf("config_save:  error opening '%s'\n",filename);
		return(1);
	}
	fprintf(fp,"# nesemu2 configuration\n\n");
	while(v) {
		fprintf(fp,"%s = %s\n",v->name,v->data);
		v = v->next;
	}
	fclose(fp);
	return(0);
}

void config_add_var(char *name,char *data)
{
	configvar_t *v,*var;

	var = (configvar_t*)mem_alloc(sizeof(configvar_t));
	memset(var,0,sizeof(configvar_t));
	var->name = mem_strdup(name);
	var->data = mem_strdup(data);
	if(config == 0)
		config = var;
	else {
		v = config;
		while(v->next)
			v = v->next;
		v->next = var;
	}
}

void config_delete_var(char *name)
{
	configvar_t *v,*prev;

	prev = 0;
	v = config;
	while(v) {
		if(strcmp(name,v->name) == 0) {
			if(prev == 0) {
				config = v->next;
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
}

static char tmpstr[64];

char *config_get_string(char *name,char *def)
{
	configvar_t *v = config;

	while(v) {
		if(strcmp(name,v->name) == 0) {
			return(v->data);
		}
		v = v->next;
	}
	return(def);
}

int config_get_int(char *name,int def)
{
	sprintf(tmpstr,"%d",def);
	return(atoi(config_get_string(name,tmpstr)));
}

double config_get_double(char *name,double def)
{
	sprintf(tmpstr,"%f",def);
	return(atof(config_get_string(name,tmpstr)));
}

void config_set_string(char *name,char *data)
{
	configvar_t *v = config;

	while(v) {
		if(strcmp(name,v->name) == 0) {
			mem_free(v->data);
			v->data = mem_strdup(data);
			return;
		}
		v = v->next;
	}
	if(v == 0)
		config_add_var(name,data);
}

void config_set_int(char *name,int data)
{
	sprintf(tmpstr,"%d",data);
	config_set_string(name,tmpstr);
}

void config_set_double(char *name,double data)
{
	sprintf(tmpstr,"%f",data);
	config_set_string(name,tmpstr);
}
