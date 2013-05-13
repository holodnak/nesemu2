/* Nintendulator Mapper DLLs
 * Copyright (C) 2002-2008 QMT Productions
 *
 * $URL: file:///c:/svnroot/Mappers/src/Hardware/Sound/s_VRC7.c $
 * $Id: s_VRC7.c 376 2008-06-29 20:58:13Z Quietust $
 */

/***********************************************************************************

	emu2413.c -- YM2413 emulator written by Mitsutaka Okazaki 2001

	2001 01-08 : Version 0.10 -- 1st version.
	2001 01-15 : Version 0.20 -- semi-public version.
	2001 01-16 : Version 0.30 -- 1st public version.
	2001 01-17 : Version 0.31 -- Fixed bassdrum problem.
		   : Version 0.32 -- LPF implemented.
	2001 01-18 : Version 0.33 -- Fixed the drum problem, refine the mix-down method.
				  -- Fixed the LFO bug.
	2001 01-24 : Version 0.35 -- Fixed the drum problem,
				     support undocumented EG behavior.
	2001 02-02 : Version 0.38 -- Improved the performance.
				     Fixed the hi-hat and cymbal model.
				     Fixed the default percussive datas.
				     Noise reduction.
				     Fixed the feedback problem.
	2001 03-03 : Version 0.39 -- Fixed some drum bugs.
				     Improved the performance.
	2001 03-04 : Version 0.40 -- Improved the feedback.
				     Change the default table size.
				     Clock and Rate can be changed during play.
	2001 06-24 : Version 0.50 -- Improved the hi-hat and the cymbal tone.
				     Added VRC7 patch (OPLL_reset_patch is changed).
				     Fixed OPLL_reset() bug.
				     Added OPLL_setMask, OPLL_getMask and OPLL_toggleMask.
				     Added OPLL_writeIO.
	2001 09-28 : Version 0.51 -- Removed the noise table.
	2002 01-28 : Version 0.52 -- Added Stereo mode.
	2002 02-07 : Version 0.53 -- Fixed some drum bugs.
	2002 02-20 : Version 0.54 -- Added the best quality mode.
	2002 03-02 : Version 0.55 -- Removed OPLL_init & OPLL_close.
	2002 05-30 : Version 0.60 -- Fixed HH&CYM generator and all voice datas.

	2003 01-24 : Modified by xodnizel to remove code not needed for the VRC7, among other things.

	References:
		fmopl.c	       -- 1999,2000 written by Tatsuyuki Satoh (MAME development).
		fmopl.c(fixed) -- (C) 2002 Jarek Burczynski.
		s_opl.c	       -- 2001 written by Mamiya (NEZplug development).
		fmgen.cpp      -- 1999,2000 written by cisc.
		fmpac.ill      -- 2000 created by NARUTO.
		MSX-Datapack
		YMU757 data sheet
		YM2143 data sheet

**************************************************************************************/
//#include	"..\..\interface.h"
#include	"s_VRC7.h"
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<math.h>
#include	"types.h"

#define PI 3.14159265358979323846

enum {OPLL_VRC7_TONE=0};

/* voice data */
typedef struct {
	u32 TL,FB,EG,ML,AR,DR,SL,RR,KR,KL,AM,PM,WF;
} OPLL_PATCH;

/* slot */
typedef struct {
	OPLL_PATCH patch;

	s32 type;          /* 0 : modulator 1 : carrier */

	/* OUTPUT */
	s32 feedback;
	s32 output[2];   /* Output value of slot */

	/* for Phase Generator (PG) */
	u16 *sintbl;    /* Wavetable */
	u32 phase;      /* Phase */
	u32 dphase;     /* Phase increment amount */
	u32 pgout;      /* output */

	/* for Envelope Generator (EG) */
	s32 fnum;          /* F-Number */
	s32 block;         /* Block */
	s32 volume;        /* Current volume */
	s32 sustine;       /* Sustine 1 = ON, 0 = OFF */
	u32 tll;	      /* Total Level + Key scale level*/
	u32 rks;        /* Key scale offset (Rks) */
	s32 eg_mode;       /* Current state */
	u32 eg_phase;   /* Phase */
	u32 eg_dphase;  /* Phase increment amount */
	u32 egout;      /* output */

} OPLL_SLOT;

/* Mask */
#define OPLL_MASK_CH(x) (1<<(x))

/* opll */
typedef struct {

	u32 adr;
	s32 out;

#ifndef EMU2413_COMPACTION
	u32 realstep;
	u32 oplltime;
	u32 opllstep;
	s32 prev, next;
#endif

	/* Register */
	u8 LowFreq[6];
	u8 HiFreq[6];
	u8 InstVol[6];

	u8 CustInst[8];

	s32 slot_on_flag[6 * 2];

	/* Pitch Modulator */
	u32 pm_phase;
	s32 lfo_pm;

	/* Amp Modulator */
	s32 am_phase;
	s32 lfo_am;

	u32 quality;

	/* Channel Data */
	s32 patch_number[6];
	s32 key_status[6];

	/* Slot */
	OPLL_SLOT slot[6 * 2];

	u32 mask;

} OPLL;

/* Create Object */
OPLL *OPLL_new(u32 clk, u32 rate);
void OPLL_delete(OPLL *);

/* Setup */
void OPLL_reset(OPLL *);
void OPLL_set_rate(OPLL *opll, u32 r);
void OPLL_set_quality(OPLL *opll, u32 q);

/* Port/Register access */
void OPLL_writeIO(OPLL *, u32 reg, u32 val);
void OPLL_writeReg(OPLL *, u32 reg, u32 val);

/* Synthsize */
s16 OPLL_calc(OPLL *);

/* Misc */
void OPLL_forceRefresh(OPLL *);

/* Channel Mask */
u32 OPLL_setMask(OPLL *, u32 mask);
u32 OPLL_toggleMask(OPLL *, u32 mask);


