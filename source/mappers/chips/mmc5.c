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

#include "mappers/mapperinc.h"
#include "mappers/chips/mmc5.h"
#include "mappers/sound/s_MMC5.h"

static apu_external_t drip = {
	MMC5sound_Load,
	MMC5sound_Unload,
	MMC5sound_Reset,
	MMC5sound_Get,
	0
};

static u8 prg[4],chrhi,prgram,mirror;
static u16 chra[8],chrb[4];
static u8 prgmode,chrmode,exrammode,chrselect;
static u8 prgprotect[2];
static u8 multiply[2];
static u8 filltile,fillattrib;
static u8 irqtarget,irqenable,irqstatus,irqcounter;
static u8 *exram;
static void (*sync)();
static readfunc_t ppuread;

u8 mmc5_ppuread(u32 addr)
{
	return(ppuread(addr));
}

u8 mmc5_ppureadfill(u32 addr)
{
//	log_printf("mmc5:  reading nt fill data!\n");
	addr &= 0x3FF;
	if(addr < 0x3C0)
		return(filltile);
	return(fillattrib);
}

void mmc5_setprg(int size,int bank,int page)
{
	//rom mapped here
	if(page & 0x80) {
		page &= 0x7F;
		if(size == 32) {
			mem_setprg32(bank,page >> 2);
		}
		else if(size == 16) {
			mem_setprg16(bank,page >> 1);
		}
		else { //8kb
			mem_setprg8(bank,page);
		}
	}
	else {
//		log_printf("mmc5:  mapping %dkb prg ram sram to %04X\n",size,bank*0x1000);
		if(size == 16) {
			mem_setsram16(bank,page);
		}
		else if(size == 8) {
			mem_setsram8(bank,page);
		}
		else
			log_printf("mmc5:  mapping %dkb prg ram?  no!\n",size);
	}
}

void mmc5_setmirror(int bank,int data)
{
	bank += 8;
	switch(data) {
		case 0:
		case 1:
			mem_setnt1(bank,data);
			mem_setppureadfunc(bank,0);
			break;
		case 2:
			//map in exram as nametable data
			if(exrammode < 2) {
//				log_printf("mmc5:  exram mapped into ppu bank %d!\n",bank);
				mem_setppureadptr(bank,exram);
				mem_setppuwriteptr(bank,exram);
			}
			//map in disabled exram
			else {
//				log_printf("mmc5:  disabled exram mapped into ppu bank %d!\n",bank);
				mem_setppureadptr(bank,exram + 0xC00);
				mem_setppuwriteptr(bank,exram + 0xC00);
			}
			mem_setppureadfunc(bank,0);
			break;
		case 3:
			mem_unsetppu1(bank);
			mem_setppureadfunc(bank,mmc5_ppureadfill);
			break;
	}
}

void mmc5_syncprg()
{
	mem_setsram8(6,prgram);
	switch(prgmode) {
		case 0:
			mmc5_setprg(32,0x8,prg[3] | 0x80);
			break;
		case 1:
			mmc5_setprg(16,0x8,prg[1]);
			mmc5_setprg(16,0xC,prg[3] | 0x80);
			break;
		case 2:
			mmc5_setprg(16,0x8,prg[1]);
			mmc5_setprg(8,0xC,prg[2]);
			mmc5_setprg(8,0xE,prg[3] | 0x80);
			break;
		case 3:
			mmc5_setprg(8,0x8,prg[0]);
			mmc5_setprg(8,0xA,prg[1]);
			mmc5_setprg(8,0xC,prg[2]);
			mmc5_setprg(8,0xE,prg[3] | 0x80);
			break;
	}
}

void mmc5_syncchra()
{
	switch(chrmode) {
		case 0:
			mem_setchr8(0,chra[7]);
			break;
		case 1:
			mem_setchr4(0,chra[3]);
			mem_setchr4(4,chra[7]);
			break;
		case 2:
			mem_setchr2(0,chra[1]);
			mem_setchr2(2,chra[3]);
			mem_setchr2(4,chra[5]);
			mem_setchr2(6,chra[7]);
			break;
		case 3:
			mem_setchr1(0,chra[0]);
			mem_setchr1(1,chra[1]);
			mem_setchr1(2,chra[2]);
			mem_setchr1(3,chra[3]);
			mem_setchr1(4,chra[4]);
			mem_setchr1(5,chra[5]);
			mem_setchr1(6,chra[6]);
			mem_setchr1(7,chra[7]);
			break;
	}
}

