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
#include "log/log.h"
#include "palette/palette.h"

/* measurement by Chris Covell */
/*
const float emphasis_factor[8][3]={
   {1.0,  1.0,  1.0},
   {1.239,0.915,0.743},
   {0.794,1.086,0.882},
   {1.019,0.98, 0.653},
   {0.905,1.026,1.277},
   {1.023,0.908,0.979},
   {0.741,0.987,1.001},
   {0.75, 0.75, 0.75}
};
*/

/* measurement by Quietust */
const double emphasis_factor[8][3]={
   {1.00, 1.00, 1.00},
   {1.00, 0.80, 0.81},
   {0.78, 0.94, 0.66},
   {0.79, 0.77, 0.63},
   {0.82, 0.83, 1.12},
   {0.81, 0.71, 0.87},
   {0.68, 0.79, 0.79},
   {0.70, 0.70, 0.70}
};

/*void palette_set(palentry_t *p)  //set palette from array
{
	int i,j;

	for(j=0;j<8;j++) {
		for(i=0;i<64;i++) {
			palettes[j][i].r = (u8)((double)p[i].r * emphasis_factor[j][0]);
			palettes[j][i].g = (u8)((double)p[i].g * emphasis_factor[j][1]);
			palettes[j][i].b = (u8)((double)p[i].b * emphasis_factor[j][2]);
		}
	}
	palette = palettes[0];
}*/

palette_t *palette_create()
{
	palette_t *ret = 0;

	ret = (palette_t*)malloc(sizeof(palette_t));
	memset(ret,0,sizeof(palette_t));
	return(ret);
}

void palette_destroy(palette_t *p)
{
	free(p);
}

palette_t *palette_load(char *filename)
{
	palette_t *ret = 0;
	FILE *fp;

	if((fp = fopen(filename,"rb")) == 0) {
		log_printf("palette_load:  error opening '%s'\n",filename);
		return(0);
	}
	
	fclose(fp);
	return(ret);
}

int palette_save(char *filename,palette_t *p)
{
	//not implemented yet
	return(1);
}