static const unsigned char default_inst[15][8] = {
	{0x03,0x21,0x04,0x06,0x8D,0xF2,0x42,0x17},
	{0x13,0x41,0x05,0x0E,0x99,0x96,0x63,0x12},
	{0x31,0x11,0x10,0x0A,0xF0,0x9C,0x32,0x02},
	{0x21,0x61,0x1D,0x07,0x9F,0x64,0x20,0x27},
	{0x22,0x21,0x1E,0x06,0xF0,0x76,0x08,0x28},
	{0x02,0x01,0x06,0x00,0xF0,0xF2,0x03,0x95},
	{0x21,0x61,0x1C,0x07,0x82,0x81,0x16,0x07},
	{0x23,0x21,0x1A,0x17,0xEF,0x82,0x25,0x15},
	{0x25,0x11,0x1F,0x00,0x86,0x41,0x20,0x11},
	{0x85,0x01,0x1F,0x0F,0xE4,0xA2,0x11,0x12},
	{0x07,0xC1,0x2B,0x45,0xB4,0xF1,0x24,0xF4},
	{0x61,0x23,0x11,0x06,0x96,0x96,0x13,0x16},
	{0x01,0x02,0xD3,0x05,0x82,0xA2,0x31,0x51},
	{0x61,0x22,0x0D,0x02,0xC3,0x7F,0x24,0x05},
	{0x21,0x62,0x0E,0x00,0xA1,0xA0,0x44,0x17}
};

/* Size of Sintable ( 8 -- 18 can be used. 9 recommended.)*/
#define PG_BITS 9
#define PG_WIDTH (1<<PG_BITS)

/* Phase increment counter */
#define DP_BITS 18
#define DP_WIDTH (1<<DP_BITS)
#define DP_BASE_BITS (DP_BITS - PG_BITS)

/* Dynamic range (Accuracy of sin table) */
#define DB_BITS 8
#define DB_STEP (48.0/(1<<DB_BITS))
#define DB_MUTE (1<<DB_BITS)

/* Dynamic range of envelope */
#define EG_STEP 0.375
#define EG_BITS 7
#define EG_MUTE (1<<EG_BITS)

/* Dynamic range of total level */
#define TL_STEP 0.75
#define TL_BITS 6
#define TL_MUTE (1<<TL_BITS)

/* Dynamic range of sustine level */
#define SL_STEP 3.0
#define SL_BITS 4
#define SL_MUTE (1<<SL_BITS)

#define EG2DB(d) ((d)*(s32)(EG_STEP/DB_STEP))
#define TL2EG(d) ((d)*(s32)(TL_STEP/EG_STEP))
#define SL2EG(d) ((d)*(s32)(SL_STEP/EG_STEP))

#define DB_POS(x) (u32)((x)/DB_STEP)
#define DB_NEG(x) (u32)(DB_MUTE+DB_MUTE+(x)/DB_STEP)

/* Bits for liner value */
#define DB2LIN_AMP_BITS 11
#define SLOT_AMP_BITS (DB2LIN_AMP_BITS)

/* Bits for envelope phase incremental counter */
#define EG_DP_BITS 22
#define EG_DP_WIDTH (1<<EG_DP_BITS)

/* Bits for Pitch and Amp modulator */
#define PM_PG_BITS 8
#define PM_PG_WIDTH (1<<PM_PG_BITS)
#define PM_DP_BITS 16
#define PM_DP_WIDTH (1<<PM_DP_BITS)
#define AM_PG_BITS 8
#define AM_PG_WIDTH (1<<AM_PG_BITS)
#define AM_DP_BITS 16
#define AM_DP_WIDTH (1<<AM_DP_BITS)

/* PM table is calcurated by PM_AMP * pow(2,PM_DEPTH*sin(x)/1200) */
#define PM_AMP_BITS 8
#define PM_AMP (1<<PM_AMP_BITS)

/* PM speed(Hz) and depth(cent) */
#define PM_SPEED 6.4
#define PM_DEPTH 13.75

/* AM speed(Hz) and depth(dB) */
#define AM_SPEED 3.7
#define AM_DEPTH 4.8

/* Cut the lower b bit(s) off. */
#define HIGHBITS(c,b) ((c)>>(b))

/* Leave the lower b bit(s). */
#define LOWBITS(c,b) ((c)&((1<<(b))-1))

/* Expand x which is s bits to d bits. */
#define EXPAND_BITS(x,s,d) ((x)<<((d)-(s)))

/* Expand x which is s bits to d bits and fill expanded bits '1' */
#define EXPAND_BITS_X(x,s,d) (((x)<<((d)-(s)))|((1<<((d)-(s)))-1))

/* Adjust envelope speed which depends on sampling rate. */
#define rate_adjust(x) (rate==49716?x:(u32)((double)(x)*clk/72/rate + 0.5))				/* added 0.5 to round the value*/

#define MOD(o,x) (&(o)->slot[(x)<<1])
#define CAR(o,x) (&(o)->slot[((x)<<1)|1])

#define BIT(s,b) (((s)>>(b))&1)

/* Input clock */
static u32 clk = 844451141;
/* Sampling rate */
static u32 rate = 3354932;

/* WaveTable for each envelope amp */
static u16 fullsintable[PG_WIDTH];
static u16 halfsintable[PG_WIDTH];

static u16 *waveform[2] = { fullsintable, halfsintable };

/* LFO Table */
static s32 pmtable[PM_PG_WIDTH];
static s32 amtable[AM_PG_WIDTH];

/* Phase delta for LFO */
static u32 pm_dphase;
static u32 am_dphase;

/* dB to Liner table */
static s16 DB2LIN_TABLE[(DB_MUTE + DB_MUTE) * 2];

/* Liner to Log curve conversion table (for Attack rate). */
static u16 AR_ADJUST_TABLE[1 << EG_BITS];