void mmc5_syncchrb()
{
	switch(chrmode) {
		case 0:
			mem_setchr8(0,chrb[3]);
			break;
		case 1:
			mem_setchr4(0,chrb[3]);
			mem_setchr4(4,chrb[3]);
			break;
		case 2:
			mem_setchr2(0,chrb[1]);
			mem_setchr2(2,chrb[3]);
			mem_setchr2(4,chrb[1]);
			mem_setchr2(6,chrb[3]);
			break;
		case 3:
			mem_setchr1(0,chrb[0]);
			mem_setchr1(1,chrb[1]);
			mem_setchr1(2,chrb[2]);
			mem_setchr1(3,chrb[3]);
			mem_setchr1(4,chrb[0]);
			mem_setchr1(5,chrb[1]);
			mem_setchr1(6,chrb[2]);
			mem_setchr1(7,chrb[3]);
			break;
	}
}

void mmc5_syncchr()
{
	if(chrselect == 0)
		mmc5_syncchra();
	else
		mmc5_syncchrb();
}

void mmc5_syncmirror()
{
	mmc5_setmirror(0,mirror & 3);
	mmc5_setmirror(1,(mirror >> 2) & 3);
	mmc5_setmirror(2,(mirror >> 4) & 3);
	mmc5_setmirror(3,(mirror >> 6) & 3);
}

void mmc5_sync()
{
	mmc5_syncprg();
	mmc5_syncchr();
	mmc5_syncmirror();
}

u8 mmc5_read(u32 addr)
{
	u8 ret = 0xFF;

	if(addr >= 0x5C00) {
		if(exrammode >= 2)
			return(exram[addr & 0x3FF]);
		return(0xFF);
	}
	switch(addr) {
		//sound
		case 0x5015:
			return(MMC5sound_Read(addr));

		//irq status
		case 0x5204:
			//return value
			ret = irqstatus;

			//acknowledge the irq
			irqstatus &= ~0x80;
			cpu_clear_irq(IRQ_MAPPER);
			return(ret);

		//low 8 bits of product
		case 0x5205:
			return((u8)(multiply[0] * multiply[1]));

		//high 8 bits of product
		case 0x5206:
			return((u8)((multiply[0] * multiply[1]) >> 8));

		default:
			log_printf("mmc5:  unhandled read $%04X\n",addr);
			break;
	}
	return(0xFF);
}

void mmc5_write(u32 addr,u8 data)
{
	if(addr >= 0x5C00) {
		if(exrammode == 2)
			exram[addr & 0x3FF] = data;
		return;
	}
	switch(addr) {
		//sound registers
		case 0x5000:
		case 0x5001:
		case 0x5002:
		case 0x5003:
		case 0x5004:
		case 0x5005:
		case 0x5006:
		case 0x5007:
		case 0x5010:
		case 0x5011:
		case 0x5015:
			MMC5sound_Write(addr,data);
			break;

		//prg mode select
		case 0x5100:
			prgmode = data & 3;
			break;

		//chr mode select
		case 0x5101:
			chrmode = data & 3;
			break;

		//prg ram 'a' and 'b' select
		case 0x5102:
		case 0x5103:
			prgprotect[addr & 1] = data & 3;
			break;

		//exram mode
		case 0x5104:
			exrammode = data & 3;
			break;

		//mirroring mode
		case 0x5105:
			mirror = data;
			break;

		//fill tile
		case 0x5106:
			filltile = data & 3;
			break;

		//fill attribute
		case 0x5107:
			fillattrib = data & 3;
			break;

		//prgram select
		case 0x5113:
			prgram = data & 7;
			break;

		//prgrom regs
		case 0x5114:
		case 0x5115:
		case 0x5116:
		case 0x5117:
			prg[addr & 3] = data;
			break;

		//chr 'a' regs
		case 0x5120:
		case 0x5121:
		case 0x5122:
		case 0x5123:
		case 0x5124:
		case 0x5125:
		case 0x5126:
		case 0x5127:
			chra[addr & 7] = data | (chrhi << 8);
			chrselect = 0;
			break;

		//chr 'b' regs
		case 0x5128:
		case 0x5129:
		case 0x512A:
		case 0x512B:
			chrb[addr & 3] = data | (chrhi << 8);
			chrselect = 1;
			break;

		//high 2 bits of all chr registers
		case 0x5130:
			chrhi = data & 3;
			break;

		//split screen control
		case 0x5200:
			break;

		//split screen vscroll
		case 0x5201:
			break;

		//split screen chr page
		case 0x5202:
			break;

		//irq enable
		case 0x5203:
			irqtarget = data;
			break;

		//irq enable
		case 0x5204:
			irqenable = data;
			break;

		//multiplicand
		case 0x5205:
			multiply[0] = data;
			break;

		//multiplier
		case 0x5206:
			multiply[1] = data;
			break;

		default:
			log_printf("mmc5:  unhandled write $%04X = $%02X\n",addr,data);
			break;
	}
	sync();
}

