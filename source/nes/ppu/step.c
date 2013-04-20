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

#include "nes/nes.h"
#include "system/video.h"
#include "log/log.h"

typedef struct {
	u64 line;				//cache line data
	u8 attr;					//attrib bits
	u8 x;						//x coord
	u8 flags;				//flags
	u8 tile;					//sprite tile index
	} sprtemp_t;			//sprite temp entry

static sprtemp_t sprtemp[8];

static INLINE void blankline()
{
	u64 *scr64 = (u64*)nes.ppu.linebuffer;
	int i;

	for(i=0;i<33;i++) {
		*scr64++ = 0;
	}
}

static INLINE void drawtileline()
{
	u64 *scr64 = (u64*)nes.ppu.linebuffer;
	u8 *attribptr = nes.ppu.attribdata;
	cache_t *cacheptr = nes.ppu.cachedata;
	int i;

	//draw all pixels
	for(i=0;i<33;i++) {
		*scr64++ = (*attribptr++ * 0x0404040404040404LL) + *cacheptr++;
	}

	//hide leftmost 8 pixels
	if((CONTROL1 & 2) == 0) {
		for(i=0;i<(8 + nes.ppu.scrollx);i++) {
			nes.ppu.linebuffer[i] = 0;
		}
	}
}

static INLINE void drawspriteline()
{
	u8 spriteline[256 + 8];
	sprtemp_t *spr = (sprtemp_t*)sprtemp + 7;
	u64 *spriteline64 = (u64*)spriteline;
	u8 *dest = nes.ppu.linebuffer;
	int n,j,x;

	//adjust for fine scroll x
	dest += nes.ppu.scrollx;

	//clear sprite line
	for(n=0;n<(256 / 8);n++)
		spriteline64[n] = 0x00;

	//loop thru all eight possible sprites
	for(n=0;n<8;n++,spr--) {
		cache_t offs;
		u64 *scr64,sp0,sp1,pmask0,pmask1,pmaskxor = 0xFFFFFFFFFFFFFFFFLL;
		u64 shiftright = (8 - (spr->x & 7)) * 8;
		u64 shiftleft = (spr->x & 7) * 8;

		if(spr->flags == 0)
			continue;

		//get offset in sprite buffer
		scr64 = ((u64*)spriteline) + (spr->x / 8);

		//setup to draw the pixel
		offs = spr->attr * 0x0404040404040404LL;
		sp0 = sp1 = spr->line & CACHE_MASK;
		pmask0 = pmask1 = (0x8080808080808080LL - sp0) >> 2;

		//sprite is sprite 0, check for hit
		if(spr->flags & 2) {
			u64 sprtileline;
			u8 *sprtileline8,*bg;

			sprtileline = sp0 & pmask0;
			sprtileline8 = ((u8*)&sprtileline);
			bg = dest + spr->x;
			for(x=spr->x,j=0;j<8 && (spr->x + j) < 255;j++,x++) {
				if(((CONTROL1 & 4) == 0) && (x < 8))
					continue;
				if(sprtileline8[j] && ((bg[j] & 0xF) != 0)) {
					STATUS |= 0x40;
					spr->flags = 0;
					break;
				}
			}
		}

		//render pixel to sprite line buffer
#ifdef __GNUC__
		if(spr->x & 7) {
#endif
			sp0 <<= shiftleft;
			sp1 >>= shiftright;
			pmask0 <<= shiftleft;
			pmask1 >>= shiftright;
			scr64[0] = ((sp0 + offs) & pmask0) | (scr64[0] & ~pmask0);
			scr64[1] = ((sp1 + offs) & pmask1) | (scr64[1] & ~pmask1);
#ifdef __GNUC__
		}
		else
			scr64[0] = ((sp0 + offs) & pmask0) | (scr64[0] & ~pmask0);
#endif
	}

	//draw the sprite line to the buffer
	for(n=(((CONTROL1 ^ 4) & 4) << 1);n<256;n++) {
		u8 pixel = spriteline[n];

		if(pixel & 3) {
			if((pixel & 0x10) == 0)    //foreground sprite
				dest[n] = pixel | 0x10;
			else if((dest[n] & 3) == 0)      //background sprite that is visible
				dest[n] = pixel | 0x10;
		}
	}
}