/* Definition of envelope mode */
enum
{ SETTLE, ATTACK, DECAY, SUSHOLD, SUSTINE, ERELEASE, FINISH };

/* Phase incr table for Attack */
static u32 dphaseARTable[16][16];
/* Phase incr table for Decay and ERELEASE */
static u32 dphaseDRTable[16][16];

/* KSL + TL Table */
static u32 tllTable[16][8][1 << TL_BITS][4];
static s32 rksTable[2][8][2];

/* Phase incr table for PG */
static u32 dphaseTable[512][8][16];

/***************************************************

		Create tables

****************************************************/
__inline static s32
Min (s32 i, s32 j)
{
	if (i < j)
		return i;
	else
		return j;
}

/* Table for AR to LogCurve. */
static void
makeAdjustTable (void)
{
	s32 i;

	AR_ADJUST_TABLE[0] = (1 << EG_BITS);
	for (i = 1; i < 128; i++)
		AR_ADJUST_TABLE[i] = (u16) ((double) (1 << EG_BITS) - 1 - (1 << EG_BITS) * log (i) / log (128));
}


/* Table for dB(0 -- (1<<DB_BITS)-1) to Liner(0 -- DB2LIN_AMP_WIDTH) */
static void
makeDB2LinTable (void)
{
	s32 i;

	for (i = 0; i < DB_MUTE + DB_MUTE; i++)
	{
		DB2LIN_TABLE[i] = (s16) ((double) ((1 << DB2LIN_AMP_BITS) - 1) * pow (10, -(double) i * DB_STEP / 20));
		if (i >= DB_MUTE) DB2LIN_TABLE[i] = 0;
		DB2LIN_TABLE[i + DB_MUTE + DB_MUTE] = (s16) (-DB2LIN_TABLE[i]);
	}
}

/* Liner(+0.0 - +1.0) to dB((1<<DB_BITS) - 1 -- 0) */
static s32
lin2db (double d)
{
	if (d == 0)
		return (DB_MUTE - 1);
	else
		return Min (-(s32) (20.0 * log10 (d) / DB_STEP), DB_MUTE-1);	/* 0 -- 127 */
}


/* Sin Table */
static void
makeSinTable (void)
{
	s32 i;

	for (i = 0; i < PG_WIDTH / 4; i++)
	{
		fullsintable[i] = (u16) lin2db (sin (2.0 * PI * i / PG_WIDTH) );
	}

	for (i = 0; i < PG_WIDTH / 4; i++)
	{
		fullsintable[PG_WIDTH / 2 - 1 - i] = fullsintable[i];
	}

	for (i = 0; i < PG_WIDTH / 2; i++)
	{
		fullsintable[PG_WIDTH / 2 + i] = (u16) (DB_MUTE + DB_MUTE + fullsintable[i]);
	}

	for (i = 0; i < PG_WIDTH / 2; i++)
		halfsintable[i] = fullsintable[i];
	for (i = PG_WIDTH / 2; i < PG_WIDTH; i++)
		halfsintable[i] = fullsintable[0];
}

/* Table for Pitch Modulator */
static void
makePmTable (void)
{
	s32 i;

	for (i = 0; i < PM_PG_WIDTH; i++)
		pmtable[i] = (s32) ((double) PM_AMP * pow (2, (double) PM_DEPTH * sin (2.0 * PI * i / PM_PG_WIDTH) / 1200));
}

/* Table for Amp Modulator */
static void
makeAmTable (void)
{
	s32 i;

	for (i = 0; i < AM_PG_WIDTH; i++)
		amtable[i] = (s32) ((double) AM_DEPTH / 2 / DB_STEP * (1.0 + sin (2.0 * PI * i / PM_PG_WIDTH)));
}

/* Phase increment counter table */
static void
makeDphaseTable (void)
{
	u32 fnum, block, ML;
	u32 mltable[16] =
		{ 1, 1 * 2, 2 * 2, 3 * 2, 4 * 2, 5 * 2, 6 * 2, 7 * 2, 8 * 2, 9 * 2, 10 * 2, 10 * 2, 12 * 2, 12 * 2, 15 * 2, 15 * 2 };

	for (fnum = 0; fnum < 512; fnum++)
		for (block = 0; block < 8; block++)
			for (ML = 0; ML < 16; ML++)
				dphaseTable[fnum][block][ML] = rate_adjust (((fnum * mltable[ML]) << block) >> (20 - DP_BITS));
}

static void
makeTllTable (void)
{
#define dB2(x) ((x)*2)

	static double kltable[16] = {
		dB2 (0.000), dB2 (9.000), dB2 (12.000), dB2 (13.875), dB2 (15.000), dB2 (16.125), dB2 (16.875), dB2 (17.625),
		dB2 (18.000), dB2 (18.750), dB2 (19.125), dB2 (19.500), dB2 (19.875), dB2 (20.250), dB2 (20.625), dB2 (21.000)
	};

	s32 tmp;
	s32 fnum, block, TL, KL;

	for (fnum = 0; fnum < 16; fnum++)
		for (block = 0; block < 8; block++)
			for (TL = 0; TL < 64; TL++)
				for (KL = 0; KL < 4; KL++)
				{
					if (KL == 0)
					{
						tllTable[fnum][block][TL][KL] = TL2EG (TL);
					}
					else
					{
						tmp = (s32) (kltable[fnum] - dB2 (3.000) * (7 - block));
						if (tmp <= 0)
							tllTable[fnum][block][TL][KL] = TL2EG (TL);
						else
							tllTable[fnum][block][TL][KL] = (u32) ((tmp >> (3 - KL)) / EG_STEP) + TL2EG (TL);
					}
				}
}

