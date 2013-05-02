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
	} sprtemp_t;			//sprite temp entry

static sprtemp_t sprtemp[8];

#include "step/calc.c"
#include "step/fetch.c"
#include "step/scroll.c"
#include "step/misc.c"
#include "step/draw.c"
#include "step/sprite.c"

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
			calc_ataddr();
			break;
		case 4:		case 12:		case 20:		case 28:		case 36:		case 44:		case 52:		case 60:
		case 68:		case 76:		case 84:		case 92:		case 100:	case 108:	case 116:	case 124:
		case 132:	case 140:	case 148:	case 156:	case 164:	case 172:	case 180:	case 188:
		case 196:	case 204:	case 212:	case 220:	case 228:	case 236:	case 244:	case 252:
			fetch_atbyte();
			break;

		//pattern table byte 0 read
		case 5:		case 13:		case 21:		case 29:		case 37:		case 45:		case 53:		case 61:
		case 69:		case 77:		case 85:		case 93:		case 101:	case 109:	case 117:	case 125:
		case 133:	case 141:	case 149:	case 157:	case 165:	case 173:	case 181:	case 189:
		case 197:	case 205:	case 213:	case 221:	case 229:	case 237:	case 245:	case 253:
			calc_pt0addr();
			break;
		case 6:		case 14:		case 22:		case 30:		case 38:		case 46:		case 54:		case 62:
		case 70:		case 78:		case 86:		case 94:		case 102:	case 110:	case 118:	case 126:
		case 134:	case 142:	case 150:	case 158:	case 166:	case 174:	case 182:	case 190:
		case 198:	case 206:	case 214:	case 222:	case 230:	case 238:	case 246:	case 254:
			fetch_pt0byte();
			break;

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
			update_scroll();
		case 264:	case 272:	case 280:	case 288:	case 296:	case 312:	case 320:
			fetch_spt1byte();
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
		case 324:	case 332:
			fetch_atbyte();
			break;

		//pattern low byte tile
		case 325:	case 333:
			calc_pt0addr();
			break;
		case 326:	case 334:
			fetch_pt0byte();
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
		case 337:	case 339:
			calc_ntaddr();
			break;
		case 338:	
			fetch_ntbyte();
			break;
		case 340:
			fetch_ntbyte();
			skip_cycle();
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
			mask_bg();
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
			calc_ataddr();
			break;
		case 4:		case 12:		case 20:		case 28:		case 36:		case 44:		case 52:		case 60:
		case 68:		case 76:		case 84:		case 92:		case 100:	case 108:	case 116:	case 124:
		case 132:	case 140:	case 148:	case 156:	case 164:	case 172:	case 180:	case 188:
		case 196:	case 204:	case 212:	case 220:	case 228:	case 236:	case 244:	case 252:
			fetch_atbyte();
			break;

		//pattern table byte 0 read
		case 5:		case 13:		case 21:		case 29:		case 37:		case 45:		case 53:		case 61:
		case 69:		case 77:		case 85:		case 93:		case 101:	case 109:	case 117:	case 125:
		case 133:	case 141:	case 149:	case 157:	case 165:	case 173:	case 181:	case 189:
		case 197:	case 205:	case 213:	case 221:	case 229:	case 237:	case 245:	case 253:
			calc_pt0addr();
			break;
		case 6:		case 14:		case 22:		case 30:		case 38:		case 46:		case 54:		case 62:
		case 70:		case 78:		case 86:		case 94:		case 102:	case 110:	case 118:	case 126:
		case 134:	case 142:	case 150:	case 158:	case 166:	case 174:	case 182:	case 190:
		case 198:	case 206:	case 214:	case 222:	case 230:	case 238:	case 246:	case 254:
			fetch_pt0byte();
			break;

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
		case 200:	case 208:	case 216:	case 224:	case 232:	case 240:	case 248:
			fetch_pt1byte();
			inc_hscroll();
			break;
		case 256:
			fetch_pt1byte();
			inc_hscroll();
			update_line();
//			log_printf("sprites %s (%s)\n",(CONTROL1 & 0x10) ? "enabled" : "disabled",(CONTROL1 & 0x10) ? "8x16" : "8x8");
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
		
		case 264:	case 272:	case 280:	case 288:	case 296:	case 304:	case 312:	case 320:
			fetch_spt1byte();
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
		case 324:	case 332:
			fetch_atbyte();
			break;

		//pattern low byte tile
		case 325:	case 333:
			calc_pt0addr();
			break;
		case 326:	case 334:
			fetch_pt0byte();
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