static INLINE void calc_ntaddr()
{
	nes.ppu.busaddr = 0x2000 | (SCROLL & 0xFFF);
}

static INLINE void calc_attribaddr()
{
	nes.ppu.busaddr = SCROLL & 0x3C00;
	nes.ppu.busaddr += 0x3C0 + ((SCROLL >> 2) & 7) + (((SCROLL >> (2 + 5)) & 7) << 3);
	nes.ppu.busaddr |= 0x2000;
}

static INLINE void calc_pattern0addr()
{
	nes.ppu.busaddr = (CONTROL0 & 0x10) << 8;
	nes.ppu.busaddr += nes.ppu.ntbyte * 16;
	nes.ppu.busaddr += SCROLL >> 12;
}

static INLINE void calc_pattern1addr()
{
	nes.ppu.busaddr += 8;
}

static INLINE void calc_spritepattern0addr()
{
	//process 8x16 sprite
	if(CONTROL0 & 0x20) {
		//bank to get tile from
		nes.ppu.busaddr = (sprtemp[nes.ppu.cursprite].tile & 1) << 12;

		//tile offset
		nes.ppu.busaddr += (sprtemp[nes.ppu.cursprite].tile & 0xFE) * 16;
	}

	//process 8x8 sprite
	else {
		//bank to get tile from
		nes.ppu.busaddr = (CONTROL0 & 8) << 9;

		//tile offset
		nes.ppu.busaddr += sprtemp[nes.ppu.cursprite].tile * 16;
	}
}

static INLINE void calc_spritepattern1addr()
{
	nes.ppu.busaddr += 8;
	nes.ppu.cursprite++;
}

static INLINE void fetch_ntbyte()
{
	if(CONTROL1 & 0x08) {
		nes.ppu.ntbyte = ppu_memread(nes.ppu.busaddr);

		//mapper tile callback
		nes.mapper->tile(nes.ppu.ntbyte,((CONTROL0 & 0x10) >> 2) >> 2);
	}
}

static INLINE void fetch_attribbyte()
{
	u32 tmp;

	if(CONTROL1 & 0x08) {
#ifdef CACHE_ATTRIB
		tmp = SCROLL & 0xFFF;
		nes.ppu.attribdata[nes.ppu.fetchpos] = nes.ppu.attribpages[tmp >> 10][tmp & 0x3FF];
#else
		tmp = ((ppu_memread(nes.ppu.busaddr) >> (((SCROLL & 2) | (((SCROLL >> 5) & 2) << 1)))) & 3);
		nes.ppu.attribdata[nes.ppu.fetchpos] = tmp;
#endif
	}
}

static INLINE void fetch_patternbyte0()
{
	if(CONTROL1 & 0x08) {
		cache_t *cache,pixels;

		nes.ppu.tiledata[0][nes.ppu.fetchpos] = ppu_memread(nes.ppu.busaddr);

		//tile bank cache pointer
		cache = nes.ppu.cachepages[(nes.ppu.ntbyte >> 6) | ((CONTROL0 & 0x10) >> 2)];

		//index to the tile data start, then the tile half (upper or lower half)
		cache += ((nes.ppu.ntbyte & 0x3F) * 2) + ((SCROLL >> 14) & 1);

		//retreive the tile pixels used
		pixels = *cache >> (((SCROLL >> 12) & 3) << 1);

		nes.ppu.cachedata[nes.ppu.fetchpos] = pixels & CACHE_MASK;
	}
}

static INLINE void fetch_patternbyte1()
{
	if(CONTROL1 & 0x08) {
		nes.ppu.tiledata[1][nes.ppu.fetchpos] = ppu_memread(nes.ppu.busaddr);
	}
}

static INLINE void inc_hscroll()
{
	if(CONTROL1 & 0x18) {
		/*	The first one, the horizontal scroll counter, consists of 6 bits, and is
		made up by daisy-chaining the HT counter to the H counter. The HT counter is
		then clocked every 8 pixel dot clocks (or every 8/3 CPU clock cycles). */
		if((SCROLL & 0x1F) == 0x1F)		//see if HT counter creates carry
			SCROLL ^= 0x41F;					//yes, clear lower 5 bits and toggle H counter
		else
			SCROLL++;							//no, increment address
	}
	nes.ppu.fetchpos++;
}

