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
#include "misc/log.h"

typedef struct {
	u64 line;				//cache line data
	u8 attr;					//attrib bits
	u8 x;						//x coord
	u8 flags;				//flags
	u8 tile;					//sprite tile index
	u8 sprline;				//line of sprite bitmap to draw
} sprtemp_t;				//sprite temp entry

//todo: this needs to be moved to the ppu struct
static sprtemp_t sprtemp[8];
static sprtemp_t *spr0 = 0;

#include "step/calc.c"
#include "step/fetch.c"
#include "step/scroll.c"
#include "step/misc.c"
#include "step/sprite.c"
#include "step/draw.c"

static INLINE void scanline_prerender()
{
	/* There are 2 conditions that update all 5 PPU scroll counters with the
	contents of the latches adjacent to them. The first is after a write to
	2006/2. The second, is at the beginning of scanline 20, when the PPU starts
	rendering data for the first time in a frame (this update won't happen if
	all rendering is disabled via 2001.3 and 2001.4). */
	switch(LINECYCLES) {

		//the idle cycle
		case 0:
			nes->ppu.rendering = 1;
			clear_sp0hit_flag();
			break;

		//nametable byte read
		case 1:
			//clear ppu status register
			clear_nmi_flag();
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

		//attribute table byte calc
		case 3:
			clear_nmi_line();
		case 11:		case 19:		case 27:		case 35:		case 43:		case 51:		case 59:
		case 67:		case 75:		case 83:		case 91:		case 99:		case 107:	case 115:	case 123:
		case 131:	case 139:	case 147:	case 155:	case 163:	case 171:	case 179:	case 187:
		case 195:	case 203:	case 211:	case 219:	case 227:	case 235:	case 243:	case 251:
			calc_ataddr();
			break;

		//attribute table byte fetch
		case 4:		fetch_atbyte(2);		break;
		case 12:		fetch_atbyte(3);		break;
		case 20:		fetch_atbyte(4);		break;
		case 28:		fetch_atbyte(5);		break;
		case 36:		fetch_atbyte(6);		break;
		case 44:		fetch_atbyte(7);		break;
		case 52:		fetch_atbyte(8);		break;
		case 60:		fetch_atbyte(9);		break;
		case 68:		fetch_atbyte(10);		break;
		case 76:		fetch_atbyte(11);		break;
		case 84:		fetch_atbyte(12);		break;
		case 92:		fetch_atbyte(13);		break;
		case 100:	fetch_atbyte(14);		break;
		case 108:	fetch_atbyte(15);		break;
		case 116:	fetch_atbyte(16);		break;
		case 124:	fetch_atbyte(17);		break;
		case 132:	fetch_atbyte(18);		break;
		case 140:	fetch_atbyte(19);		break;
		case 148:	fetch_atbyte(20);		break;
		case 156:	fetch_atbyte(21);		break;
		case 164:	fetch_atbyte(22);		break;
		case 172:	fetch_atbyte(23);		break;
		case 180:	fetch_atbyte(24);		break;
		case 188:	fetch_atbyte(25);		break;
		case 196:	fetch_atbyte(26);		break;
		case 204:	fetch_atbyte(27);		break;
		case 212:	fetch_atbyte(28);		break;
		case 220:	fetch_atbyte(29);		break;
		case 228:	fetch_atbyte(30);		break;
		case 236:	fetch_atbyte(31);		break;
		case 244:	fetch_atbyte(32);		break;
		case 252:	fetch_atbyte(33);		break;

		//pattern table byte 0 calc
		case 5:		case 13:		case 21:		case 29:		case 37:		case 45:		case 53:		case 61:
		case 69:		case 77:		case 85:		case 93:		case 101:	case 109:	case 117:	case 125:
		case 133:	case 141:	case 149:	case 157:	case 165:	case 173:	case 181:	case 189:
		case 197:	case 205:	case 213:	case 221:	case 229:	case 237:	case 245:	case 253:
			calc_pt0addr();
			break;

		//pattern table byte 0 fetch
		case 6:		fetch_pt0byte(2);		break;
		case 14:		fetch_pt0byte(3);		break;
		case 22:		fetch_pt0byte(4);		break;
		case 30:		fetch_pt0byte(5);		break;
		case 38:		fetch_pt0byte(6);		break;
		case 46:		fetch_pt0byte(7);		break;
		case 54:		fetch_pt0byte(8);		break;
		case 62:		fetch_pt0byte(9);		break;
		case 70:		fetch_pt0byte(10);	break;
		case 78:		fetch_pt0byte(11);	break;
		case 86:		fetch_pt0byte(12);	break;
		case 94:		fetch_pt0byte(13);	break;
		case 102:	fetch_pt0byte(14);	break;
		case 110:	fetch_pt0byte(15);	break;
		case 118:	fetch_pt0byte(16);	break;
		case 126:	fetch_pt0byte(17);	break;
		case 134:	fetch_pt0byte(18);	break;
		case 142:	fetch_pt0byte(19);	break;
		case 150:	fetch_pt0byte(20);	break;
		case 158:	fetch_pt0byte(21);	break;
		case 166:	fetch_pt0byte(22);	break;
		case 174:	fetch_pt0byte(23);	break;
		case 182:	fetch_pt0byte(24);	break;
		case 190:	fetch_pt0byte(25);	break;
		case 198:	fetch_pt0byte(26);	break;
		case 206:	fetch_pt0byte(27);	break;
		case 214:	fetch_pt0byte(28);	break;
		case 222:	fetch_pt0byte(29);	break;
		case 230:	fetch_pt0byte(30);	break;
		case 238:	fetch_pt0byte(31);	break;
		case 246:	fetch_pt0byte(32);	break;
		case 254:	fetch_pt0byte(33);	break;

		//pattern table byte 1 read
		case 7:		case 15:		case 23:		case 31:		case 39:		case 47:		case 55:		case 63:
		case 71:		case 79:		case 87:		case 95:		case 103:	case 111:	case 119:	case 127:
		case 135:	case 143:	case 151:	case 159:	case 167:	case 175:	case 183:	case 191:
		case 199:	case 207:	case 215:	case 223:	case 231:	case 239:	case 247:	case 255:
			calc_pt1addr();
			break;
		case 8:		case 16:		case 24:		case 32:		case 40:		case 48:		case 56:		case 64:
		case 72:		case 80:		case 88:		case 96:		case 104:	case 112:	case 120:	case 128:
		case 136:	case 144:	case 152:	case 160:	case 168:	case 176:	case 184:	case 192:
		case 200:	case 208:	case 216:	case 224:	case 232:	case 240:	case 248://	case 256:
			fetch_pt1byte();
			inc_hscroll();
			break;
		case 256:
			fetch_pt1byte();
			inc_hscroll();
			inc_vscroll();
			break;

		//garbage nametable address calc
		case 257:
#ifdef QUICK_SPRITES
			quick_process_sprites();
#endif
			update_hscroll();
		case 265:	case 273:	case 281:	case 289:	case 297:	case 305:	case 313:
			calc_ntaddr();
			break;

		//garbage nametable fetch
		case 258:	case 266:	case 274:	case 282:	case 290:	case 298:	case 306:	case 314:
			break;

		//garbage attribute address calc
		case 259:	case 267:	case 275:	case 283:	case 291:	case 299:	case 307:	case 315:
			calc_ataddr();
			break;

		//garbage attribute fetch
		case 260:	case 268:	case 276:	case 284:	case 292:	case 300:	case 308:	case 316:
			break;

		//calculate address of sprite pattern low
		case 261:	case 269:	case 277:	case 285:	case 293:	case 301:	case 309:	case 317:
			calc_spt0addr();
			break;

		//fetch sprite pattern low
		case 262:	case 270:	case 278:	case 286:	case 294:	case 302:	case 310:	case 318:
			fetch_spt0byte();
			break;

		//calculate address of sprite pattern high
		case 263:	case 271:	case 279:	case 287:	case 295:	case 303:	case 311:	case 319:
			calc_spt1addr();
			break;

		//fetch sprite pattern high (update scroll registers on cycle 304)
		case 304:
			update_vscroll();
		case 264:
		case 272:
		case 280:
		case 288:
		case 296:
		case 312:
			fetch_spt1byte();
			break;
		case 320:
			fetch_spt1byte();
//			nes->ppu.oamaddr = 0;
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
			calc_ataddr();
			break;
		case 324:
			fetch_atbyte(0);
			break;
		case 332:
			fetch_atbyte(1);
			break;

		//pattern low byte tile
		case 325:
		case 333:
			calc_pt0addr();
			break;
		case 326:
			fetch_pt0byte(0);
			break;
		case 334:
			fetch_pt0byte(1);
			break;

		//pattern high byte tile
		case 327:	case 335:
			calc_pt1addr();
			break;
		case 328:	case 336:
			fetch_pt1byte();
			inc_hscroll();
			break;

		//garbage nametable fetches
		case 337:
			calc_ntaddr();
			break;
		case 338:
			fetch_ntbyte();
			skip_cycle();
			break;
		case 339:
//			calc_ntaddr();
			break;
		case 340:
			fetch_ntbyte();
			break;
	}
#ifndef QUICK_SPRITES
	if(CONTROL1 & 0x10)
		process_sprites();
#endif
}

