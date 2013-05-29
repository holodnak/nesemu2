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
#include "misc/log.h"
#include "misc/config.h"
#include "palette/palette.h"
#include "palette/generator.h"
#include "system/video.h"

static palette_t *pal = 0;

/* measurement by Quietust */
static const double emphasis_factor[8][3]={
   {1.00, 1.00, 1.00},
   {1.00, 0.80, 0.81},
   {0.78, 0.94, 0.66},
   {0.79, 0.77, 0.63},
   {0.82, 0.83, 1.12},
   {0.81, 0.71, 0.87},
   {0.68, 0.79, 0.79},
   {0.70, 0.70, 0.70}
};

static void generate_emphasis(palette_t *p)
{
	int i,j;

	for(j=1;j<8;j++) {
		for(i=0;i<64;i++) {
			p->pal[j][i].r = (u8)((double)p->pal[0][i].r * emphasis_factor[j][0]);
			p->pal[j][i].g = (u8)((double)p->pal[0][i].g * emphasis_factor[j][1]);
			p->pal[j][i].b = (u8)((double)p->pal[0][i].b * emphasis_factor[j][2]);
		}
	}
}

palette_t *palette_create()
{
	palette_t *ret = 0;

	ret = (palette_t*)mem_alloc(sizeof(palette_t));
	memset(ret,0,sizeof(palette_t));
	return(ret);
}

void palette_destroy(palette_t *p)
{
	mem_free(p);
}

palette_t *palette_load(char *filename)
{
	palette_t *ret = 0;
	FILE *fp;
	u8 colors[3];
	int i;

	if((fp = fopen(filename,"rb")) == 0) {
		log_printf("palette_load:  error opening '%s'\n",filename);
		return(0);
	}
	ret = palette_create();
	for(i=0;i<64;i++) {
		fread(colors,1,3,fp);
		if(feof(fp)) {
			log_printf("palette_load:  unexpected end of file in '%s'\n",filename);
			palette_destroy(ret);
			ret = 0;
			break;
		}
		ret->pal[0][i].r = colors[0];
		ret->pal[0][i].g = colors[1];
		ret->pal[0][i].b = colors[2];
	}
	fclose(fp);
	generate_emphasis(ret);
	return(ret);
}

int palette_save(char *filename,palette_t *p)
{
	//not implemented yet
	return(1);
}

int palette_init()
{
	if(strcmp(config_get_string("palette.source"),"file") == 0) {
		pal = palette_load(config_get_string("palette.filename"));
	}
	if(pal == 0) {
		pal = palette_generate(config_get_int("palette.hue"),config_get_int("palette.saturation"));
	}
	video_setpalette(pal);
	return(0);
}

void palette_kill()
{
	palette_destroy(pal);
}