static INLINE void inc_vscroll()
{
	int n;

	if(CONTROL1 & 0x18) {
		//update y coordinate
		if((SCROLL >> 12) == 7) {
			SCROLL &= ~0x7000;
			n = (SCROLL >> 5) & 0x1F;
			if(n == 29) {
				SCROLL &= ~0x03E0;
				SCROLL ^= 0x0800;
			}
			else if(n == 31)
				SCROLL &= ~0x03E0;
			else
				SCROLL += 0x20;
		}
		else
			SCROLL += 0x1000;
	}
}

static INLINE void update_hscroll()
{
	if(CONTROL1 & 0x18) {
		SCROLL &= ~0x041F;
		SCROLL |= TMPSCROLL & 0x041F;
	}
	nes.ppu.fetchpos = 0;
	nes.ppu.cursprite = 0;
}

static INLINE void update_scroll()
{
	if(CONTROL1 & 0x18)
		SCROLL = TMPSCROLL;
}

static INLINE void inc_linecycles()
{
	LINECYCLES++;
	if(LINECYCLES >= 341) {
		LINECYCLES = 0;
		SCANLINE++;
		if(SCANLINE >= 262) {
			SCANLINE = 0;
			FRAMES++;
		}
	}
}

static INLINE void skip_cycle()
{
	if((FRAMES & 1) && (CONTROL1 & 0x18))
		inc_linecycles();
}

static INLINE void clear_nmi()
{
	STATUS = 0;
	cpu_set_nmi(0);
}

static INLINE void set_nmi()
{
	STATUS |= 0x80;
	if(CONTROL0 & 0x80)
		cpu_set_nmi(1);
}

static INLINE void process_sprites()
{
	if(LINECYCLES == 0) {
		nes.ppu.oam2pos = 0;
	}

	/* Cycles 1-64: Secondary OAM (32-byte buffer for current sprites on scanline) is
	   initialized to $FF - attempting to read $2004 will return $FF */
	else if((LINECYCLES <= 64) && ((LINECYCLES & 1) == 0)) {
		nes.ppu.oam2[nes.ppu.oam2pos++] = 0xFF;
	}

	/* Cycles 65-256: Sprite evaluation
	On odd cycles, data is read from (primary) OAM
	On even cycles, data is written to secondary OAM (unless writes are inhibited, in
	   which case it will read the value in secondary OAM instead)
	1. Starting at n = 0, read a sprite's Y-coordinate (OAM[n][0], copying it to the
	   next open slot in secondary OAM (unless 8 sprites have been found, in which case the write is ignored).
	1a. If Y-coordinate is in range, copy remaining bytes of sprite data (OAM[n][1] thru
	    OAM[n][3]) into secondary OAM.
	2. Increment n
	2a. If n has overflowed back to zero (all 64 sprites evaluated), go to 4
	2b. If less than 8 sprites have been found, go to 1
	2c. If exactly 8 sprites have been found, disable writes to secondary OAM. This causes sprites in back to drop out.
	3. Starting at m = 0, evaluate OAM[n][m] as a Y-coordinate.
	3a. If the value is in range, set the sprite overflow flag in $2002 and read the next 3 entries
	    of OAM (incrementing 'm' after each byte and incrementing 'n' when 'm' overflows); if m = 3, increment n
	3b. If the value is not in range, increment n AND m (without carry). If n overflows to 0, go to 4; otherwise go to 3
	4. Attempt (and fail) to copy OAM[n][0] into the next free slot in secondary OAM, and increment n (repeat until HBLANK is reached) */
	else if(LINECYCLES <= 256) {

	}

	/* Cycles 257-320: Sprite fetches (8 sprites total, 8 cycles per sprite)
	1-4: Read the Y-coordinate, tile number, attributes, and X-coordinate
	     of the selected sprite from secondary OAM
	5-8: Read the X-coordinate of the selected sprite from secondary OAM
	     4 times (while the PPU fetches the sprite tile data)
	For the first empty sprite slot, this will consist of sprite #63's Y-coordinate
	followed by 3 $FF bytes; for subsequent empty sprite slots, this will be four $FF bytes */
	else if(LINECYCLES <= 320) {

	}

	/* Cycles 321-340+0: Background render pipeline initialization
    Read the first byte in secondary OAM (while the PPU fetches
	 the first two background tiles for the next scanline) */
	else {

	}
}

