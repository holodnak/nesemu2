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

static INLINE void drawtileline()
{
	int i,line = SCANLINE - 21;
	u64 *scr64 = (u64*)nes.ppu.linebuffer;
	cache_t *cacheptr = nes.ppu.cachedata;
	u8 *attribptr = nes.ppu.attribdata;

	for(i=0;i<33;i++) {
		*scr64++ = (*attribptr++ * 0x0404040404040404LL) + *cacheptr++;
	}
	video_updateline(SCANLINE - 21,nes.ppu.linebuffer + nes.ppu.scrollx);
}

static INLINE void fetch_ntbyte()
{
	if(CONTROL1 & 0x08)
		nes.ppu.ntbyte = ppu_memread(0x2000 | (SCROLL & 0xFFF));
}

static INLINE void fetch_attribbyte()
{
	u32 tmp;

	if(CONTROL1 & 0x08) {
#ifdef CACHE_ATTRIB
		tmp = SCROLL & 0xFFF;
		nes.ppu.attribdata[nes.ppu.fetchpos] = nes.ppu.attribpages[tmp >> 10][tmp & 0x3FF];
#else
		tmp = SCROLL & 0x3C00;
		tmp += 0x3C0 + ((SCROLL >> 2) & 7) + (((SCROLL >> (2 + 5)) & 7) << 3);
		tmp = ((ppu_memread(tmp | 0x2000) >> (((SCROLL & 2) | (((SCROLL >> 5) & 2) << 1)))) & 3);
		nes.ppu.attribdata[nes.ppu.fetchpos] = tmp;
#endif
	}
}

