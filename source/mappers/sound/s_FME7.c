/* Nintendulator Mapper DLLs
 * Copyright (C) 2002-2008 QMT Productions
 *
 * $URL: file:///c:/svnroot/Mappers/src/Hardware/Sound/s_FME7.c $
 * $Id: s_FME7.c 376 2008-06-29 20:58:13Z Quietust $
 */

//#include	"..\..\interface.h"
#include <string.h>
#include	"s_FME7.h"
#include	"types.h"

// Sunsoft FME-7, based on the AY-8910

typedef	struct	FME7sqr
{
	union
	{
		struct
		{
			unsigned freq    :12;
			unsigned         : 4;
			unsigned volume  : 4;
			unsigned envmode : 1;
			unsigned         : 3;
		};
		struct
		{
			unsigned byte0 : 8;
			unsigned byte1 : 8;
			unsigned byte2 : 8;
		};
	};
	u8 CurP;
	s32 LCtr;
}	TFME7sqr, *PFME7sqr;

typedef	struct	FME7sound
{
	union
	{
		struct
		{
			unsigned tone      : 3;
			unsigned           : 3;
			unsigned           : 2;
			unsigned envelope  :16;
			unsigned envhold   : 1;
			unsigned envaltr   : 1;
			unsigned envattk   : 1;
			unsigned envcont   : 1;
			unsigned           : 4;
		};
		struct
		{
			unsigned byte7 : 8;
			unsigned byteB : 8;
			unsigned byteC : 8;
			unsigned byteD : 8;
		};
	};
	u8 select;
	TFME7sqr Sqr[3];
}	TFME7sound, *PFME7sound;

static	TFME7sound	FME7sound;

static	int	FME7_DoSquare (PFME7sqr ChanData, int Cycles)
{
	ChanData->LCtr -= Cycles;
	while (ChanData->LCtr <= 0)
	{
		ChanData->CurP++;
		ChanData->CurP &= 0x1F;
		ChanData->LCtr += ChanData->freq + 1;
	}
	return ChanData->volume * ((ChanData->CurP & 0x10) ? 3 : -3);
}

void	FME7sound_Load (void)
{
	memset(&FME7sound,0,sizeof(FME7sound));
}

void	FME7sound_Reset (void)
{
	FME7sound.Sqr[0].LCtr = 1;
	FME7sound.Sqr[1].LCtr = 1;
	FME7sound.Sqr[2].LCtr = 1;
}

void	FME7sound_Unload (void)
{
}

void	FME7sound_Write (int Addr, int Val)
{
	switch (Addr & 0xE000)
	{
	case 0xC000:	FME7sound.select = Val & 0xF;	break;
	case 0xE000:	switch (FME7sound.select)
			{
			case 0x0:	FME7sound.Sqr[0].byte0 = Val;	break;
			case 0x1:	FME7sound.Sqr[0].byte1 = Val;	break;
			case 0x2:	FME7sound.Sqr[1].byte0 = Val;	break;
			case 0x3:	FME7sound.Sqr[1].byte1 = Val;	break;
			case 0x4:	FME7sound.Sqr[2].byte0 = Val;	break;
			case 0x5:	FME7sound.Sqr[2].byte1 = Val;	break;
			case 0x7:	FME7sound.byte7 = Val;	break;
			case 0x8:	FME7sound.Sqr[0].byte2 = Val;	break;
			case 0x9:	FME7sound.Sqr[1].byte2 = Val;	break;
			case 0xA:	FME7sound.Sqr[2].byte2 = Val;	break;
			case 0xB:	FME7sound.byteB = Val;	break;
			case 0xC:	FME7sound.byteC = Val;	break;
			case 0xD:	FME7sound.byteD = Val;	break;
			}				break;
	}
}

int	FME7sound_Get (int Cycles)
{
	int z = 0;
	if (!(FME7sound.tone & 1))	z += FME7_DoSquare(&FME7sound.Sqr[0],Cycles);
	if (!(FME7sound.tone & 2))	z += FME7_DoSquare(&FME7sound.Sqr[1],Cycles);
	if (!(FME7sound.tone & 4))	z += FME7_DoSquare(&FME7sound.Sqr[2],Cycles);
	return z << 6;
}

int	FME7sound_SaveLoad (int mode, int x, unsigned char *data)
{
/*	SAVELOAD_BYTE(mode,x,data,FME7sound.select);
	SAVELOAD_BYTE(mode,x,data,FME7sound.Sqr[0].byte0);
	SAVELOAD_BYTE(mode,x,data,FME7sound.Sqr[0].byte1);
	SAVELOAD_BYTE(mode,x,data,FME7sound.Sqr[0].byte2);
	SAVELOAD_BYTE(mode,x,data,FME7sound.Sqr[0].CurP);
	SAVELOAD_LONG(mode,x,data,FME7sound.Sqr[0].LCtr);
	SAVELOAD_BYTE(mode,x,data,FME7sound.Sqr[1].byte0);
	SAVELOAD_BYTE(mode,x,data,FME7sound.Sqr[1].byte1);
	SAVELOAD_BYTE(mode,x,data,FME7sound.Sqr[1].byte2);
	SAVELOAD_BYTE(mode,x,data,FME7sound.Sqr[1].CurP);
	SAVELOAD_LONG(mode,x,data,FME7sound.Sqr[1].LCtr);
	SAVELOAD_BYTE(mode,x,data,FME7sound.Sqr[2].byte0);
	SAVELOAD_BYTE(mode,x,data,FME7sound.Sqr[2].byte1);
	SAVELOAD_BYTE(mode,x,data,FME7sound.Sqr[2].byte2);
	SAVELOAD_BYTE(mode,x,data,FME7sound.Sqr[2].CurP);
	SAVELOAD_LONG(mode,x,data,FME7sound.Sqr[2].LCtr);
*/	return x;
}