//process all sprites that belong to the next scanline
static INLINE void quick_process_sprites()
{
	int i,h,sprinrange;
	u8 *s;

	//clear the sprite temp memory
	for(i=0;i<8;i++) {
		sprtemp[i].line = 0;
		sprtemp[i].attr = sprtemp[i].x = sprtemp[i].flags = 0;
	}

	//determine sprite height
	h = 8 + ((CONTROL0 & 0x20) >> 2);

	//loop thru all 64 visible sprites, keeping note of the first eight visible
	for(sprinrange=0,i=0;i<64;i++) {
		int sprline;
		cache_t *cache;
		u8 patternbank;

		//sprite data pointer
		s = &nes.ppu.oam[i * 4];

		//get the sprite tile line to draw
		sprline = (SCANLINE - 21) - s[0];

		//check for visibility on this line
		if(sprline >= 0 && sprline < h) {

			//if more than 8 sprites are found, set the flag and exit the loop
			if(sprinrange == 8) {
				STATUS |= 0x20;
				break;
			}

			//copy sprite data to temp memory
			sprtemp[sprinrange].attr = (s[2] & 3) | ((s[2] & 0x20) >> 3);
			sprtemp[sprinrange].x = s[3];
			sprtemp[sprinrange].flags = 1;
			sprtemp[sprinrange].tile = s[1];

			//if sprite0 check is needed
			if(i == 0)
				sprtemp[sprinrange].flags = 2;

			//call mapper callback
			nes.mapper->tile(s[1],(CONTROL0 & 8) >> 3);

			//determine pattern bank used (upper or lower)
			if(CONTROL0 & 0x20)
				patternbank = (s[1] & 1) << 2;
			else
				patternbank = (CONTROL0 & 8) >> 1;

			//if sprite is to be flipped horizontally
			if(s[2] & 0x40)
				cache = (cache_t*)nes.ppu.cachepages_hflip[(s[1] >> 6) | patternbank];
			else
				cache = (cache_t*)nes.ppu.cachepages[(s[1] >> 6) | patternbank];

			//select tile for 8x16 sprites
			if(CONTROL0 & 0x20) {
				cache += ((s[1] & 0x3E) * 2) + ((s[2] & 0x80) >> 6);
				if(sprline >= 8) {
					if(s[2] & 0x80)
						cache -= 2;
					else
						cache += 2;
					sprline &= 7;
				}
			}

			//select tile for 8x8 sprites
			else
				cache += (s[1] & 0x3F) * 2;

			//if sprite is to be flipped vertically
			if((s[2] & 0x80) != 0)
				sprline = 7 - sprline;

			//select tile line
			cache += (sprline / 4);

			//store sprite tile line
			sprtemp[sprinrange].line = *cache >> ((sprline & 3) * 2);

			//increment sprite in range counter
			sprinrange++;

		}
	}
}