#ifdef USE_SPEC_ENV_SPEED
static double attacktime[16][4] = {
	{0, 0, 0, 0},
	{1730.15, 1400.60, 1153.43, 988.66},
	{865.08, 700.30, 576.72, 494.33},
	{432.54, 350.15, 288.36, 247.16},
	{216.27, 175.07, 144.18, 123.58},
	{108.13, 87.54, 72.09, 61.79},
	{54.07, 43.77, 36.04, 30.90},
	{27.03, 21.88, 18.02, 15.45},
	{13.52, 10.94, 9.01, 7.72},
	{6.76, 5.47, 4.51, 3.86},
	{3.38, 2.74, 2.25, 1.93},
	{1.69, 1.37, 1.13, 0.97},
	{0.84, 0.70, 0.60, 0.54},
	{0.50, 0.42, 0.34, 0.30},
	{0.28, 0.22, 0.18, 0.14},
	{0.00, 0.00, 0.00, 0.00}
};

static double decaytime[16][4] = {
	{0, 0, 0, 0},
	{20926.60, 16807.20, 14006.00, 12028.60},
	{10463.30, 8403.58, 7002.98, 6014.32},
	{5231.64, 4201.79, 3501.49, 3007.16},
	{2615.82, 2100.89, 1750.75, 1503.58},
	{1307.91, 1050.45, 875.37, 751.79},
	{653.95, 525.22, 437.69, 375.90},
	{326.98, 262.61, 218.84, 187.95},
	{163.49, 131.31, 109.42, 93.97},
	{81.74, 65.65, 54.71, 46.99},
	{40.87, 32.83, 27.36, 23.49},
	{20.44, 16.41, 13.68, 11.75},
	{10.22, 8.21, 6.84, 5.87},
	{5.11, 4.10, 3.42, 2.94},
	{2.55, 2.05, 1.71, 1.47},
	{1.27, 1.27, 1.27, 1.27}
};
#endif

/* Rate Table for Attack */
static void
makeDphaseARTable (void)
{
	s32 AR, Rks, RM, RL;
#ifdef USE_SPEC_ENV_SPEED
	u32 attacktable[16][4];

	for (RM = 0; RM < 16; RM++)
		for (RL = 0; RL < 4; RL++)
		{
			if (RM == 0)
				attacktable[RM][RL] = 0;
			else if (RM == 15)
				attacktable[RM][RL] = EG_DP_WIDTH;
			else
				attacktable[RM][RL] = (u32) ((double) (1 << EG_DP_BITS) / (attacktime[RM][RL] * 3579545 / 72000));

		}
#endif

	for (AR = 0; AR < 16; AR++)
		for (Rks = 0; Rks < 16; Rks++)
		{
			RM = AR + (Rks >> 2);
			RL = Rks & 3;
			if (RM > 15)
				RM = 15;
			switch (AR)
			{
			case 0:
				dphaseARTable[AR][Rks] = 0;
				break;
			case 15:
				dphaseARTable[AR][Rks] = 0;/*EG_DP_WIDTH;*/
				break;
			default:
#ifdef USE_SPEC_ENV_SPEED
				dphaseARTable[AR][Rks] = rate_adjust (attacktable[RM][RL]);
#else
				dphaseARTable[AR][Rks] = rate_adjust ((3 * (RL + 4) << (RM + 1)));
#endif
				break;
			}
		}
}

/* Rate Table for Decay and ERELEASE */
static void
makeDphaseDRTable (void)
{
	s32 DR, Rks, RM, RL;

#ifdef USE_SPEC_ENV_SPEED
	u32 decaytable[16][4];

	for (RM = 0; RM < 16; RM++)
		for (RL = 0; RL < 4; RL++)
			if (RM == 0)
				decaytable[RM][RL] = 0;
			else
				decaytable[RM][RL] = (u32) ((double) (1 << EG_DP_BITS) / (decaytime[RM][RL] * 3579545 / 72000));
#endif

	for (DR = 0; DR < 16; DR++)
		for (Rks = 0; Rks < 16; Rks++)
		{
			RM = DR + (Rks >> 2);
			RL = Rks & 3;
			if (RM > 15)
				RM = 15;
			switch (DR)
			{
			case 0:
				dphaseDRTable[DR][Rks] = 0;
				break;
			default:
#ifdef USE_SPEC_ENV_SPEED
				dphaseDRTable[DR][Rks] = rate_adjust (decaytable[RM][RL]);
#else
				dphaseDRTable[DR][Rks] = rate_adjust ((RL + 4) << (RM - 1));
#endif
				break;
			}
		}
}

static void
makeRksTable (void)
{

	s32 fnum8, block, KR;

	for (fnum8 = 0; fnum8 < 2; fnum8++)
		for (block = 0; block < 8; block++)
			for (KR = 0; KR < 2; KR++)
			{
				if (KR != 0)
					rksTable[fnum8][block][KR] = (block << 1) + fnum8;
				else
					rksTable[fnum8][block][KR] = block >> 1;
			}
}

/************************************************************

			Calc Parameters

************************************************************/

__inline static u32
calc_eg_dphase (OPLL_SLOT * slot)
{

	switch (slot->eg_mode)
	{
	case ATTACK:
		return dphaseARTable[slot->patch.AR][slot->rks];

	case DECAY:
		return dphaseDRTable[slot->patch.DR][slot->rks];

	case SUSHOLD:
		return 0;

	case SUSTINE:
		return dphaseDRTable[slot->patch.RR][slot->rks];

	case ERELEASE:
		if (slot->sustine)
			return dphaseDRTable[5][slot->rks];
		else if (slot->patch.EG)
			return dphaseDRTable[slot->patch.RR][slot->rks];
		else
			return dphaseDRTable[7][slot->rks];

	case FINISH:
		return 0;

	default:
		return 0;
	}
}

/*************************************************************

		OPLL internal interfaces

*************************************************************/