//scanline 0 with rendering disabled
static INLINE void scanline_prerender_norender()
{
	switch(LINECYCLES) {

		case 0:
			clear_sp0hit_flag();
			break;
		case 1:
			clear_nmi_flag();
			break;
		case 3:
			clear_nmi_line();
			break;
		case 320:
//			nes->ppu.oamaddr = 0;
			break;
		case 338:
//			skip_cycle();
			break;
	}
}

//scanlines 0-239
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
			drawpixel();
			break;
		case 2:		case 10:		case 18:		case 26:		case 34:		case 42:		case 50:		case 58:
		case 66:		case 74:		case 82:		case 90:		case 98:		case 106:	case 114:	case 122:
		case 130:	case 138:	case 146:	case 154:	case 162:	case 170:	case 178:	case 186:
		case 194:	case 202:	case 210:	case 218:	case 226:	case 234:	case 242:	case 250:
			fetch_ntbyte();
			drawpixel();
			break;

		//attribute table byte read
		case 3:		case 11:		case 19:		case 27:		case 35:		case 43:		case 51:		case 59:
		case 67:		case 75:		case 83:		case 91:		case 99:		case 107:	case 115:	case 123:
		case 131:	case 139:	case 147:	case 155:	case 163:	case 171:	case 179:	case 187:
		case 195:	case 203:	case 211:	case 219:	case 227:	case 235:	case 243:	case 251:
			calc_ataddr();
			drawpixel();
			break;

		//attribute table byte fetch
		case 4:		fetch_atbyte(2);		drawpixel();	break;
		case 12:		fetch_atbyte(3);		drawpixel();	break;
		case 20:		fetch_atbyte(4);		drawpixel();	break;
		case 28:		fetch_atbyte(5);		drawpixel();	break;
		case 36:		fetch_atbyte(6);		drawpixel();	break;
		case 44:		fetch_atbyte(7);		drawpixel();	break;
		case 52:		fetch_atbyte(8);		drawpixel();	break;
		case 60:		fetch_atbyte(9);		drawpixel();	break;
		case 68:		fetch_atbyte(10);		drawpixel();	break;
		case 76:		fetch_atbyte(11);		drawpixel();	break;
		case 84:		fetch_atbyte(12);		drawpixel();	break;
		case 92:		fetch_atbyte(13);		drawpixel();	break;
		case 100:	fetch_atbyte(14);		drawpixel();	break;
		case 108:	fetch_atbyte(15);		drawpixel();	break;
		case 116:	fetch_atbyte(16);		drawpixel();	break;
		case 124:	fetch_atbyte(17);		drawpixel();	break;
		case 132:	fetch_atbyte(18);		drawpixel();	break;
		case 140:	fetch_atbyte(19);		drawpixel();	break;
		case 148:	fetch_atbyte(20);		drawpixel();	break;
		case 156:	fetch_atbyte(21);		drawpixel();	break;
		case 164:	fetch_atbyte(22);		drawpixel();	break;
		case 172:	fetch_atbyte(23);		drawpixel();	break;
		case 180:	fetch_atbyte(24);		drawpixel();	break;
		case 188:	fetch_atbyte(25);		drawpixel();	break;
		case 196:	fetch_atbyte(26);		drawpixel();	break;
		case 204:	fetch_atbyte(27);		drawpixel();	break;
		case 212:	fetch_atbyte(28);		drawpixel();	break;
		case 220:	fetch_atbyte(29);		drawpixel();	break;
		case 228:	fetch_atbyte(30);		drawpixel();	break;
		case 236:	fetch_atbyte(31);		drawpixel();	break;
		case 244:	fetch_atbyte(32);		drawpixel();	break;
		case 252:	fetch_atbyte(33);		drawpixel();	break;

		//pattern table byte 0 calc
		case 5:		case 13:		case 21:		case 29:		case 37:		case 45:		case 53:		case 61:
		case 69:		case 77:		case 85:		case 93:		case 101:	case 109:	case 117:	case 125:
		case 133:	case 141:	case 149:	case 157:	case 165:	case 173:	case 181:	case 189:
		case 197:	case 205:	case 213:	case 221:	case 229:	case 237:	case 245:	case 253:
			calc_pt0addr();
			drawpixel();
			break;

		//pattern table byte 0 fetch
		case 6:		fetch_pt0byte(2);		drawpixel();	break;
		case 14:		fetch_pt0byte(3);		drawpixel();	break;
		case 22:		fetch_pt0byte(4);		drawpixel();	break;
		case 30:		fetch_pt0byte(5);		drawpixel();	break;
		case 38:		fetch_pt0byte(6);		drawpixel();	break;
		case 46:		fetch_pt0byte(7);		drawpixel();	break;
		case 54:		fetch_pt0byte(8);		drawpixel();	break;
		case 62:		fetch_pt0byte(9);		drawpixel();	break;
		case 70:		fetch_pt0byte(10);	drawpixel();	break;
		case 78:		fetch_pt0byte(11);	drawpixel();	break;
		case 86:		fetch_pt0byte(12);	drawpixel();	break;
		case 94:		fetch_pt0byte(13);	drawpixel();	break;
		case 102:	fetch_pt0byte(14);	drawpixel();	break;
		case 110:	fetch_pt0byte(15);	drawpixel();	break;
		case 118:	fetch_pt0byte(16);	drawpixel();	break;
		case 126:	fetch_pt0byte(17);	drawpixel();	break;
		case 134:	fetch_pt0byte(18);	drawpixel();	break;
		case 142:	fetch_pt0byte(19);	drawpixel();	break;
		case 150:	fetch_pt0byte(20);	drawpixel();	break;
		case 158:	fetch_pt0byte(21);	drawpixel();	break;
		case 166:	fetch_pt0byte(22);	drawpixel();	break;
		case 174:	fetch_pt0byte(23);	drawpixel();	break;
		case 182:	fetch_pt0byte(24);	drawpixel();	break;
		case 190:	fetch_pt0byte(25);	drawpixel();	break;
		case 198:	fetch_pt0byte(26);	drawpixel();	break;
		case 206:	fetch_pt0byte(27);	drawpixel();	break;
		case 214:	fetch_pt0byte(28);	drawpixel();	break;
		case 222:	fetch_pt0byte(29);	drawpixel();	break;
		case 230:	fetch_pt0byte(30);	drawpixel();	break;
		case 238:	fetch_pt0byte(31);	drawpixel();	break;
		case 246:	fetch_pt0byte(32);	drawpixel();	break;
		case 254:	fetch_pt0byte(33);	drawpixel();	break;

		//pattern table byte 1 read
		case 7:		case 15:		case 23:		case 31:		case 39:		case 47:		case 55:		case 63:
		case 71:		case 79:		case 87:		case 95:		case 103:	case 111:	case 119:	case 127:
		case 135:	case 143:	case 151:	case 159:	case 167:	case 175:	case 183:	case 191:
		case 199:	case 207:	case 215:	case 223:	case 231:	case 239:	case 247:	case 255:
			calc_pt1addr();
			drawpixel();
			break;
		case 8:		case 16:		case 24:		case 32:		case 40:		case 48:		case 56:		case 64:
		case 72:		case 80:		case 88:		case 96:		case 104:	case 112:	case 120:	case 128:
		case 136:	case 144:	case 152:	case 160:	case 168:	case 176:	case 184:	case 192:
		case 200:	case 208:	case 216:	case 224:	case 232:	case 240:	case 248:
			fetch_pt1byte();
			drawpixel();
			inc_hscroll();
			break;
		case 256:
			fetch_pt1byte();
			drawpixel();
			inc_hscroll();