static INLINE void fetch_patternbyte0()
{
	if(CONTROL1 & 0x08) {
		cache_t *cache,pixels;

		nes.ppu.tiledataaddr = (CONTROL0 & 0x10) << 8;
		nes.ppu.tiledataaddr += nes.ppu.ntbyte * 16;
		nes.ppu.tiledataaddr += SCROLL >> 12;
		nes.ppu.tiledata[0][nes.ppu.fetchpos] = ppu_memread(nes.ppu.tiledataaddr);

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
		nes.ppu.tiledata[1][nes.ppu.fetchpos] = ppu_memread(nes.ppu.tiledataaddr + 8);
		nes.ppu.fetchpos++;
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
}

static INLINE void inc_vscroll()
{
	if(CONTROL1 & 0x18) {
		//update y coordinate
		if((SCROLL >> 12) == 7) {
			int n;

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

static INLINE void scanline_20()
{
	/* There are 2 conditions that update all 5 PPU scroll counters with the
	contents of the latches adjacent to them. The first is after a write to
	2006/2. The second, is at the beginning of scanline 20, when the PPU starts
	rendering data for the first time in a frame (this update won't happen if
	all rendering is disabled via 2001.3 and 2001.4). */
/*	if(LINECYCLES == 0) {
		if((FRAMES & 1) && (CONTROL1 & 0x18)) {
			LINECYCLES++;
			STATUS = 0;
		}
	}
	else if(LINECYCLES == 1) {
		STATUS = 0;
	}
	else if(LINECYCLES >= 280 && LINECYCLES <= 304) {
		if(CONTROL1 & 0x18)
			SCROLL = TMPSCROLL;
	}*/
	switch(LINECYCLES) {
		//the idle cycle
		case 0:
//			LINECYCLES++;
			break;

		//nametable byte read
		case 1:
			STATUS = 0;
		case 9:		case 17:		case 25:		case 33:		case 41:		case 49:		case 57:
		case 65:		case 73:		case 81:		case 89:		case 97:		case 105:	case 113:	case 121:
		case 129:	case 137:	case 145:	case 153:	case 161:	case 169:	case 177:	case 185:
		case 193:	case 201:	case 209:	case 217:	case 225:	case 233:	case 241:	case 249:
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
			break;
		case 8:		case 16:		case 24:		case 32:		case 40:		case 48:		case 56:		case 64:
		case 72:		case 80:		case 88:		case 96:		case 104:	case 112:	case 120:	case 128:
		case 136:	case 144:	case 152:	case 160:	case 168:	case 176:	case 184:	case 192:
		case 200:	case 208:	case 216:	case 224:	case 232:	case 240:	case 248:	case 256:
			fetch_patternbyte1();
			inc_hscroll();
			if(LINECYCLES == 256) {
				inc_vscroll();
			}
			break;
		case 257:
			update_hscroll();
			break;

		case 280:	case 281:	case 282:	case 283:	case 284:	case 285:	case 286:	case 287:
		case 288:	case 289:	case 290:	case 291:	case 292:	case 293:	case 294:	case 295:
		case 296:	case 297:	case 298:	case 299:	case 300:	case 301:	case 302:	case 303:
		case 304:
			if(CONTROL1 & 0x18)
				SCROLL = TMPSCROLL;
			break;
	/* Memory fetch phase 161 thru 168
	-------------------------------
	1. Name table byte
	2. Attribute table byte
	3. Pattern table bitmap #0 (for next scanline)
	4. Pattern table bitmap #1 (for next scanline) */
		case 321:	case 329:
			break;
		case 322:	case 330:
			fetch_ntbyte();
			break;

		case 323:	case 331:
			break;
		case 324:	case 332:
			fetch_attribbyte();
			break;

		case 325:	case 333:
			break;
		case 326:	case 334:
			fetch_patternbyte0();
			break;

		case 327:	case 335:
			break;
		case 328:	case 336:
			fetch_patternbyte1();
			inc_hscroll();
			break;

		case 337:
		case 339:
			break;
		case 338:
		case 340:
			fetch_ntbyte();
			break;

		default:
//			log_printf("scanline_20:  unhandled cycle %d\n",LINECYCLES);
			break;
	}
}

//scanlines 21-260
static INLINE void scanline_visible()
{
	switch(LINECYCLES) {
		//the idle cycle
		case 0:
			if((SCANLINE == 21) && (FRAMES & 1) && (CONTROL1 & 0x18))
				inc_linecycles();
			else
				break;

		//nametable byte read
		case 1:		case 9:		case 17:		case 25:		case 33:		case 41:		case 49:		case 57:
		case 65:		case 73:		case 81:		case 89:		case 97:		case 105:	case 113:	case 121:
		case 129:	case 137:	case 145:	case 153:	case 161:	case 169:	case 177:	case 185:
		case 193:	case 201:	case 209:	case 217:	case 225:	case 233:	case 241:	case 249:
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
				drawtileline();
				if(SCANLINE == 260)
					video_endframe();
				inc_vscroll();
			}
			break;
		case 257:
			update_hscroll();
			break;

	/* Memory fetch phase 161 thru 168
	-------------------------------
	1. Name table byte
	2. Attribute table byte
	3. Pattern table bitmap #0 (for next scanline)
	4. Pattern table bitmap #1 (for next scanline) */
		case 321:	case 329:
			break;
		case 322:	case 330:
			fetch_ntbyte();
			break;

		case 323:	case 331:
			break;
		case 324:	case 332:
			fetch_attribbyte();
			break;

		case 325:	case 333:
			break;
		case 326:	case 334:
			fetch_patternbyte0();
			break;

		case 327:	case 335:
			break;
		case 328:	case 336:
			fetch_patternbyte1();
			inc_hscroll();
			break;

		case 337:
		case 339:
			break;
		case 338:
		case 340:
			fetch_ntbyte();
			break;

		default:
//			log_printf("scanline_visible:  unhandled cycle %d\n",LINECYCLES);
			break;
	}
	if(LINECYCLES > 0 && LINECYCLES <= 64)
		nes.ppu.oam2[(LINECYCLES - 1) / 2] = 0xFF;
}

static INLINE void scanline_261()
{
	if(LINECYCLES == 1) {
		STATUS |= 0x80;
		if(CONTROL0 & 0x80)
			cpu_set_nmi(1);
	}
	//last cycle
//	if(LINECYCLES == 340) {
//		log_printf("ppu_step:  scanline %d, cycle %d:  setting VBLANK\n",SCANLINE,LINECYCLES);
//	}
}

void ppu_step()
{
	switch(SCANLINE) {
		//vblank
		case 0:		case 1:		case 2:		case 3:		case 4:		case 5:		case 6:		case 7:		case 8:		case 9:
		case 10:		case 11:		case 12:		case 13:		case 14:		case 15:		case 16:		case 17:		case 18:		case 19:
			break;

		//dummy scanline
		case 20:
			scanline_20();
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

		case 261:
			scanline_261();
			break;

		default:
			log_printf("ppu_step:  unhandled scanline %d (bug!)\n",SCANLINE);
			break;
	}
	inc_linecycles();
}