#define UPDATE_PG(S)	(S)->dphase = dphaseTable[(S)->fnum][(S)->block][(S)->patch.ML]
#define UPDATE_TLL(S)\
(((S)->type==0)?\
((S)->tll = tllTable[((S)->fnum)>>5][(S)->block][(S)->patch.TL][(S)->patch.KL]):\
((S)->tll = tllTable[((S)->fnum)>>5][(S)->block][(S)->volume][(S)->patch.KL]))
#define UPDATE_RKS(S) (S)->rks = rksTable[((S)->fnum)>>8][(S)->block][(S)->patch.KR]
#define UPDATE_WF(S)	(S)->sintbl = waveform[(S)->patch.WF]
#define UPDATE_EG(S)	(S)->eg_dphase = calc_eg_dphase(S)
#define UPDATE_ALL(S)\
	UPDATE_PG(S);\
	UPDATE_TLL(S);\
	UPDATE_RKS(S);\
	UPDATE_WF(S); \
	UPDATE_EG(S)		/* EG should be updated last. */


/* Slot key on	*/
__inline static void
slotOn (OPLL_SLOT * slot)
{
	slot->eg_mode = ATTACK;
	slot->eg_phase = 0;
	slot->phase = 0;
}

/* Slot key on without reseting the phase */
__inline static void
slotOn2 (OPLL_SLOT * slot)
{
	slot->eg_mode = ATTACK;
	slot->eg_phase = 0;
}

/* Slot key off */
__inline static void
slotOff (OPLL_SLOT * slot)
{
	if (slot->eg_mode == ATTACK)
		slot->eg_phase = EXPAND_BITS (AR_ADJUST_TABLE[HIGHBITS (slot->eg_phase, EG_DP_BITS - EG_BITS)], EG_BITS, EG_DP_BITS);
	slot->eg_mode = ERELEASE;
}

/* Channel key on */
__inline static void
keyOn (OPLL * opll, s32 i)
{
	if (!opll->slot_on_flag[i * 2])
		slotOn (MOD(opll,i));
	if (!opll->slot_on_flag[i * 2 + 1])
		slotOn (CAR(opll,i));
	opll->key_status[i] = 1;
}

/* Channel key off */
__inline static void
keyOff (OPLL * opll, s32 i)
{
	if (opll->slot_on_flag[i * 2 + 1])
		slotOff (CAR(opll,i));
	opll->key_status[i] = 0;
}

/* Set sustine parameter */
__inline static void
setSustine (OPLL * opll, s32 c, s32 sustine)
{
	CAR(opll,c)->sustine = sustine;
	if (MOD(opll,c)->type)
		MOD(opll,c)->sustine = sustine;
}

/* Volume : 6bit ( Volume register << 2 ) */
__inline static void
setVolume (OPLL * opll, s32 c, s32 volume)
{
	CAR(opll,c)->volume = volume;
}

__inline static void
setSlotVolume (OPLL_SLOT * slot, s32 volume)
{
	slot->volume = volume;
}

/* Set F-Number ( fnum : 9bit ) */
__inline static void
setFnumber (OPLL * opll, s32 c, s32 fnum)
{
	CAR(opll,c)->fnum = fnum;
	MOD(opll,c)->fnum = fnum;
}

/* Set Block data (block : 3bit ) */
__inline static void
setBlock (OPLL * opll, s32 c, s32 block)
{
	CAR(opll,c)->block = block;
	MOD(opll,c)->block = block;
}

__inline static void update_key_status (OPLL * opll)
{
	int ch;

	for (ch = 0; ch < 6; ch++)
		opll->slot_on_flag[ch * 2] = opll->slot_on_flag[ch * 2 + 1] = (opll->HiFreq[ch]) & 0x10;
}

/***********************************************************

											Initializing

***********************************************************/

static void
OPLL_SLOT_reset (OPLL_SLOT * slot, int type)
{
	slot->type = type;
	slot->sintbl = waveform[0];
	slot->phase = 0;
	slot->dphase = 0;
	slot->output[0] = 0;
	slot->output[1] = 0;
	slot->feedback = 0;
	slot->eg_mode = SETTLE;
	slot->eg_phase = EG_DP_WIDTH;
	slot->eg_dphase = 0;
	slot->rks = 0;
	slot->tll = 0;
	slot->sustine = 0;
	slot->fnum = 0;
	slot->block = 0;
	slot->volume = 0;
	slot->pgout = 0;
	slot->egout = 0;
}

static void
internal_refresh (void)
{
	makeDphaseTable ();
	makeDphaseARTable ();
	makeDphaseDRTable ();
	pm_dphase = (u32) rate_adjust (PM_SPEED * PM_DP_WIDTH / (clk / 72));
	am_dphase = (u32) rate_adjust (AM_SPEED * AM_DP_WIDTH / (clk / 72));
}

static void
maketables (u32 c, u32 r)
{
	if (c != clk)
	{
		clk = c;
		makePmTable ();
		makeAmTable ();
		makeDB2LinTable ();
		makeAdjustTable ();
		makeTllTable ();
		makeRksTable ();
		makeSinTable ();
		//makeDefaultPatch ();
	}

	if (r != rate)
	{
		rate = r;
		internal_refresh ();
	}
}

OPLL *OPLL_new (u32 clk, u32 rate)
{
	OPLL *opll;

	maketables (clk, rate);

	opll = (OPLL *) calloc (sizeof (OPLL), 1);
	if (opll == NULL)
		return NULL;

	opll->mask = 0;

	OPLL_reset (opll);

	return opll;
}


void
OPLL_delete (OPLL * opll)
{
	free (opll);
}

/* Reset whole of OPLL except patch datas. */
void
OPLL_reset (OPLL * opll)
{
	s32 i;

	if (!opll)
		return;

	opll->adr = 0;
	opll->out = 0;

	opll->pm_phase = 0;
	opll->am_phase = 0;

	opll->mask = 0;

	for (i = 0; i < 12; i++)
		OPLL_SLOT_reset(&opll->slot[i], i%2);

	for (i = 0; i < 6; i++)
	{
		opll->key_status[i] = 0;
		//setPatch (opll, i, 0);
	}

	for (i = 0; i < 0x40; i++)
		OPLL_writeReg (opll, i, 0);

#ifndef EMU2413_COMPACTION
	opll->realstep = (u32) ((1 << 31) / rate);
	opll->opllstep = (u32) ((1 << 31) / (clk / 72));
	opll->oplltime = 0;
#endif
}