//			video_updateline(SCANLINE,nes->ppu.linebuffer);
			inc_vscroll();
			break;

		//garbage nametable address calc
		case 257:
#ifdef QUICK_SPRITES
			quick_process_sprites();
#endif
			update_hscroll();
		case 265:	case 273:	case 281:	case 289:	case 297:	case 305:	case 313:
			calc_ntaddr();
			break;

		//garbage nametable fetch
		case 258:	case 266:	case 274:	case 282:	case 290:	case 298:	case 306:	case 314:
			break;

		//garbage attribute address calc
		case 259:	case 267:	case 275:	case 283:	case 291:	case 299:	case 307:	case 315:
			calc_ataddr();
			break;

		//garbage attribute fetch
		case 260:	case 268:	case 276:	case 284:	case 292:	case 300:	case 308:	case 316:
			break;

		//calculate address of sprite pattern low
		case 261:	case 269:	case 277:	case 285:	case 293:	case 301:	case 309:	case 317:
			calc_spt0addr();
			break;

		//fetch sprite pattern low
		case 262:	case 270:	case 278:	case 286:	case 294:	case 302:	case 310:	case 318:
			fetch_spt0byte();
			break;

		//calculate address of sprite pattern high
		case 263:	case 271:	case 279:	case 287:	case 295:	case 303:	case 311:	case 319:
			calc_spt1addr();
			break;

		//fetch sprite pattern high
		case 264:	case 272:	case 280:	case 288:	case 296:	case 304:	case 312:
			fetch_spt1byte();
			break;
		case 320:
			fetch_spt1byte();
