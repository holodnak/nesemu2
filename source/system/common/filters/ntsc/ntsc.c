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

#include "ntsc.h"
#include "nes_ntsc/nes_ntsc.h"
#include "misc/memutil.h"
#include "system/video.h"
#include "nes/nes.h"

#define NES_NTSC_NO_BLITTERS
#include "nes_ntsc/nes_ntsc.inl"

nes_ntsc_t *ntsc = 0;

static void palette_changed()
{
	nes_ntsc_setup_t setup;
	double sharpness = 100.0f;
	double resolution = 100.0f;
	double artifacts = 100.0f;
	double fringing = 100.0f;
	double bleed = 100.0f;
	int merge_fields = 1;

	setup.hue = 0;
	setup.saturation = 0;
	setup.contrast = 0;
	setup.brightness = 0;
	setup.sharpness = sharpness / 100.0;
	setup.gamma = 0;
	setup.resolution = resolution / 100.0;
	setup.artifacts = artifacts / 100.0;
	setup.fringing = fringing / 100.0;
	setup.bleed = bleed / 100.0;
	setup.merge_fields = merge_fields;
	setup.decoder_matrix = NULL;
	setup.palette_out = NULL;
	setup.palette = NULL;
	setup.base_palette = video_getpalette();
	nes_ntsc_init(ntsc,&setup);
}

int ntsc_init()
{
	ntsc_kill();
	ntsc = (nes_ntsc_t*)mem_alloc(sizeof(nes_ntsc_t));
	palette_changed();
	return(0);
}

void ntsc_kill()
{
	if(ntsc)
		mem_free(ntsc);
	ntsc = 0;
}

void ntsc_palette_changed()
{
	if(ntsc)
		palette_changed();
}

void ntsc2x(void *d,u32 dst_slice,void *s,u32 src_slice,u32 width,u32 height)
{
	static int phase = 0;
	int bpp = 32;
	int i,h;
	int chunk_count = (256 - 1) / nes_ntsc_in_chunk;
	u8 *screen = video_getscreen();
	u8 *src = screen;
	u32 *dst = (u32*)d;
	u8 *pal = nes->ppu.palette;

	for(h=0;h<240;h++) {
		u32 *dststart = dst;

		NES_NTSC_BEGIN_ROW( ntsc, phase, nes_ntsc_black, nes_ntsc_black, pal[*src & 0x1F] | ((*src & 0xE0) << 1) );
		src++;

		for(i=0;i<chunk_count;i++,src+=3,dst+=7) {

			NES_NTSC_COLOR_IN( 0, pal[src[0] & 0x1F] | ((src[0] & 0xE0) << 1) );
			NES_NTSC_RGB_OUT( 0, dst[0], bpp );
			NES_NTSC_RGB_OUT( 1, dst[1], bpp );

			NES_NTSC_COLOR_IN( 1, pal[src[1] & 0x1F] | ((src[1] & 0xE0) << 1) );
			NES_NTSC_RGB_OUT( 2, dst[2], bpp );
			NES_NTSC_RGB_OUT( 3, dst[3], bpp );

			NES_NTSC_COLOR_IN( 2, pal[src[2] & 0x1F] | ((src[2] & 0xE0) << 1) );
			NES_NTSC_RGB_OUT( 4, dst[4], bpp );
			NES_NTSC_RGB_OUT( 5, dst[5], bpp );
			NES_NTSC_RGB_OUT( 6, dst[6], bpp );

		}

		NES_NTSC_COLOR_IN( 0, nes_ntsc_black );
		NES_NTSC_RGB_OUT( 0, dst[0], bpp );
		NES_NTSC_RGB_OUT( 1, dst[1], bpp );

		NES_NTSC_COLOR_IN( 1, nes_ntsc_black );
		NES_NTSC_RGB_OUT( 2, dst[2], bpp );
		NES_NTSC_RGB_OUT( 3, dst[3], bpp );

		NES_NTSC_COLOR_IN( 2, nes_ntsc_black );
		NES_NTSC_RGB_OUT( 4, dst[4], bpp );
		NES_NTSC_RGB_OUT( 5, dst[5], bpp );
		NES_NTSC_RGB_OUT( 6, dst[6], bpp );

		phase = (phase + 1) % 3;
		memcpy((u8*)dststart + dst_slice,dststart,dst_slice);
		src = screen + (h * 256);
		dst = (u32*)((u8*)d + dst_slice * (h * 2));
	}
}

void ntsc3x(void *dst,u32 dst_slice,void *src,u32 src_slice,u32 width,u32 height)
{
}

void ntsc4x(void *dst,u32 dst_slice,void *src,u32 src_slice,u32 width,u32 height)
{
}