/* Force Refresh (When external program changes some parameters). */
void
OPLL_forceRefresh (OPLL * opll)
{
	s32 i;

	if (opll == NULL)
		return;

	for (i = 0; i < 12; i++)
	{
		UPDATE_PG (&opll->slot[i]);
		UPDATE_RKS (&opll->slot[i]);
		UPDATE_TLL (&opll->slot[i]);
		UPDATE_WF (&opll->slot[i]);
		UPDATE_EG (&opll->slot[i]);
	}
}

void
OPLL_set_rate (OPLL * opll, u32 r)
{
	if (opll->quality)
		rate = 49716;
	else
		rate = r;
	internal_refresh ();
	rate = r;
}

void
OPLL_set_quality (OPLL * opll, u32 q)
{
	opll->quality = q;
	OPLL_set_rate (opll, rate);
}

/*********************************************************

								 Generate wave data

*********************************************************/
/* Convert Amp(0 to EG_HEIGHT) to Phase(0 to 2PI). */
#if ( SLOT_AMP_BITS - PG_BITS ) > 0
#define wave2_2pi(e)	( (e) >> ( SLOT_AMP_BITS - PG_BITS ))
#else
#define wave2_2pi(e)	( (e) << ( PG_BITS - SLOT_AMP_BITS ))
#endif

/* Convert Amp(0 to EG_HEIGHT) to Phase(0 to 4PI). */
#if ( SLOT_AMP_BITS - PG_BITS - 1 ) == 0
#define wave2_4pi(e)	(e)
#elif ( SLOT_AMP_BITS - PG_BITS - 1 ) > 0
#define wave2_4pi(e)	( (e) >> ( SLOT_AMP_BITS - PG_BITS - 1 ))
#else
#define wave2_4pi(e)	( (e) << ( 1 + PG_BITS - SLOT_AMP_BITS ))
#endif

/* Convert Amp(0 to EG_HEIGHT) to Phase(0 to 8PI). */
#if ( SLOT_AMP_BITS - PG_BITS - 2 ) == 0
#define wave2_8pi(e)	(e)
#elif ( SLOT_AMP_BITS - PG_BITS - 2 ) > 0
#define wave2_8pi(e)	( (e) >> ( SLOT_AMP_BITS - PG_BITS - 2 ))
#else
#define wave2_8pi(e)	( (e) << ( 2 + PG_BITS - SLOT_AMP_BITS ))
#endif



/* Update AM, PM unit */
static void
update_ampm (OPLL * opll)
{
	opll->pm_phase = (opll->pm_phase + pm_dphase) & (PM_DP_WIDTH - 1);
	opll->am_phase = (opll->am_phase + am_dphase) & (AM_DP_WIDTH - 1);
	opll->lfo_am = amtable[HIGHBITS (opll->am_phase, AM_DP_BITS - AM_PG_BITS)];
	opll->lfo_pm = pmtable[HIGHBITS (opll->pm_phase, PM_DP_BITS - PM_PG_BITS)];
}

/* PG */
__inline static void
calc_phase (OPLL_SLOT * slot, s32 lfo)
{
	if (slot->patch.PM)
		slot->phase += (slot->dphase * lfo) >> PM_AMP_BITS;
	else
		slot->phase += slot->dphase;

	slot->phase &= (DP_WIDTH - 1);

	slot->pgout = HIGHBITS (slot->phase, DP_BASE_BITS);
}

/* EG */
static void
calc_envelope (OPLL_SLOT * slot, s32 lfo)
{
#define S2E(x) (SL2EG((s32)(x/SL_STEP))<<(EG_DP_BITS-EG_BITS))

	static u32 SL[16] = {
		S2E (0.0), S2E (3.0), S2E (6.0), S2E (9.0), S2E (12.0), S2E (15.0), S2E (18.0), S2E (21.0),
		S2E (24.0), S2E (27.0), S2E (30.0), S2E (33.0), S2E (36.0), S2E (39.0), S2E (42.0), S2E (48.0)
	};

	u32 egout;

	switch (slot->eg_mode)
	{

	case ATTACK:
		egout = AR_ADJUST_TABLE[HIGHBITS (slot->eg_phase, EG_DP_BITS - EG_BITS)];
		slot->eg_phase += slot->eg_dphase;
		if((EG_DP_WIDTH & slot->eg_phase)||(slot->patch.AR==15))
		{
			egout = 0;
			slot->eg_phase = 0;
			slot->eg_mode = DECAY;
			UPDATE_EG (slot);
		}
		break;

	case DECAY:
		egout = HIGHBITS (slot->eg_phase, EG_DP_BITS - EG_BITS);
		slot->eg_phase += slot->eg_dphase;
		if (slot->eg_phase >= SL[slot->patch.SL])
		{
			if (slot->patch.EG)
			{
				slot->eg_phase = SL[slot->patch.SL];
				slot->eg_mode = SUSHOLD;
				UPDATE_EG (slot);
			}
			else
			{
				slot->eg_phase = SL[slot->patch.SL];
				slot->eg_mode = SUSTINE;
				UPDATE_EG (slot);
			}
		}
		break;

	case SUSHOLD:
		egout = HIGHBITS (slot->eg_phase, EG_DP_BITS - EG_BITS);
		if (slot->patch.EG == 0)
		{
			slot->eg_mode = SUSTINE;
			UPDATE_EG (slot);
		}
		break;

	case SUSTINE:
	case ERELEASE:
		egout = HIGHBITS (slot->eg_phase, EG_DP_BITS - EG_BITS);
		slot->eg_phase += slot->eg_dphase;
		if (egout >= (1 << EG_BITS))
		{
			slot->eg_mode = FINISH;
			egout = (1 << EG_BITS) - 1;
		}
		break;

	case FINISH:
		egout = (1 << EG_BITS) - 1;
		break;

	default:
		egout = (1 << EG_BITS) - 1;
		break;
	}

	if (slot->patch.AM)
		egout = EG2DB (egout + slot->tll) + lfo;
	else
		egout = EG2DB (egout + slot->tll);

	if (egout >= DB_MUTE)
		egout = DB_MUTE - 1;

	slot->egout = egout;
}