//			nes->ppu.oamaddr = 0;
#ifdef QUICK_SPRITES
			quick_draw_sprite_line();
#endif
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
			calc_ataddr();
			break;
		case 324:
			fetch_atbyte(0);
			break;
		case 332:
			fetch_atbyte(1);
			break;

		//pattern low byte tile
		case 325:	case 333:
			calc_pt0addr();
			break;
		case 326:
			fetch_pt0byte(0);
			break;
		case 334:
			fetch_pt0byte(1);
			break;

		//pattern high byte tile
		case 327:	case 335:
			calc_pt1addr();
			break;
		case 328:	case 336:
			fetch_pt1byte();
			inc_hscroll();
			break;

		//garbage nametable fetches
		case 337:
			calc_ntaddr();
			break;
		case 338:
			fetch_ntbyte();
			break;
		case 339:
//			calc_ntaddr();
			break;
		case 340:
			fetch_ntbyte();
			break;
	}
#ifndef QUICK_SPRITES
	process_sprites();
	draw_sprites();
#endif
}

//scanlines in visible area with rendering disabled
static INLINE void scanline_visible_norender()
{
	u8 color;

	//visible pixels
	if(LINECYCLES < 256) {

		//palette index 0 (with emphasis bits)
		color = nes->ppu.control1 & 0xE0;

		//the 'background palette hack' (see nesdev wiki)
		if((SCROLL & 0x3F00) == 0x3F00) {
			color |= SCROLL & 0x1F;
		}

		//draw pixel
		video_updatepixel(SCANLINE,LINECYCLES,color);
	}
}