void mmc5_reset(int hard)
{
	int i;

	//setup read/write funcs
	mem_setreadfunc(5,mmc5_read);
	mem_setwritefunc(5,mmc5_write);

	//setup sram/wram (for exram)
	mem_setsramsize(8);
	mem_setwramsize(1);		//for exram

	//map in wram and get pointer to data (for exram)
	mem_setwram8(8,0);
	exram = mem_getreadptr(8);
	mem_unsetcpu8(8);

	//hijack the ppu memory read function
	ppuread = ppu_getreadfunc();
	ppu_setreadfunc(mmc5_ppuread);

	//zero out exram (for disabled exram)
	for(i=0;i<4096;i++)
		exram[i] = 0;

	//clear the registers
	for(i=0;i<8;i++) {
		prg[i & 3] = 0;
		chra[i] = chrb[i & 3] = 0;
	}

	//setup the registers
	prg[3] = 0xFF;
	chrhi = 0;
	prgram = 0;
	prgmode = 3;
	chrmode = exrammode = 0;
	irqenable = 0;
	irqtarget = 0;
	irqstatus = 0;
	irqcounter = 0;
	sync = mmc5_sync;
	sync();
}

static void scanline_detected()
{
	//see if 'in frame' flag is set
	if(irqstatus & 0x40) {
		irqcounter++;
		if(irqcounter == irqtarget && irqenable) {
			irqstatus |= 0x80;
			cpu_set_irq(IRQ_MAPPER);
		}
	}

	else {
		//set the in frame flag
		irqstatus |= 0x40;

		//clear irq pending flag
		irqstatus &= ~0x80;
		cpu_clear_irq(IRQ_MAPPER);

		//reset counter
		irqcounter = 0;
	}
//	log_printf("mmc5:  irqcounter incremented to %d on line %d\n",irqcounter,SCANLINE);
}

void mmc5_ppucycle()
{
	if(SCANLINE < 240) {
		//see if rendering is enabled
		if(CONTROL1 & 0x18) {

			//detect scanlines
			if(LINECYCLES == 337) {
				scanline_detected();
			}

			//if 8x16 sprites
			if(CONTROL0 & 0x20) {
				//detect when to switch between sprite and bg tiles
				if(LINECYCLES == 257) {
					mmc5_syncchra();
					chrselect = 0;
				}
				if(LINECYCLES == 321) {
					mmc5_syncchrb();
					chrselect = 1;
				}
			}

		}
	}
	else {
		//clear in frame flag
		irqstatus &= ~0x40;
	}
}

void mmc5_state(int mode,u8 *data)
{
	STATE_ARRAY_U8(prg,4);
	STATE_ARRAY_U16(chra,8);
	STATE_ARRAY_U16(chrb,4);
	STATE_U8(chrhi);
	STATE_U8(prgram);
	STATE_U8(mirror);
	STATE_U8(prgmode);
	STATE_U8(chrmode);
	STATE_U8(exrammode);
	STATE_ARRAY_U8(multiply,2);
	STATE_U8(filltile);
	STATE_U8(fillattrib);
	STATE_U8(irqtarget);
	STATE_U8(irqcounter);
	STATE_U8(irqenable);
	MMC5sound_SaveLoad(mode,0,data);
}