/* CARRIOR */
__inline static s32
calc_slot_car (OPLL_SLOT * slot, s32 fm)
{
	slot->output[1] = slot->output[0];

	if (slot->egout >= (DB_MUTE - 1))
	{
		slot->output[0] = 0;
	}
	else
	{
		slot->output[0] = DB2LIN_TABLE[slot->sintbl[(slot->pgout+wave2_8pi(fm))&(PG_WIDTH-1)] + slot->egout];
	}

	return (slot->output[1] + slot->output[0]) >> 1;
}

/* MODULATOR */
__inline static s32
calc_slot_mod (OPLL_SLOT * slot)
{
	s32 fm;

	slot->output[1] = slot->output[0];

	if (slot->egout >= (DB_MUTE - 1))
	{
		slot->output[0] = 0;
	}
	else if (slot->patch.FB != 0)
	{
		fm = wave2_4pi (slot->feedback) >> (7 - slot->patch.FB);
		slot->output[0] = DB2LIN_TABLE[slot->sintbl[(slot->pgout + fm)&(PG_WIDTH-1)] + slot->egout];
	}
	else
	{
		slot->output[0] = DB2LIN_TABLE[slot->sintbl[slot->pgout] + slot->egout];
	}

	slot->feedback = (slot->output[1] + slot->output[0]) >> 1;

	return slot->feedback;

}

static __inline s16 calc (OPLL * opll)
{
	s32 inst = 0, out = 0;
	s32 i;

	update_ampm (opll);

	for (i = 0; i < 12; i++)
	{
		calc_phase(&opll->slot[i],opll->lfo_pm);
		calc_envelope(&opll->slot[i],opll->lfo_am);
	}

	for (i = 0; i < 6; i++)
		if (!(opll->mask & OPLL_MASK_CH (i)) && (CAR(opll,i)->eg_mode != FINISH))
			inst += calc_slot_car (CAR(opll,i), calc_slot_mod(MOD(opll,i)));

	out = inst;
	return (s16) out;
}

#ifdef EMU2413_COMPACTION
s16
OPLL_calc (OPLL * opll)
{
	return calc (opll);
}
#else
s16
OPLL_calc (OPLL * opll)
{
	if (!opll->quality)
	 return calc (opll);

	while (opll->realstep > opll->oplltime)
	{
		opll->oplltime += opll->opllstep;
		opll->prev = opll->next;
		opll->next = calc (opll);
	}

	opll->oplltime -= opll->realstep;
	opll->out = (s16) (((double) opll->next * (opll->opllstep - opll->oplltime)
			+ (double) opll->prev * opll->oplltime) / opll->opllstep);

	return (s16) opll->out;
}
#endif

u32
OPLL_setMask (OPLL * opll, u32 mask)
{
	u32 ret;

	if (opll)
	{
		ret = opll->mask;
		opll->mask = mask;
		return ret;
	}
	else
		return 0;
}

u32
OPLL_toggleMask (OPLL * opll, u32 mask)
{
	u32 ret;

	if (opll)
	{
		ret = opll->mask;
		opll->mask ^= mask;
		return ret;
	}
	else
		return 0;
}

/****************************************************

											 I/O Ctrl

*****************************************************/

static void setInstrument(OPLL * opll, unsigned int i, unsigned int inst)
{
	const u8 *src;
	OPLL_PATCH *modp, *carp;

	opll->patch_number[i]=inst;

	if(inst)
		src=default_inst[inst-1];
	else
		src=opll->CustInst;

	modp=&MOD(opll,i)->patch;
	carp=&CAR(opll,i)->patch;

	modp->AM=(src[0]>>7)&1;
	modp->PM=(src[0]>>6)&1;
	modp->EG=(src[0]>>5)&1;
	modp->KR=(src[0]>>4)&1;
	modp->ML=(src[0]&0xF);

	carp->AM=(src[1]>>7)&1;
	carp->PM=(src[1]>>6)&1;
	carp->EG=(src[1]>>5)&1;
	carp->KR=(src[1]>>4)&1;
	carp->ML=(src[1]&0xF);

	modp->KL=(src[2]>>6)&3;
	modp->TL=(src[2]&0x3F);

	carp->KL = (src[3] >> 6) & 3;
	carp->WF = (src[3] >> 4) & 1;

	modp->WF = (src[3] >> 3) & 1;

	modp->FB = (src[3]) & 7;

	modp->AR = (src[4]>>4)&0xF;
	modp->DR = (src[4]&0xF);

	carp->AR = (src[5]>>4)&0xF;
	carp->DR = (src[5]&0xF);

	modp->SL = (src[6]>>4)&0xF;
	modp->RR = (src[6]&0xF);

	carp->SL = (src[7]>>4)&0xF;
	carp->RR = (src[7]&0xF);
}