static INLINE void scanline_prerender()
{
//	if(LINECYCLES == 3)
//		clear_nmi();
	/* There are 2 conditions that update all 5 PPU scroll counters with the
	contents of the latches adjacent to them. The first is after a write to
	2006/2. The second, is at the beginning of scanline 20, when the PPU starts
	rendering data for the first time in a frame (this update won't happen if
	all rendering is disabled via 2001.3 and 2001.4). */
	switch(LINECYCLES) {

		//the idle cycle
		case 0:
			break;

		//nametable byte read
		case 1:
			//clear ppu status register
			clear_nmi();
		case 9:		case 17:		case 25:		case 33:		case 41:		case 49:		case 57:
		case 65:		case 73:		case 81:		case 89:		case 97:		case 105:	case 113:	case 121:
		case 129:	case 137:	case 145:	case 153:	case 161:	case 169:	case 177:	case 185:
		case 193:	case 201:	case 209:	case 217:	case 225:	case 233:	case 241:	case 249:
			calc_ntaddr();
			break;
		case 2:		case 10:		case 18:		case 26:		case 34:		case 42:		case 50:		case 58:
		case 66:		case 74:		case 82:		case 90:		case 98:		case 106:	case 114:	case 122:
		case 130:	case 138:	case 146:	case 154:	case 162:	case 170:	case 178:	case 186:
		case 194:	case 202:	case 210:	case 218:	case 226:	case 234:	case 242:	case 250:
			fetch_ntbyte();
			break;

		//attribute table byte read
		case 3:		case 11:		case 19:		case 27:		case 35:		case 43:		case 51:		case 59:
		case 67:		case 75:		case 83:		case 91:		case 99:		case 107:	case 115:	case 123:
		case 131:	case 139:	case 147:	case 155:	case 163:	case 171:	case 179:	case 187:
		case 195:	case 203:	case 211:	case 219:	case 227:	case 235:	case 243:	case 251:
			calc_attribaddr();
			break;
		case 4:		case 12:		case 20:		case 28:		case 36:		case 44:		case 52:		case 60:
		case 68:		case 76:		case 84:		case 92:		case 100:	case 108:	case 116:	case 124:
		case 132:	case 140:	case 148:	case 156:	case 164:	case 172:	case 180:	case 188:
		case 196:	case 204:	case 212:	case 220:	case 228:	case 236:	case 244:	case 252:
			fetch_attribbyte();
			break;

		//pattern table byte 0 read
		case 5:		case 13:		case 21:		case 29:		case 37:		case 45:		case 53:		case 61:
		case 69:		case 77:		case 85:		case 93:		case 101:	case 109:	case 117:	case 125:
		case 133:	case 141:	case 149:	case 157:	case 165:	case 173:	case 181:	case 189:
		case 197:	case 205:	case 213:	case 221:	case 229:	case 237:	case 245:	case 253:
			calc_pattern0addr();
			break;
		case 6:		case 14:		case 22:		case 30:		case 38:		case 46:		case 54:		case 62:
		case 70:		case 78:		case 86:		case 94:		case 102:	case 110:	case 118:	case 126:
		case 134:	case 142:	case 150:	case 158:	case 166:	case 174:	case 182:	case 190:
		case 198:	case 206:	case 214:	case 222:	case 230:	case 238:	case 246:	case 254:
			fetch_patternbyte0();
			break;

		//pattern table byte 1 read
		case 7:		case 15:		case 23:		case 31:		case 39:		case 47:		case 55:		case 63:
		case 71:		case 79:		case 87:		case 95:		case 103:	case 111:	case 119:	case 127:
		case 135:	case 143:	case 151:	case 159:	case 167:	case 175:	case 183:	case 191:
		case 199:	case 207:	case 215:	case 223:	case 231:	case 239:	case 247:	case 255:
			calc_pattern1addr();
			break;
		case 8:		case 16:		case 24:		case 32:		case 40:		case 48:		case 56:		case 64:
		case 72:		case 80:		case 88:		case 96:		case 104:	case 112:	case 120:	case 128:
		case 136:	case 144:	case 152:	case 160:	case 168:	case 176:	case 184:	case 192:
		case 200:	case 208:	case 216:	case 224:	case 232:	case 240:	case 248://	case 256:
			fetch_patternbyte1();
			inc_hscroll();
			break;
		case 256:
			fetch_patternbyte1();
			inc_hscroll();
			inc_vscroll();
#ifdef QUICK_SPRITES
			quick_process_sprites();
#endif
			break;

		//garbage nametable address calc
		case 257:
			update_hscroll();
		case 265:	case 273:	case 281:	case 289:	case 297:	case 305:	case 313:
		//	calc_ntaddr();
			break;

		//garbage nametable fetch
		case 258:	case 266:	case 274:	case 282:	case 290:	case 298:	case 306:	case 314:
			break;

		//garbage attribute address calc
		case 259:	case 267:	case 275:	case 283:	case 291:	case 299:	case 307:	case 315:
		//	calc_attribaddr();
			break;

		//garbage attribute fetch
		case 260:	case 268:	case 276:	case 284:	case 292:	case 300:	case 308:	case 316:
			break;

		//calculate address of sprite pattern low
		case 261:	case 269:	case 277:	case 285:	case 293:	case 301:	case 309:	case 317:
		//	calc_spritepattern0addr();
			break;

		//fetch sprite pattern low
		case 262:	case 270:	case 278:	case 286:	case 294:	case 302:	case 310:	case 318:
			break;

		//calculate address of sprite pattern high
		case 263:	case 271:	case 279:	case 287:	case 295:	case 303:	case 311:	case 319:
		//	calc_spritepattern1addr();
			break;

		//fetch sprite pattern high (update scroll registers on cycle 304)
		case 304:
			update_scroll();
		case 264:	case 272:	case 280:	case 288:	case 296:	case 312:	case 320:
			break;

		//nametable byte for next scanline
		case 321:	case 329:
			calc_ntaddr();
			break;
		case 322:	case 330:
			fetch_ntbyte();
			break;

		//attribute byte for next scanline
		case 323:	case 331:
			calc_attribaddr();
			break;
		case 324:	case 332:
			fetch_attribbyte();
			break;

		//pattern low byte tile
		case 325:	case 333:
			calc_pattern0addr();
			break;
		case 326:	case 334:
			fetch_patternbyte0();
			break;

		//pattern high byte tile
		case 327:	case 335:
			calc_pattern1addr();
			break;
		case 328:	case 336:
			fetch_patternbyte1();
			inc_hscroll();
			break;

		//garbage nametable fetches
		case 337:	case 339:
			calc_ntaddr();
			break;
		case 340:
			skip_cycle();
		case 338:	
			fetch_ntbyte();
			break;

		default:
//			log_printf("scanline_20:  unhandled cycle %d\n",LINECYCLES);
			break;
	}
#ifndef QUICK_SPRITES
	if(CONTROL1 & 0x10)
		process_sprites();
#endif
}

