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

//this code is ported from nintendulator's c++ apu

#include "nes/nes.h"
#include "nes/state/state.h"
#include "misc/log.h"
#include "misc/memutil.h"
#include "system/sound.h"

#define SOUND_HZ	44100
#define NES_HZ		1789773

u8 LengthCounts[32] = {
	0x0A,0xFE,
	0x14,0x02,
	0x28,0x04,
	0x50,0x06,
	0xA0,0x08,
	0x3C,0x0A,
	0x0E,0x0C,
	0x1A,0x0E,

	0x0C,0x10,
	0x18,0x12,
	0x30,0x14,
	0x60,0x16,
	0xC0,0x18,
	0x48,0x1A,
	0x10,0x1C,
	0x20,0x1E
};

#include "nes/apu/race.c"
#include "nes/apu/units/square.c"
#include "nes/apu/units/triangle.c"
#include "nes/apu/units/noise.c"
#include "nes/apu/units/dpcm.c"
#include "nes/apu/units/frame.c"

static u8 regs[0x20];
static int cycles = 0;
static const int soundbufsize = 1024 * 3;
static s16 *soundbuf = 0;
static u16 soundbuflen = 0;
static u16 soundbufpos = 0;

static void apu_callback(void *data,int length)
{
	s16 *dest = (s16*)data;
	int i,pos,len;

//	printf("apu_callback:  need %d bytes, have %d ready (pixel %d, line %d, frame %d)\n",length,soundbuflen,LINECYCLES,SCANLINE,FRAMES);
	len = ((length > soundbuflen) ? soundbuflen : length);
	pos = soundbufpos;
	for(i=0;i<length;i++) {
		*dest++ = soundbuf[pos];
		pos = (pos + 1) % soundbufsize;
	}
	soundbuflen -= len;
	soundbufpos = (soundbufpos + len) % soundbufsize;
	cycles = 0;
}

int apu_init()
{
	int i;

	state_register(B_APU,apu_state);
	sound_setcallback(apu_callback);
	soundbuf = (s16*)mem_alloc(sizeof(s16) * soundbufsize);
	for(i=0;i<soundbufsize;i++)
		soundbuf[i] = 0;
	return(0);
}

void apu_kill()
{
	if(nes.apu.external)
		nes.apu.external->kill();
	if(soundbuf) {
		mem_free(soundbuf);
		soundbuf = 0;
	}
	sound_setcallback(0);
}

void apu_reset(int hard)
{
	if(hard) {
		cpu_clear_irq(IRQ_FRAME | IRQ_DPCM);
	}
	apu_frame_reset(hard);
	apu_race_reset(hard);
	apu_square0_reset(hard);
	apu_square1_reset(hard);
	apu_triangle_reset(hard);
	apu_noise_reset(hard);
	apu_dpcm_reset(hard);
	if(nes.apu.external)
		nes.apu.external->reset();
	cycles = 0;
}

u8 apu_read(u32 addr)
{
	u8 ret = 0;

	switch(addr) {
		case 0x4015:
			ret = 0;
			if(nes.apu.square[0].LengthCtr)	ret |= 0x01;
			if(nes.apu.square[1].LengthCtr)	ret |= 0x02;
			if(nes.apu.triangle.LengthCtr)	ret |= 0x04;
			if(nes.apu.noise.LengthCtr)		ret |= 0x08;
			if(nes.apu.dpcm.LengthCtr)			ret |= 0x10;
			if(nes.cpu.irqstate & IRQ_FRAME)	ret |= 0x40;
			if(nes.cpu.irqstate & IRQ_DPCM)	ret |= 0x80;
			cpu_clear_irq(IRQ_FRAME);
//			log_printf("apu_read:  $4015:  ret = %02X (cycle %d, line %d, frame %d)\n",ret,LINECYCLES,SCANLINE,FRAMES);
			return(ret);
	}
//	log_printf("apu_read: $%04X\n",addr);
	return(0);
}

void apu_write(u32 addr,u8 data)
{
//	log_printf("apu_write: $%04X = $%02X\n",addr,data);
	regs[addr & 0x1F] = data;
	switch(addr) {
		case 0x4000:
		case 0x4001:
		case 0x4002:
		case 0x4003:
			apu_square0_write(addr & 3,data);
			break;
		case 0x4004:
		case 0x4005:
		case 0x4006:
		case 0x4007:
			apu_square1_write(addr & 3,data);
			break;
		case 0x4008:
		case 0x4009:
		case 0x400A:
		case 0x400B:
			apu_triangle_write(addr & 3,data);
			break;
		case 0x400C:
		case 0x400D:
		case 0x400E:
		case 0x400F:
			apu_noise_write(addr & 3,data);
			break;
		case 0x4010:
		case 0x4011:
		case 0x4012:
		case 0x4013:
			apu_dpcm_write(addr & 3,data);
			break;
		case 0x4015:
			apu_square0_write(4,data & 1);
			apu_square1_write(4,data & 2);
			apu_triangle_write(4,data & 4);
			apu_noise_write(4,data & 8);
			apu_dpcm_write(4,data & 0x10);
			break;
		case 0x4017:
			apu_frame_write(addr,data);
			break;
		default:
			log_printf("apu_write:  unhandled write $%04X = $%02X\n",addr,data);
			break;
	}
}

static int oldpos = -1;

static INLINE void updatebuffer()
{
	int sample,n;

	if(soundbuflen >= soundbufsize) {
//			log_printf("soundbuffer overflow!  %d! (cycles = %d)\n",soundbuflen,cycles);
		return;
	}
	sample = nes.apu.square[0].Pos + nes.apu.square[1].Pos + nes.apu.triangle.Pos + nes.apu.noise.Pos + nes.apu.dpcm.Pos;
	sample *= 64;
	if(nes.apu.external)
		sample += nes.apu.external->process(40);
	if(sample < -0x8000)
		sample = -0x8000;
	if(sample > 0x7FFF)
		sample = 0x7FFF;
	n = (soundbuflen++ + soundbufpos) % soundbufsize;
	soundbuf[n] = (s16)sample;
}

//this is called every cycle
void apu_step()
{
	int pos;

	apu_frame_step();
	apu_race_step();
	apu_square0_step();
	apu_square1_step();
	apu_triangle_step();
	apu_noise_step();
	apu_dpcm_step();
	pos = SOUND_HZ * cycles++ / NES_HZ;
	if(pos != oldpos) {
		oldpos = pos;
		updatebuffer();
	}
}

void apu_setexternal(external_t *ext)
{
	if(nes.apu.external)
		nes.apu.external->kill();
	nes.apu.external = ext;
	ext->init();
}

void apu_state(int mode,u8 *data)
{
	STATE_ARRAY_U8(regs,0x20);
}