void
OPLL_writeReg (OPLL * opll, u32 reg, u32 data)
{

	s32 i, v, ch;

	data = data & 0xff;
	reg = reg & 0x3f;

	switch (reg)
	{
	 case 0x00:
		opll->CustInst[0]=(u8)data;
		for (i = 0; i < 6; i++)
		{
			if (opll->patch_number[i] == 0)
			{
				setInstrument(opll, i, 0);
				UPDATE_PG (MOD(opll,i));
				UPDATE_RKS (MOD(opll,i));
				UPDATE_EG (MOD(opll,i));
			}
		}
		break;

	case 0x01:
		opll->CustInst[1]=(u8)data;
		for (i = 0; i < 6; i++)
		{
			if (opll->patch_number[i] == 0)
			{
				setInstrument(opll, i, 0);
				UPDATE_PG (CAR(opll,i));
				UPDATE_RKS (CAR(opll,i));
				UPDATE_EG (CAR(opll,i));
			}
		}
		break;

	case 0x02:
		opll->CustInst[2]=(u8)data;
		for (i = 0; i < 6; i++)
		{
			if (opll->patch_number[i] == 0)
			{
				setInstrument(opll, i, 0);
				UPDATE_TLL(MOD(opll,i));
			}
		}
		break;

	case 0x03:
		opll->CustInst[3]=(u8)data;
		for (i = 0; i < 6; i++)
		{
			if (opll->patch_number[i] == 0)
			{
				setInstrument(opll, i, 0);
				UPDATE_WF(MOD(opll,i));
				UPDATE_WF(CAR(opll,i));
			}
		}
		break;

	case 0x04:
		opll->CustInst[4]=(u8)data;
		for (i = 0; i < 6; i++)
		{
			if (opll->patch_number[i] == 0)
			{
				setInstrument(opll, i, 0);
				UPDATE_EG (MOD(opll,i));
			}
		}
		break;

	case 0x05:
		opll->CustInst[5]=(u8)data;
		for (i = 0; i < 6; i++)
		{
			if (opll->patch_number[i] == 0)
			{
				setInstrument(opll, i, 0);
				UPDATE_EG(CAR(opll,i));
			}
		}
		break;

	case 0x06:
		opll->CustInst[6]=(u8)data;
		for (i = 0; i < 6; i++)
		{
			if (opll->patch_number[i] == 0)
			{
				setInstrument(opll, i, 0);
				UPDATE_EG (MOD(opll,i));
			}
		}
		break;

	case 0x07:
		opll->CustInst[7]=(u8)data;
		for (i = 0; i < 6; i++)
		{
			if (opll->patch_number[i] == 0)
			{
				setInstrument(opll, i, 0);
				UPDATE_EG (CAR(opll,i));
			}
		}
		break;

	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
		ch = reg - 0x10;
		opll->LowFreq[ch]=(u8)data;
		setFnumber (opll, ch, data + ((opll->HiFreq[ch] & 1) << 8));
		UPDATE_ALL (MOD(opll,ch));
		UPDATE_ALL (CAR(opll,ch));
		break;

	case 0x20:
	case 0x21:
	case 0x22:
	case 0x23:
	case 0x24:
	case 0x25:
		ch = reg - 0x20;
		opll->HiFreq[ch]=(u8)data;

		setFnumber (opll, ch, ((data & 1) << 8) + opll->LowFreq[ch]);
		setBlock (opll, ch, (data >> 1) & 7);
		setSustine (opll, ch, (data >> 5) & 1);
		if (data & 0x10)
			keyOn (opll, ch);
		else
			keyOff (opll, ch);
		UPDATE_ALL (MOD(opll,ch));
		UPDATE_ALL (CAR(opll,ch));
		update_key_status (opll);
		break;

	case 0x30:
	case 0x31:
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x35:
		opll->InstVol[reg-0x30]=(u8)data;
		i = (data >> 4) & 15;
		v = data & 15;
		setInstrument(opll, reg-0x30, i);
		setVolume (opll, reg - 0x30, v << 2);
		UPDATE_ALL (MOD(opll,reg - 0x30));
		UPDATE_ALL (CAR(opll,reg - 0x30));
		break;

	default:
		break;

	}
}

// Konami VRC7, based on the YM2413

OPLL *OPL = NULL;

void	VRC7sound_Load (void)
{
	if (OPL != NULL)
	{
//		MessageBox(hWnd,_T("YM2413 already created!"),_T("VRC7"),MSGBOX_FLAGS);
		log_printf("YM2413 already created!\n");
		return;
	}
	else
	{
		OPL = OPLL_new(3579545,44100);
		if (OPL == NULL)
		{
//			MessageBox(hWnd,_T("Unable to create YM2413!"),_T("VRC7"),MSGBOX_FLAGS);
			log_printf("Unable to create YM2413!\n");
			return;
		}
	}
}

void	VRC7sound_Reset (void)
{
}

void	VRC7sound_Unload (void)
{
	if (OPL)
	{
		OPLL_delete(OPL);
		OPL = NULL;
	}
//	else	MessageBox(hWnd,_T("Unable to destroy YM2413!"),_T("VRC7"),MSGBOX_FLAGS);
}

void	VRC7sound_Write (int Addr, int Val)
{
	static unsigned char addr = 0;
	switch (Addr & 0xF030)
	{
	case 0x9010:	addr = Val;			break;
	case 0x9030:	OPLL_writeReg(OPL,addr,Val);	break;
	}
}

int	VRC7sound_Get (int numCycles)
{
	return OPLL_calc(OPL) << 3;	// currently don't use numCycles
}

int	VRC7sound_SaveLoad (int mode, int x, unsigned char *data)
{
/*	if (mode == STATE_SAVE)
	{
		memcpy(data+x,OPL,sizeof(OPLL));
		x += sizeof(OPLL);
	}
	else if (mode == STATE_LOAD)
	{
		int i;
		memcpy(OPL,data+x,sizeof(OPLL));
		x += sizeof(OPLL);
		for (i = 0; i < 6; i++)
		{
			UPDATE_ALL(MOD(OPL,i));
			UPDATE_ALL(CAR(OPL,i));
		}
	}
	else if (mode == STATE_SIZE)
		x += sizeof(OPLL);
	else MessageBox(hWnd,_T("Invalid save/load type!"),_T(__FILE__),MB_OK);
*/	return x;
}