//scanlines 21-260
static INLINE void scanline_visible()
{
	switch(LINECYCLES) {
		//the idle cycle
		case 0:
			break;

		//nametable byte read
		case 1:		case 9:		case 17:		case 25:		case 33:		case 41:		case 49:		case 57:
		case 65:		case 73:		case 81:		case 89:		case 97:		case 105:	case 113:	case 121:
		case 129:	case 137:	case 145:	case 153:	case 161:	case 169:	case 177:	case 185:
		case 193:	case 201:	case 209:	case 217:	case 225:	case 233:	case 241:	case 249:
			calc_ntaddr();
			break;
		case 2:		case 10:		case 18:		case 26:		case 34:		case 42:		case 50:		case 58:
		case 66:		case 74:		case 82:		case 90:		case 98:		case 106:	case 114:	case 122:
		case 130:	case 138:	case 146:	case 154:	case 162:	case 170:	case 178:	case 186:
		case 194:	case 202:	case 210:	case 218:	case 226:	case 234:	case 242:	case 250:
			fetch_ntbyte();
			break;

		//attribute table byte read
		case 3:		case 11:		case 19:		case 27:		case 35:		case 43:		case 51:		case 59:
		case 67:		case 75:		case 83:		case 91:		case 99:		case 107:	case 115:	case 123:
		case 131:	case 139:	case 147:	case 155:	case 163:	case 171:	case 179:	case 187:
		case 195:	case 203:	case 211:	case 219:	case 227:	case 235:	case 243:	case 251:
			calc_attribaddr();
			break;
		case 4:		case 12:		case 20:		case 28:		case 36:		case 44:		case 52:		case 60:
		case 68:		case 76:		case 84:		case 92:		case 100:	case 108:	case 116:	case 124:
		case 132:	case 140:	case 148:	case 156:	case 164:	case 172:	case 180:	case 188:
		case 196:	case 204:	case 212:	case 220:	case 228:	case 236:	case 244:	case 252:
			fetch_attribbyte();
			break;

		//pattern table byte 0 read
		case 5:		case 13:		case 21:		case 29:		case 37:		case 45:		case 53:		case 61:
		case 69:		case 77:		case 85:		case 93:		case 101:	case 109:	case 117:	case 125:
		case 133:	case 141:	case 149:	case 157:	case 165:	case 173:	case 181:	case 189:
		case 197:	case 205:	case 213:	case 221:	case 229:	case 237:	case 245:	case 253:
			calc_pattern0addr();
			break;
		case 6:		case 14:		case 22:		case 30:		case 38:		case 46:		case 54:		case 62:
		case 70:		case 78:		case 86:		case 94:		case 102:	case 110:	case 118:	case 126:
		case 134:	case 142:	case 150:	case 158:	case 166:	case 174:	case 182:	case 190:
		case 198:	case 206:	case 214:	case 222:	case 230:	case 238:	case 246:	case 254:
			fetch_patternbyte0();
			break;

		//pattern table byte 1 read
		case 7:		case 15:		case 23:		case 31:		case 39:		case 47:		case 55:		case 63:
		case 71:		case 79:		case 87:		case 95:		case 103:	case 111:	case 119:	case 127:
		case 135:	case 143:	case 151:	case 159:	case 167:	case 175:	case 183:	case 191:
		case 199:	case 207:	case 215:	case 223:	case 231:	case 239:	case 247:	case 255:
			calc_pattern1addr();
			break;
		case 8:		case 16:		case 24:		case 32:		case 40:		case 48:		case 56:		case 64:
		case 72:		case 80:		case 88:		case 96:		case 104:	case 112:	case 120:	case 128:
		case 136:	case 144:	case 152:	case 160:	case 168:	case 176:	case 184:	case 192:
		case 200:	case 208:	case 216:	case 224:	case 232:	case 240:	case 248:	case 256:
			fetch_patternbyte1();
			inc_hscroll();
			if(LINECYCLES == 256) {
				if(SCANLINE == 21)
					video_startframe();
				if(CONTROL1 & 0x8)
					drawtileline();
				else
					blankline();
				drawspriteline();
				video_updateline(SCANLINE - 21,nes.ppu.linebuffer + nes.ppu.scrollx);
				if(SCANLINE == 260)
					video_endframe();
				inc_vscroll();
#ifdef QUICK_SPRITES
				if(CONTROL1 & 0x10)
					quick_process_sprites();
#endif
			}
			break;

		//garbage nametable address calc
		case 257:
			update_hscroll();
		case 265:	case 273:	case 281:	case 289:	case 297:	case 305:	case 313:
		//	calc_ntaddr();
			break;

		//garbage nametable fetch
		case 258:	case 266:	case 274:	case 282:	case 290:	case 298:	case 306:	case 314:
			break;

		//garbage attribute address calc
		case 259:	case 267:	case 275:	case 283:	case 291:	case 299:	case 307:	case 315:
		//	calc_attribaddr();
			break;

		//garbage attribute fetch
		case 260:	case 268:	case 276:	case 284:	case 292:	case 300:	case 308:	case 316:
			break;

		//calculate address of sprite pattern low
		case 261:	case 269:	case 277:	case 285:	case 293:	case 301:	case 309:	case 317:
		//	calc_spritepattern0addr();
			break;

		//fetch sprite pattern low
		case 262:	case 270:	case 278:	case 286:	case 294:	case 302:	case 310:	case 318:
			break;

		//calculate address of sprite pattern high
		case 263:	case 271:	case 279:	case 287:	case 295:	case 303:	case 311:	case 319:
		//	calc_spritepattern1addr();
			break;

		//fetch sprite pattern high
		
		case 264:	case 272:	case 280:	case 288:	case 296:	case 304:	case 312:	case 320:
			break;

		//nametable byte for next scanline
		case 321:	case 329:
			calc_ntaddr();
			break;
		case 322:	case 330:
			fetch_ntbyte();
			break;

		//attribute byte for next scanline
		case 323:	case 331:
			calc_attribaddr();
			break;
		case 324:	case 332:
			fetch_attribbyte();
			break;

		//pattern low byte tile
		case 325:	case 333:
			calc_pattern0addr();
			break;
		case 326:	case 334:
			fetch_patternbyte0();
			break;

		//pattern high byte tile
		case 327:	case 335:
			calc_pattern1addr();
			break;
		case 328:	case 336:
			fetch_patternbyte1();
			inc_hscroll();
			break;

		//garbage nametable fetches
		case 337:	case 339:
			calc_ntaddr();
			break;
		case 338:	case 340:
			fetch_ntbyte();
			break;

		default:
//			log_printf("scanline_visible:  unhandled cycle %d\n",LINECYCLES);
			break;
	}
#ifndef QUICK_SPRITES
	process_sprites();
#endif
}

