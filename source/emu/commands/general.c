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

#include "emu/commands.h"
#include "misc/log.h"
#include "misc/config.h"
#include "mappers/mappers.h"
#include "mappers/mapperid.h"

//just use include file, ditch this
extern int quit;

COMMAND_FUNC(mappers)
{
	int i,j,n,n2;
	const char *str;

	log_printf("supported ines mappers:  ");
	for(n=0,i=0;i<256;i++) {
		if(mapper_get_mapperid_ines(i) >= 0) {
			if(n)
				log_printf(", ");
			log_printf("%d",i);
			n++;
		}
	}

	log_printf("\n\nsupported ines20 mappers:  ");
	for(n2=0,i=0;i<(256 * 1);i++) {
		for(j=1;j<16;j++) {
			if(mapper_get_mapperid_ines20(i,j) >= 0) {
				if(n2)
					log_printf(", ");
				log_printf("%d.%d",i,j);
				n2++;
			}
		}
	}

	log_printf("\n\nsupported unif mappers:\n");
	for(i=0;;i++) {
		if((str = mapper_get_unif_boardname(i)) == 0)
			break;
		log_printf("   %s\n",str);
	}
	log_printf("\n%d ines, %d ines20, %d unif, %d internal boards supported\n",n,n2,i,B_BOARDEND);
	return(0);
}

COMMAND_FUNC(set)
{
	var_t *var;

	//if no arguments was passed, just show the variables
	if(argc < 2) {
		var = config_get_head();
		while(var) {
			log_printf("  %s = %s\n",var->name,var->data);
			var = var->next;
		}
	}

	//we have arguments, validate them and set var
	else {
		char *name = argv[1];
		char *data = argv[2];

		var_set_string(name,data);
	}
	return(0);
}

COMMAND_FUNC(unset)
{
	CHECK_ARGS(2,"usage:  unset <varname>\n");
	var_unset(argv[1]);
	return(0);
}

COMMAND_FUNC(quit)
{
	quit++;
	return(0);
}