//post render scanline
static INLINE void scanline_postrender()
{
	if(LINECYCLES == 0) {
		nes->ppu.rendering = 0;
	}
}

//first scanline of vblank
static INLINE void scanline_startvblank()
{
	if(LINECYCLES == 0) {
		set_nmi();
	}
}

void ppu_step()
{
	u8 color;
	u32 addr;

//blargg test debug output
/*	if(SCANLINE == 0 && LINECYCLES == 0) {
		u8 *sram = nes->cart->sram.size ? nes->cart->sram.data : 0;

		if(sram && sram[0] < 0x80 && sram[1] == 0xDE && sram[2] == 0xB0 && sram[3] == 0x61) {
			sram[0] = 0xFF;
			log_printf("%s",sram + 4);
		}
	}*/

	//visible scanlines
	if(SCANLINE < 240) {

		//rendering is enabled
		if(CONTROL1 & 0x18) {
			scanline_visible();
		}

		//rendering is turned off
		else {
			scanline_visible_norender();
		}
	}

	//post render scanline
	else if(SCANLINE == 240) {
		scanline_postrender();
	}

	//first scanline of vblank
	else if(SCANLINE == nes->region->vblank_start) {
		scanline_startvblank();
	}

	//last line in the frame
	else if(SCANLINE == nes->region->end_line) {
		if(CONTROL1 & 0x18)
			scanline_prerender();
		else
			scanline_prerender_norender();
	}


	if(LINECYCLES & 1) {
		if(IOMODE) {
			addr = IOADDR & 0x3FFF;

			//call to ppucycle
			if(IOMODE >= 5 && nes->ppu.rendering == 0) {
				nes->ppu.busaddr = addr;
				nes->mapper->ppucycle();
			}

			//perform delayed write
			else if(IOMODE == 2) {
				if(nes->ppu.rendering == 0)
					ppu_memwrite(addr,IODATA);
			}

			//perform delayed read
			else if(IOMODE == 1) {
				IOMODE++;
				if(nes->ppu.rendering == 0) {
					nes->ppu.latch = ppu_memread(addr);
				}
			}
			IOMODE -= 2;
		}
		if(nes->ppu.rendering == 0) {
			if(IOMODE == 0) {
				nes->ppu.busaddr = SCROLL;
				nes->mapper->ppucycle();
			}
		}
		else {
			nes->mapper->ppucycle();
		}
	}
	inc_linecycles();
}
