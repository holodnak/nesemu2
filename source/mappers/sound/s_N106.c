/* Nintendulator Mapper DLLs
 * Copyright (C) 2002-2008 QMT Productions
 *
 * $URL: file:///c:/svnroot/Mappers/src/Hardware/Sound/s_N106.c $
 * $Id: s_N106.c 376 2008-06-29 20:58:13Z Quietust $
 */

//#include	"..\..\interface.h"
#include <string.h>
#include "types.h"
#include	"s_N106.h"

// Namco 106

struct	N106chan
{
	u8 freql, freqm, freqh;
	u32 freq;
	u8 len;
	u8 addr;
	u8 volume;
	u8 CurP;
	u8 CurA;
	s32 LCtr;
} Ch[8];

static	struct	TN106sound
{
	u8 data[0x80];
	struct	N106chan Ch[8];
	u8 chans;
	u8 addr;
	u8 inc;
}	N106sound;

void	N106sound_Load (void)
{
	memset(&N106sound,0,sizeof(N106sound));
}

void	N106sound_Reset (void)
{
	int i;
	for (i = 0; i < 8; i++)
		N106sound.Ch[i].len = 0x10;
	N106sound.chans = 8;
	N106sound.inc = 0x80;
}

void	N106sound_Unload (void)
{
}

void	N106sound_Write (int Addr, int Val)
{
	switch (Addr & 0xF800)
	{
	case 0xF800:
		N106sound.addr = Val & 0x7F;
		N106sound.inc = Val & 0x80;
		break;
	case 0x4800:
		N106sound.data[N106sound.addr] = Val;
		if (N106sound.addr & 0x40)
		{
			struct N106chan *Chan = &N106sound.Ch[(N106sound.addr & 0x3F) >> 3];
			switch (N106sound.addr & 0x7)
			{
			case 0:	Chan->freql = Val;
				Chan->freq = Chan->freql | (Chan->freqm << 8) | (Chan->freqh << 16);
				break;
			case 2:	Chan->freqm = Val;
				Chan->freq = Chan->freql | (Chan->freqm << 8) | (Chan->freqh << 16);
				break;
			case 4:	Chan->freqh = Val & 3;
				Chan->freq = Chan->freql | (Chan->freqm << 8) | (Chan->freqh << 16);
				if (Chan->len != 0x20 - (Val & 0x1C))
				{
					Chan->len = 0x20 - (Val & 0x1C);
					Chan->CurA = 0;
				}
				break;
			case 6:	Chan->addr = Val;
				break;
			case 7:	Chan->volume = Val & 0xF;
				if (Chan == &N106sound.Ch[7])
					N106sound.chans = 1 + ((Val >> 4) & 0x7);
				break;
			}
		}
		if (N106sound.inc)
		{
			N106sound.addr++;
			N106sound.addr &= 0x7F;
		}
		break;
	}
}

int	N106sound_Read (int Addr)
{
	int data = N106sound.data[N106sound.addr];

	if (N106sound.inc)
	{
		N106sound.addr++;
		N106sound.addr &= 0x7F;
	}

	return data;
}

static	int	N106_GenerateWave (struct N106chan *Chan, int Cycles)
{
	s32 freq;

	if (!Chan->freq)
		return 0;
	freq = (0xF0000 * N106sound.chans) / Chan->freq;
	Chan->LCtr += Cycles;
	while (Chan->LCtr > freq)
	{
		u8 addr;
		Chan->CurA++;
		while (Chan->CurA >= Chan->len)
			Chan->CurA -= Chan->len;
		addr = Chan->addr + Chan->CurA;
		Chan->CurP = N106sound.data[addr >> 1];
		if (addr & 1)
			Chan->CurP >>= 4;
		else	Chan->CurP &= 0xF;
		Chan->LCtr -= freq;
	}
	return (Chan->CurP - 0x8) * Chan->volume;
}
int	N106sound_Get (int Cycles)
{
	int out = 0;
	int i;
	for (i = 8 - N106sound.chans; i < 8; i++)
		out += N106_GenerateWave(&N106sound.Ch[i],Cycles);
	return out << 5;
}

int	N106sound_SaveLoad (int statetype, int x, unsigned char *data)
{
	int i;
	switch (statetype)
	{
	case 0: //save
		for (i = 0; i < 0x80; i++)
			data[x++] = N106sound.data[i];
		data[x++] = N106sound.addr | N106sound.inc;
		break;
	case 1: //load
		N106sound_Write(0xF800,0x80);
		for (i = 0; i < 0x80; i++)
			N106sound_Write(0x4800,data[x++]);
		N106sound_Write(0xF800,data[x++]);
		break;
	case 2: //size of state data
		x += 0x81;
		break;
	}
	for (i = 0; i < 8; i++)
	{
//		SAVELOAD_BYTE(mode,x,data,N106sound.Ch[i].CurP);
//		SAVELOAD_BYTE(mode,x,data,N106sound.Ch[i].CurA);
//		SAVELOAD_LONG(mode,x,data,N106sound.Ch[i].LCtr);
	}
	return x;
}