static INLINE void scanline_0()
{
	if(LINECYCLES == 0) {
		set_nmi();
//		log_printf("scanline_0:  frame %d, scanline %d, cycle %d:  setting VBLANK flag\n",FRAMES,SCANLINE,LINECYCLES);
	}
	//last cycle
//	if(LINECYCLES == 340) {
//		log_printf("ppu_step:  scanline %d, cycle %d:  setting VBLANK\n",SCANLINE,LINECYCLES);
//	}
}

static INLINE void scanline_261()
{
}

void ppu_step()
{
	switch(SCANLINE) {
		//vblank
		case 0:
			scanline_0();
			break;
		case 1:		case 2:		case 3:		case 4:		case 5:		case 6:		case 7:		case 8:		case 9:		case 10:
		case 11:		case 12:		case 13:		case 14:		case 15:		case 16:		case 17:		case 18:		case 19:
			break;

		//pre-render scanline
		case 20:
			scanline_prerender();
			break;

		//visible scanlines
		case 21:		case 22:		case 23:		case 24:		case 25:		case 26:		case 27:		case 28:		case 29:		case 30:
		case 31:		case 32:		case 33:		case 34:		case 35:		case 36:		case 37:		case 38:		case 39:		case 40:
		case 41:		case 42:		case 43:		case 44:		case 45:		case 46:		case 47:		case 48:		case 49:		case 50:
		case 51:		case 52:		case 53:		case 54:		case 55:		case 56:		case 57:		case 58:		case 59:		case 60:
		case 61:		case 62:		case 63:		case 64:		case 65:		case 66:		case 67:		case 68:		case 69:		case 70:
		case 71:		case 72:		case 73:		case 74:		case 75:		case 76:		case 77:		case 78:		case 79:		case 80:
		case 81:		case 82:		case 83:		case 84:		case 85:		case 86:		case 87:		case 88:		case 89:		case 90:
		case 91:		case 92:		case 93:		case 94:		case 95:		case 96:		case 97:		case 98:		case 99:		case 100:
		case 101:	case 102:	case 103:	case 104:	case 105:	case 106:	case 107:	case 108:	case 109:	case 110:
		case 111:	case 112:	case 113:	case 114:	case 115:	case 116:	case 117:	case 118:	case 119:	case 120:
		case 121:	case 122:	case 123:	case 124:	case 125:	case 126:	case 127:	case 128:	case 129:	case 130:
		case 131:	case 132:	case 133:	case 134:	case 135:	case 136:	case 137:	case 138:	case 139:	case 140:
		case 141:	case 142:	case 143:	case 144:	case 145:	case 146:	case 147:	case 148:	case 149:	case 150:
		case 151:	case 152:	case 153:	case 154:	case 155:	case 156:	case 157:	case 158:	case 159:	case 160:
		case 161:	case 162:	case 163:	case 164:	case 165:	case 166:	case 167:	case 168:	case 169:	case 170:
		case 171:	case 172:	case 173:	case 174:	case 175:	case 176:	case 177:	case 178:	case 179:	case 180:
		case 181:	case 182:	case 183:	case 184:	case 185:	case 186:	case 187:	case 188:	case 189:	case 190:
		case 191:	case 192:	case 193:	case 194:	case 195:	case 196:	case 197:	case 198:	case 199:	case 200:
		case 201:	case 202:	case 203:	case 204:	case 205:	case 206:	case 207:	case 208:	case 209:	case 210:
		case 211:	case 212:	case 213:	case 214:	case 215:	case 216:	case 217:	case 218:	case 219:	case 220:
		case 221:	case 222:	case 223:	case 224:	case 225:	case 226:	case 227:	case 228:	case 229:	case 230:
		case 231:	case 232:	case 233:	case 234:	case 235:	case 236:	case 237:	case 238:	case 239:	case 240:
		case 241:	case 242:	case 243:	case 244:	case 245:	case 246:	case 247:	case 248:	case 249:	case 250:
		case 251:	case 252:	case 253:	case 254:	case 255:	case 256:	case 257:	case 258:	case 259:	case 260:
			scanline_visible();
			break;

		//wasted line
		case 261:
			scanline_261();
			break;

		default:
			log_printf("ppu_step:  unhandled scanline %d (bug!)\n",SCANLINE);
			break;
	}
	if(LINECYCLES & 1)
		nes.mapper->cycle();
	inc_linecycles();
}
