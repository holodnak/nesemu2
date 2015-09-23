#include <string.h>
#include "mappers/sound/s_DRIP.h"
#include "types.h"

typedef struct dripsound_s {
	u8 FIFO[256], ReadPos, WritePos;
	int IsFull, IsEmpty;
	int freq, vol;
	int timer;
	int Pos;
} dripsound_t;

static dripsound_t chan[2];

static int	GenerateWave (dripsound_t *ds,int Cycles)
{
	int i,z = 0;

	for(i = 0; i < Cycles; i++)
	{
		if (ds->IsEmpty)
			break;
		if (!ds->timer--)
		{
			ds->timer = ds->freq;
			if (ds->ReadPos == ds->WritePos)
				ds->IsFull = 0;
			ds->Pos = (ds->FIFO[++ds->ReadPos] - 0x80) * ds->vol;
			if (ds->ReadPos == ds->WritePos)
				ds->IsEmpty = 1;
		}
		z += ds->Pos;
	}
	return z / Cycles;
}

static int	Read(dripsound_t *ds,int Addr)
{
	int result = 0;

	if (ds->IsFull)
		result |= 0x80;
	if (ds->IsEmpty)
		result |= 0x40;
	return result;
}

static void	Write(dripsound_t *ds,int Addr,int Val)
{
	switch (Addr & 3)
		{
		case 0x0:
			memset(ds->FIFO,0,256);
			ds->ReadPos = ds->WritePos = 0;
			ds->IsFull = 0;
			ds->IsEmpty = 1;
			ds->Pos = 0;
			ds->timer = ds->freq;
			break;
		case 0x1:
			if (ds->ReadPos == ds->WritePos)
			{
				ds->IsEmpty = 0;
				ds->Pos = (Val - 0x80) * ds->vol;
				ds->timer = ds->freq;
			}
			ds->FIFO[ds->WritePos++] = Val;
			if (ds->ReadPos == ds->WritePos)
				ds->IsFull = 1;
			break;
		case 0x2:
			ds->freq = (ds->freq & 0xF00) | Val;
			break;
		case 0x3:
			ds->freq = (ds->freq & 0xFF) | ((Val & 0xF) << 8);
			ds->vol = (Val & 0xF0) >> 4;
			if (!ds->IsEmpty)
				ds->Pos = (ds->FIFO[ds->ReadPos] - 0x80) * ds->vol;
			break;
		}
}

void	DRIPsound_Load(void)
{
}

void	DRIPsound_Reset(void)
{
	memset(chan,0,sizeof(dripsound_t) * 2);
	chan[0].IsEmpty = 1;
	chan[1].IsEmpty = 1;
}

void	DRIPsound_Unload(void)
{
}

int	DRIPsound_Read(int Addr)
{
	if(Addr & 0x800)
		return(Read(&chan[1],Addr));
	return(Read(&chan[0],Addr));
}

void	DRIPsound_Write(int Addr,int Val)
{
	if(Addr & 4)
		Write(&chan[1],Addr,Val);
	else
		Write(&chan[0],Addr,Val);
}

int	DRIPsound_Get(int Cycles)
{
	int out = 0;
	out += GenerateWave(&chan[0],Cycles);
	out += GenerateWave(&chan[1],Cycles);
	return out << 3;
}

int	DRIPsound_SaveLoad(int statetype,int a,unsigned char *d)
{
/*	for (int i = 0; i < 256; i++)
		SAVELOAD_BYTE(mode, offset, data, Chan[0].FIFO[i]);
	for (int i = 0; i < 256; i++)
		SAVELOAD_BYTE(mode, offset, data, Chan[1].FIFO[i]);
	SAVELOAD_BYTE(mode, offset, data, Chan[0].ReadPos);
	SAVELOAD_BYTE(mode, offset, data, Chan[1].ReadPos);
	SAVELOAD_BYTE(mode, offset, data, Chan[0].WritePos);
	SAVELOAD_BYTE(mode, offset, data, Chan[1].WritePos);
	SAVELOAD_BYTE(mode, offset, data, Chan[0].IsFull);
	SAVELOAD_BYTE(mode, offset, data, Chan[1].IsFull);
	SAVELOAD_BYTE(mode, offset, data, Chan[0].IsEmpty);
	SAVELOAD_BYTE(mode, offset, data, Chan[1].IsEmpty);
	SAVELOAD_WORD(mode, offset, data, Chan[0].freq);
	SAVELOAD_WORD(mode, offset, data, Chan[1].freq);
	SAVELOAD_BYTE(mode, offset, data, Chan[0].vol);
	SAVELOAD_BYTE(mode, offset, data, Chan[1].vol);
	SAVELOAD_WORD(mode, offset, data, Chan[0].timer);
	SAVELOAD_WORD(mode, offset, data, Chan[1].timer);*/
	return(0);
}