static INLINE void scanline_startvblank()
{
	if(LINECYCLES == 0) {
		set_nmi();
//		log_printf("scanline_0:  frame %d, scanline %d, cycle %d:  setting VBLANK flag\n",FRAMES,SCANLINE,LINECYCLES);
	}
}

static INLINE void scanline_postrender()
{
	if(LINECYCLES == 0) {
//		printf("%s\n",nes.cart->sram.data+4);
	}
}

void ppu_step()
{
	switch(SCANLINE) {
		case 0:		case 1:		case 2:		case 3:		case 4:		case 5:		case 6:		case 7:		case 8:		case 9:
		case 10:		case 11:		case 12:		case 13:		case 14:		case 15:		case 16:		case 17:		case 18:		case 19:
		case 20:		case 21:		case 22:		case 23:		case 24:		case 25:		case 26:		case 27:		case 28:		case 29:
		case 30:		case 31:		case 32:		case 33:		case 34:		case 35:		case 36:		case 37:		case 38:		case 39:
		case 40:		case 41:		case 42:		case 43:		case 44:		case 45:		case 46:		case 47:		case 48:		case 49:
		case 50:		case 51:		case 52:		case 53:		case 54:		case 55:		case 56:		case 57:		case 58:		case 59:
		case 60:		case 61:		case 62:		case 63:		case 64:		case 65:		case 66:		case 67:		case 68:		case 69:
		case 70:		case 71:		case 72:		case 73:		case 74:		case 75:		case 76:		case 77:		case 78:		case 79:
		case 80:		case 81:		case 82:		case 83:		case 84:		case 85:		case 86:		case 87:		case 88:		case 89:
		case 90:		case 91:		case 92:		case 93:		case 94:		case 95:		case 96:		case 97:		case 98:		case 99:
		case 100:	case 101:	case 102:	case 103:	case 104:	case 105:	case 106:	case 107:	case 108:	case 109:
		case 110:	case 111:	case 112:	case 113:	case 114:	case 115:	case 116:	case 117:	case 118:	case 119:
		case 120:	case 121:	case 122:	case 123:	case 124:	case 125:	case 126:	case 127:	case 128:	case 129:
		case 130:	case 131:	case 132:	case 133:	case 134:	case 135:	case 136:	case 137:	case 138:	case 139:
		case 140:	case 141:	case 142:	case 143:	case 144:	case 145:	case 146:	case 147:	case 148:	case 149:
		case 150:	case 151:	case 152:	case 153:	case 154:	case 155:	case 156:	case 157:	case 158:	case 159:
		case 160:	case 161:	case 162:	case 163:	case 164:	case 165:	case 166:	case 167:	case 168:	case 169:
		case 170:	case 171:	case 172:	case 173:	case 174:	case 175:	case 176:	case 177:	case 178:	case 179:
		case 180:	case 181:	case 182:	case 183:	case 184:	case 185:	case 186:	case 187:	case 188:	case 189:
		case 190:	case 191:	case 192:	case 193:	case 194:	case 195:	case 196:	case 197:	case 198:	case 199:
		case 200:	case 201:	case 202:	case 203:	case 204:	case 205:	case 206:	case 207:	case 208:	case 209:
		case 210:	case 211:	case 212:	case 213:	case 214:	case 215:	case 216:	case 217:	case 218:	case 219:
		case 220:	case 221:	case 222:	case 223:	case 224:	case 225:	case 226:	case 227:	case 228:	case 229:
		case 230:	case 231:	case 232:	case 233:	case 234:	case 235:	case 236:	case 237:	case 238:	case 239:
			if(CONTROL1 & 0x18)
				scanline_visible();
			else {
				nes.ppu.busaddr = SCROLL; //hack
				//kludge
				if(LINECYCLES <= 256)
					nes.ppu.fetchpos = (LINECYCLES - 1 / 8) + 2;
				else if(LINECYCLES < 320)
					nes.ppu.fetchpos = 0;
				else
					nes.ppu.fetchpos = 1;
				//end kludge
				if(LINECYCLES == 256) {
					blankline();
					video_updateline(SCANLINE,nes.ppu.linebuffer);
				}
			}
			break;
		case 240:
			scanline_postrender();
			break;
		case 241:
			scanline_startvblank();
			break;
		case 242:	case 243:	case 244:	case 245:	case 246:	case 247:	case 248:	case 249:	case 250:	case 251:
		case 252:	case 253:	case 254:	case 255:	case 256:	case 257:	case 258:	case 259:	case 260:
			break;
		case 261:
			scanline_prerender();
			break;
	}
	if(LINECYCLES & 1)
		nes.mapper->ppucycle();
	inc_linecycles();
}
