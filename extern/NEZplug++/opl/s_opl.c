/*
  s_opl.c -- YM2413/Y8950/YM3526/YM3812 emulator by Mamiya, 2001.

  References: 
    fmopl.c        -- 1999,2000 written by Tatsuyuki Satoh (MAME development).
    fmopl.c(fixed) -- 2000 modified by mamiya (NEZplug development).
    fmgen.cpp      -- 1999-2001 written by cisc.
    emu2413.c      -- a YM2413 emulator : written by Mitsutaka Okazaki 2001
    fmpac.ill      -- 2000 created by sama. 
    fmpac.ill      -- 2000 created by NARUTO. 
    fmpac.ill      -- 2001 created by Okazaki. 
    YM2413 application manual
*/


#include "kmsnddev.h"
#include "divfix.h"
#include "s_logtbl.h"
#include "s_opltbl.h"
#include "s_opl.h"
#include "s_deltat.h"

#define PG_SHIFT 13 /* fix */
#define CPS_SHIFTE 20
#define CPS_SHIFTP 14
#define LFO_SHIFT 16
#define DP_BITS 23
#define NOISE_BITS 14

#define OUTPUT_BUFFER 2

#define OPLL_INST_WORK  0x40
#define OPLL_INST_WORK2 (OPLL_INST_WORK + 8 * 0x13)

#define AR_BITS   6 /* fix */
#define AR_SHIFT 14 /* fix */
#define EG_SHIFT 15 /* fix */
#define EG_SHIFT2 32768
#define NOISE_SHIFT 9
#define OUTPUT_VOL 1/2

#define FM_FREQ 44100.0
#define VOLUME_BITS 0

//1にすると、ローパスフィルタがかかるが、重すぎて使い物になりまへん。
#define LOWPASS_FILTER 0


//#define AR_PHASEMAX 0x7fffffff
#define AR_PHASEMAX (((1 << AR_BITS) - 1) << AR_SHIFT)
#define EG_PHASEMAX (127 << EG_SHIFT)
#define EG_KEYOFF   (128 << EG_SHIFT)
#define LOG_KEYOFF   (31 << (LOG_BITS + 1))

#if 0
  0-48dB
  AR_BITS=6
  AR_SHIFT=14
  x = FC000h(20.7618(sec) * 3579545 / 72)cycles 63 * 4000h
  x / 3 * 4 1730.15(ms)
  x / 3 * 256 27.03(ms)

  EG_BITS=7
  EG_SHIFT=15
  x = 3F8000h(83.7064(sec) * 3579545 / 72)cycles 127 * 8000h
  x / 4 = 20926.6(ms)
#endif


#define TESTING_OPTIMIZE_AME 0
#define USE_FBBUF 1

typedef struct {
	Uint32 phase;
	Uint32 spd;
	Uint32 pgout;
} OPL_PG;

enum {
	EG_MODE_ATTACK,
	EG_MODE_DECAY,
	EG_MODE_SUSTINE,
	EG_MODE_RELEASE,
	EG_MODE_NUM,
	EG_MODE_SUSHOLD,
	EG_MODE_OFF = EG_MODE_NUM
};

typedef struct {
	Uint32 phasear;
	Uint32 phasearc;
	Uint32 phase;
	Uint32 spd[EG_MODE_NUM];
	Uint32 dr_phasemax;
	Uint8 mode;
	Uint8 pad4_1;
	Uint8 pad4_2;
	Uint8 pad4_3;
} OPL_EG;

enum {
	FLAG_AME		= (1 << 0),
	FLAG_PME		= (1 << 1),
	FLAG_EGT		= (1 << 2),
	FLAG_KSR		= (1 << 3)
};

typedef struct {
	OPL_PG pg;
	OPL_EG eg;
	Int32 input;
#if USE_FBBUF
	Int32 fbbuf;
#endif
#if TESTING_OPTIMIZE_AME
	Uint32 *amin;
#endif
	Uint32 tl_ofs;
	Uint32 *sintable;

	Uint8 modcar;	/* 1:m 0:c */
	Uint8 fb;
	Uint8 lvl;
	Uint8 nst;

	Uint8 tll;
	Uint8 key;
	Uint8 rkey;
	Uint8 prevkey;

	Uint8 enable;
	Uint8 __enablehold__;
	Uint8 flag;
	Uint8 ksr;

	Uint8 mul;
	Uint8 ksl;
	Uint8 ar;
	Uint8 dr;
	Uint8 sl;
	Uint8 rr;
	Uint8 tl;
	Uint8 wf;
	Uint8 type;
	Uint8 *tone;
	Int32 outputbf[OUTPUT_BUFFER];
} OPL_OP;

typedef struct {
	OPL_OP op[2];
	Uint8 con;
	Uint8 freql;
	Uint8 freqh;
	Uint8 blk;
	Uint8 kcode;
	Uint8 sus;
	Uint8 ksb;
	Uint8 pad4_3;
} OPL_CH;

enum {
	LFO_UNIT_AM,
	LFO_UNIT_PM,
	LFO_UNIT_NUM
};

typedef struct {
	Uint32 output;
	Uint32 cnt;
	Uint32 sps;	/* step per sample */
	Uint32 adr;
	Uint32 adrmask;
	Uint32 *table;
} OPL_LFO;

typedef struct {
	KMIF_SOUND_DEVICE kmif;
	KMIF_SOUND_DEVICE *deltatpcm;
	KMIF_LOGTABLE *logtbl;
	KMIF_OPLTABLE *opltbl;
	OPL_CH ch[9];
	OPL_LFO lfo[LFO_UNIT_NUM];
	struct OPLSOUND_COMMON_TAG {
		Uint32 cpsp;
		Uint32 cnt;
		Uint32 *ar_table;
		Uint32 *tll2logtbl;
#if TESTING_OPTIMIZE_AME
		Uint32 amzero;
#endif
		Int32 mastervolume;
		Uint32 sintablemask;
		Uint32 ratetbla[4];
		Uint32 ratetbl[4];
		Uint8 adr;
		Uint8 wfe;
		Uint8 rc;
		Uint8 rmode;
		Uint8 enable;
	} common;
	Uint8 regs[0x100];
	Uint8 oplregs[0x100];
	Uint8 opllregs[0x100];
	Uint8 opl_type;
	Uint32 freqp;
	Uint32 freq;
	long output;
	Int32 fbbuf;
	double fb;
	Uint32 rng;
	Uint32 phase;
} OPLSOUND;

static Uint8 romtone[3][16 * 19] =
{
	{
#include "ill/i_fmpac.h"
	},
	{
#include "ill/i_fmunit.h" // ←SMSのFM音源はFM-PACと同じ「YM2413」を使ってるのであんまり意味が無い
	},
	{
#include "ill/i_vrc7.h" // ←VRC7は明らかに音色が違う。ラグランジュポイントをYM2413で鳴らしたら、ベース音が残念な音色になった。
	},
};

#if (((-1) >> 1) == -1)
/* RIGHT SHIFT IS SIGNED */
#define SSR(x, y) (((Int32)x) >> (y))
#else
/* RIGHT SHIFT IS UNSIGNED */
#define SSR(x, y) (((x) >= 0) ? ((x) >> (y)) : (-((-(x) - 1) >> (y)) - 1))
#endif

const static Uint8 ksltable[4]={15,2,1,0};
const static Uint8 hhouttable[8]={
	0x34, //010
	0x0d, //001
	0x8d, //101
	0xb4, //110
	0x8d, //101
	0xb4, //110
	0x8d, //101
	0xb4, //110
};
const static Uint8 rymouttable[4]={
	      
	0x40, //01
	0xc0, //11
	0xc0, //11
	0xc0, //11
};

__inline static void SetOpOff(OPL_OP *opp)
{
	opp->eg.mode = EG_MODE_OFF;
	opp->eg.phase = EG_KEYOFF;
	opp->enable = 0;
}
#define CalcFbbuf(x,y)	CalcFbbuf2(x,y)
#if LOWPASS_FILTER
#define CalcFbbuf2(x,y) {\
	x = (x + y / sndp->fb) / (1+1/sndp->fb);\
}
#else
#define CalcFbbuf2(x,y) {\
	x = (x + y) / 2;\
}
#endif

__inline static Int32 CalcOutputBuffer(Int32 output, Int32 *outputbf){
	Uint8 a;
	Int64 buffer,c;
	for(a=OUTPUT_BUFFER-1;a>0;a--) outputbf[a]=outputbf[a-1];
	outputbf[0] = output;
	buffer=0;c=0;
	for(a=0;a<OUTPUT_BUFFER;a++)buffer+=outputbf[a];
	return buffer/OUTPUT_BUFFER;
}

__inline static void EgStep(OPLSOUND *sndp, OPL_OP *opp)
{
	switch (opp->eg.mode)
	{
		default:
			NEVER_REACH
		case EG_MODE_ATTACK:
			opp->eg.phase = sndp->common.ar_table[opp->eg.phasear >> (AR_SHIFT + AR_BITS - ARTBL_BITS)] >> (ARTBL_SHIFT -  EG_SHIFT);
			opp->eg.phasear += opp->eg.spd[EG_MODE_ATTACK];
			if (opp->eg.phasear >= AR_PHASEMAX)
			{
				opp->eg.mode = EG_MODE_DECAY;
				opp->eg.phase = 0;
			}
			break;
		case EG_MODE_DECAY:
			opp->eg.phase += opp->eg.spd[EG_MODE_DECAY];
			if (opp->eg.phase >= opp->eg.dr_phasemax)
			{
				opp->eg.phase = opp->eg.dr_phasemax;
				opp->eg.mode = (opp->flag & FLAG_EGT) ? EG_MODE_SUSHOLD : EG_MODE_SUSTINE;
			}
			break;
		case EG_MODE_SUSTINE:
		case EG_MODE_RELEASE:
			opp->eg.phase += opp->eg.spd[opp->eg.mode];
			if (opp->eg.phase >= EG_PHASEMAX){
				SetOpOff(opp);
			}
			break;
		case EG_MODE_SUSHOLD:
			break;
		case EG_MODE_OFF:
			break;
	}
}

static void __fastcall OpStep(OPLSOUND *sndp, OPL_OP *opp)
{
	EgStep(sndp, opp);
	if (opp->flag & FLAG_PME)
		opp->pg.phase += (opp->pg.spd * sndp->lfo[LFO_UNIT_PM].output) >> PM_SHIFT;
	else
		opp->pg.phase += opp->pg.spd;
	opp->pg.phase &= (1<<DP_BITS)-1;
	opp->pg.pgout = opp->pg.phase >> (DP_BITS - NOISE_SHIFT);
}

__inline static void OpStepNG(OPLSOUND *sndp)
{
	sndp->phase += sndp->common.cpsp;
	while (sndp->phase >= 1<<NOISE_BITS)
	{
		sndp->phase -= 1<<NOISE_BITS;
//		sndp->rng ^= ((sndp->rng & 1) << 16) + ((sndp->rng & 1) << 13);
		if(sndp->rng & 1)sndp->rng^= 0x8003020;
		sndp->rng >>= 1;
	}
}


__inline static void OpSynthMod(OPLSOUND *sndp, OPL_OP *opp)
{
	if (opp->enable)
	{
		Uint32 tll;
		Int32 output;
		OpStep(sndp, opp);
		tll = opp->tll + (opp->eg.phase >> EG_SHIFT);
		tll = (tll >= (1 << TLLTBL_BITS)) ? LOG_KEYOFF : sndp->common.tll2logtbl[tll];
		tll += opp->tl_ofs;
#if TESTING_OPTIMIZE_AME
		tll += *opp->amin;
#else
		if (opp->flag & FLAG_AME) tll += sndp->lfo[LFO_UNIT_AM].output;
#endif
		tll += opp->sintable[sndp->common.sintablemask & (opp->input + (opp->pg.phase >> PG_SHIFT))];
		output = LogToLin(sndp->logtbl, tll, -8 + ((LOG_LIN_BITS + 1) - (SINTBL_BITS + 2))-1);
		if (opp->fb)
		{
#if USE_FBBUF == 1
			Int32 fbtmp;
			fbtmp = opp->fbbuf + output;
			opp->fbbuf = output;
			opp->input = SSR(fbtmp, (9 - opp->fb));
#else
#if USE_FBBUF == 2
			opp->fbbuf = CalcOutputBuffer(output, opp->outputbf);
			opp->input = SSR(opp->fbbuf, (8 - opp->fb));
#else
			opp->input = SSR(output, (8 - opp->fb));
#endif
#endif
		}
		opp[1].input = output;
	}
}
/*
__inline static void OpSynthMod(OPLSOUND *sndp, OPL_OP *opp)
{
	if (opp->enable)
	{
		Uint32 tll;
		Int32 output;
		OpStep(sndp, opp);
		tll = opp->tll + (opp->eg.phase >> EG_SHIFT);
		tll = (tll >= (1 << TLLTBL_BITS)) ? LOG_KEYOFF : sndp->common.tll2logtbl[tll];
		tll += opp->tl_ofs;
#if TESTING_OPTIMIZE_AME
		tll += *opp->amin;
#else
		if (opp->flag & FLAG_AME) tll += sndp->lfo[LFO_UNIT_AM].output;
#endif
		tll += opp->sintable[sndp->common.sintablemask & (opp->input + (opp->pg.phase >> PG_SHIFT))];
		output = LogToLin(sndp->logtbl, tll, -8 + ((LOG_LIN_BITS + 1) - (SINTBL_BITS + 2)));
		if (opp->fb)
		{
#if USE_FBBUF
			Int32 fbtmp;
			fbtmp = opp->fbbuf + output;
			opp->fbbuf = output;
			opp->input = SSR(fbtmp, (9 - opp->fb));
#else
			opp->input = SSR(output, (8 - opp->fb));
#endif
		}
		opp[1].input = output;
	}
}
*/

__inline static Int32 OpSynthCarFb(OPLSOUND *sndp, OPL_OP *opp)
{
	if (opp->enable)
	{
		Uint32 tll;
		OpStep(sndp, opp);
		tll = opp->tll + (opp->eg.phase >> EG_SHIFT);
		tll = (tll >= (1 << TLLTBL_BITS)) ? LOG_KEYOFF : sndp->common.tll2logtbl[tll];
		tll += opp->tl_ofs;
#if TESTING_OPTIMIZE_AME
		tll += *opp->amin;
#else
		if (opp->flag & FLAG_AME) tll += sndp->lfo[LFO_UNIT_AM].output;
#endif
		tll += opp->sintable[sndp->common.sintablemask & (opp->input + (opp->pg.phase >> PG_SHIFT))];
		if (opp->fb)
		{
#if USE_FBBUF
			Int32 output, fbtmp;
			output = LogToLin(sndp->logtbl, tll, -8 + ((LOG_LIN_BITS + 1) - (SINTBL_BITS + 2)) -1);
			fbtmp = opp->fbbuf + output;
			opp->fbbuf = output;
			opp->input = SSR(fbtmp, (9 - opp->fb));
#else
			Int32 output;
			output = LogToLin(sndp->logtbl, tll, -8 + ((LOG_LIN_BITS + 1) - (SINTBL_BITS + 2)) -1);
			opp->input = SSR(output, (8 - opp->fb));
#endif
		}
		return LogToLin(sndp->logtbl, tll + sndp->common.mastervolume, -8 + LOG_LIN_BITS - 19 - opp->lvl);
	}
	return 0;
}
/*
__inline static Int32 OpSynthCarFb(OPLSOUND *sndp, OPL_OP *opp)
{
	if (opp->enable)
	{
		Uint32 tll;
		OpStep(sndp, opp);
		tll = opp->tll + (opp->eg.phase >> EG_SHIFT);
		tll = (tll >= (1 << TLLTBL_BITS)) ? LOG_KEYOFF : sndp->common.tll2logtbl[tll];
		tll += opp->tl_ofs;
#if TESTING_OPTIMIZE_AME
		tll += *opp->amin;
#else
		if (opp->flag & FLAG_AME) tll += sndp->lfo[LFO_UNIT_AM].output;
#endif
		tll += opp->sintable[sndp->common.sintablemask & (opp->input + (opp->pg.phase >> PG_SHIFT))];
		if (opp->fb)
		{
#if USE_FBBUF
			Int32 output, fbtmp;
			output = LogToLin(sndp->logtbl, tll, -8 + ((LOG_LIN_BITS + 1) - (SINTBL_BITS + 2)));
			fbtmp = opp->fbbuf + output;
			opp->fbbuf = output;
			opp->input = SSR(fbtmp, (9 - opp->fb));
#else
			Int32 output;
			output = LogToLin(sndp->logtbl, tll, -8 + ((LOG_LIN_BITS + 1) - (SINTBL_BITS + 2)));
			opp->input = SSR(output, (8 - opp->fb));
#endif
		}
		return LogToLin(sndp->logtbl, tll + sndp->common.mastervolume, -8 + LOG_LIN_BITS - 19 - opp->lvl);
	}
	return 0;
}
*/

__inline static Int32 OpSynthCar(OPLSOUND *sndp, OPL_OP *opp)
{
	OpStep(sndp, opp);
	if (opp->enable)
	{
		Uint32 tll;
		tll = opp->tll + (opp->eg.phase >> EG_SHIFT);
		tll = (tll >= (1 << TLLTBL_BITS)) ? LOG_KEYOFF : sndp->common.tll2logtbl[tll];
		tll += opp->tl_ofs;
#if TESTING_OPTIMIZE_AME
		tll += *opp->amin;
#else
		if (opp->flag & FLAG_AME) tll += sndp->lfo[LFO_UNIT_AM].output;
#endif
		tll += opp->sintable[sndp->common.sintablemask & (opp->input + (opp->pg.phase >> PG_SHIFT))];
		return LogToLin(sndp->logtbl, tll + sndp->common.mastervolume, -8 + LOG_LIN_BITS - 19 - opp->lvl -1);
	}
	return 0;
}
/*
__inline static Int32 OpSynthCar(OPLSOUND *sndp, OPL_OP *opp)
{
	if (opp->enable)
	{
		Uint32 tll;
		OpStep(sndp, opp);
		tll = opp->tll + (opp->eg.phase >> EG_SHIFT);
		tll = (tll >= (1 << TLLTBL_BITS)) ? LOG_KEYOFF : sndp->common.tll2logtbl[tll];
		tll += opp->tl_ofs;
#if TESTING_OPTIMIZE_AME
		tll += *opp->amin;
#else
		if (opp->flag & FLAG_AME) tll += sndp->lfo[LFO_UNIT_AM].output;
#endif
		tll += opp->sintable[sndp->common.sintablemask & (opp->input + (opp->pg.phase >> PG_SHIFT))];
		return LogToLin(sndp->logtbl, tll + sndp->common.mastervolume, -8 + LOG_LIN_BITS - 19 - opp->lvl);
	}
	return 0;
}
*/

__inline static Int32 OpSynthTom(OPLSOUND *sndp, OPL_OP *opp, OPL_OP *opp2)
{
	if (opp->enable)
	{
		Uint32 tll;
		Int32 output;
		tll = opp->tll + (opp->eg.phase / EG_SHIFT2);
		tll = (tll >= 128 - 16) ? LOG_KEYOFF : sndp->common.tll2logtbl[tll];
		tll += opp->tl_ofs;
		tll += opp->sintable[sndp->common.sintablemask & (opp->pg.phase >> PG_SHIFT)];
		output = LogToLin(sndp->logtbl, tll + sndp->common.mastervolume, -8 + LOG_LIN_BITS - 18 - opp->lvl);
		return output;
	}
	return 0;
}


static Int32 __fastcall OpSynthRym(OPLSOUND *sndp, OPL_OP *opp, OPL_OP *opp2)
{
/*
	if (opp2->enable)
	{
		Uint32 tll,c1,c2;
		Int32 output=0;
		tll = opp2->tll + (opp2->eg.phase / EG_SHIFT2);
		tll = (tll >= (1 << TLLTBL_BITS)) ? LOG_KEYOFF : sndp->common.tll2logtbl[tll];
		tll += opp2->tl_ofs;
		c1 = (((opp->pg.pgout >> (NOISE_SHIFT - 8)) & 1) 
			^ ((opp->pg.pgout >> (NOISE_SHIFT - 3)) & 1)) 
			| ((opp->pg.pgout >> (NOISE_SHIFT - 7)) & 1);
		c2 = (((opp2->pg.pgout >> (NOISE_SHIFT - 7)) & 1)
			^ ((opp2->pg.pgout >> (NOISE_SHIFT - 5)) & 1));
		output = LogToLin(sndp->logtbl, tll + sndp->common.mastervolume + LinToLog(sndp->logtbl, rymouttable[(c1<<1) | c2]), -13 + LOG_LIN_BITS - 18 - opp2->lvl);

		return output;
	}
	return 0;
*/
//	OpStep(sndp, opp);
	if (opp2->enable)
	{
		Uint32 tll;
		Int32 output;
		tll = opp2->tll + (opp2->eg.phase / EG_SHIFT2);
		tll = (tll >= (1 << TLLTBL_BITS)) ? LOG_KEYOFF : sndp->common.tll2logtbl[tll];
		tll += opp2->tl_ofs;
		if( (((opp->pg.pgout >> (NOISE_SHIFT - 8)) & 1) 
			^ ((opp->pg.pgout >> (NOISE_SHIFT - 1)) & 1) 
			| ((opp->pg.pgout >> (NOISE_SHIFT - 7)) & 1)) 
			^ ( ((opp2->pg.pgout >> (NOISE_SHIFT - 7)) & 1)
			& !((opp2->pg.pgout >> (NOISE_SHIFT - 5)) & 1))
		)tll += 1;
//		tll += (sndp->rng & 1);
//		if(sndp->ch[7].op[0].rkey)
		output = LogToLin(sndp->logtbl, tll + sndp->common.mastervolume, -8 + LOG_LIN_BITS - 20 - opp2->lvl);
		return output;
	}
	return 0;

}

static Int32 __fastcall OpSynthHat(OPLSOUND *sndp, OPL_OP *opp, OPL_OP *opp2)
{
/*
	if (opp->enable)
	{
		Uint32 tll,c1,c2,c3;
		Int32 output=0;
		tll = opp->tll + (opp->eg.phase / EG_SHIFT2);
		tll = (tll >= (1 << TLLTBL_BITS)) ? LOG_KEYOFF : sndp->common.tll2logtbl[tll];
		tll += opp->tl_ofs;
		c1 = (((opp->pg.pgout >> (NOISE_SHIFT - 8)) & 1) 
			^ ((opp->pg.pgout >> (NOISE_SHIFT - 3)) & 1))
			| ((opp->pg.pgout >> (NOISE_SHIFT - 7)) & 1);
		c2 = (((opp2->pg.pgout >> (NOISE_SHIFT - 7)) & 1)
			^ ((opp2->pg.pgout >> (NOISE_SHIFT - 5)) & 1));
		c3 = (sndp->rng & 1);
		output = LogToLin(sndp->logtbl, tll + sndp->common.mastervolume + LinToLog(sndp->logtbl, hhouttable[(c1<<2) | (c2<<1) | c3]), -13 + LOG_LIN_BITS - 16 - opp->lvl)*3;

		return output;
	}
	return 0;
*/
//	OpStep(sndp, opp);
	if (opp->enable)
	{
		Uint32 tll;
		Int32 output=0;
		tll = opp->tll + (opp->eg.phase / EG_SHIFT2);
		tll = (tll >= (1 << TLLTBL_BITS)) ? LOG_KEYOFF : sndp->common.tll2logtbl[tll];
		tll += opp->tl_ofs;
		if( (((opp->pg.pgout >> (NOISE_SHIFT - 8)) & 1) 
			^ ((opp->pg.pgout >> (NOISE_SHIFT - 1)) & 1) 
			| ((opp->pg.pgout >> (NOISE_SHIFT - 7)) & 1)) 
			^ ( ((opp2->pg.pgout >> (NOISE_SHIFT - 7)) & 1)
			& !((opp2->pg.pgout >> (NOISE_SHIFT - 5)) & 1))
		)tll += 1;
		output = LogToLin(sndp->logtbl, tll + sndp->common.mastervolume, -8 + LOG_LIN_BITS - 19 - opp->lvl);

//		tll = opp->tll + (opp->eg.phase / EG_SHIFT2);
//		tll = (tll >= (1 << TLLTBL_BITS)) ? LOG_KEYOFF : sndp->common.tll2logtbl[tll];
//		tll += opp->tl_ofs;
		output += LogToLin(sndp->logtbl, tll + sndp->common.mastervolume + (sndp->rng & 1), -8 + LOG_LIN_BITS - 19 - opp->lvl);
		return output;
	}
	return 0;
}

static Int32 __fastcall OpSynthSnr(OPLSOUND *sndp, OPL_OP *opp, OPL_OP *opp2)
{
	if (opp2->enable)
	{
		Uint32 tll;
		Int32 output=0,outval;

		tll = opp2->tll + (opp2->eg.phase / EG_SHIFT2);
		tll = (tll >= (1 << TLLTBL_BITS)-16) ? LOG_KEYOFF : sndp->common.tll2logtbl[tll];
		tll += opp2->tl_ofs;
		outval = LogToLin(sndp->logtbl, tll + sndp->common.mastervolume + 1, -8 + LOG_LIN_BITS - 17 - opp2->lvl);

		return outval * ((((((opp->pg.pgout) >> 7) & 1))*2-1) + ((sndp->rng & 1) * 3-1));
	}
	return 0;
}



__inline static void LfoStep(OPL_LFO *lfop)
{
	lfop->cnt += lfop->sps;
	lfop->adr += lfop->cnt >> LFO_SHIFT;
	lfop->cnt &= (1 << LFO_SHIFT) - 1;
	lfop->output = lfop->table[lfop->adr & lfop->adrmask];
}

static void sndsynth(OPLSOUND *sndp, Int32 *p)
{
	long accum[2] = { 0, 0 };
//	int i = 0 ,count;
//	count = (FM_FREQ-sndp->freqp)/sndp->freq;
//	if(count){
//		sndp->output = 0;
//		for(i=0;sndp->freqp < FM_FREQ;sndp->freqp += sndp->freq ,i++){
//			accum[0] = 0;
			if (sndp->common.enable)
			{
				Uint32 i, rch;
				OpStepNG(sndp);
				for (i = 0; i < LFO_UNIT_NUM; i++) LfoStep(&sndp->lfo[i]);
				rch = sndp->opl_type == OPL_TYPE_VRC7 ? 6 : (sndp->common.rmode ? 7 : 9);
				if(rch==9)
					for (i = 0; i < rch; i++)
					{
						if (sndp->ch[i].op[0].modcar)
							OpSynthMod(sndp, &sndp->ch[i].op[0]);
						else
							accum[0] += OpSynthCarFb(sndp, &sndp->ch[i].op[0]) * chmask[DEV_YM2413_CH1+i];
						accum[0] += OpSynthCar(sndp, &sndp->ch[i].op[1]) * chmask[DEV_YM2413_CH1+i];
					}
				else
					for (i = 0; i < rch; i++)
					{
						if (sndp->ch[i].op[0].modcar)
							OpSynthMod(sndp, &sndp->ch[i].op[0]);
						else{
							if(i==6) accum[0] += OpSynthCarFb(sndp, &sndp->ch[i].op[0]) * chmask[DEV_YM2413_BD];
							else     accum[0] += OpSynthCarFb(sndp, &sndp->ch[i].op[0]) * chmask[DEV_YM2413_CH1+i];
						}

						if(i==6) accum[0] += OpSynthCar(sndp, &sndp->ch[i].op[1]) * chmask[DEV_YM2413_BD];
						else     accum[0] += OpSynthCar(sndp, &sndp->ch[i].op[1]) * chmask[DEV_YM2413_CH1+i];
					}

				if (sndp->common.rmode)
				{
					OpStep(sndp, &sndp->ch[7].op[0]);
					OpStep(sndp, &sndp->ch[7].op[1]);
					OpStep(sndp, &sndp->ch[8].op[0]);
					OpStep(sndp, &sndp->ch[8].op[1]);
					accum[0] += OpSynthHat (sndp, &sndp->ch[7].op[0], &sndp->ch[8].op[1]) * chmask[DEV_YM2413_HH];
					accum[0] += OpSynthSnr (sndp, &sndp->ch[7].op[0], &sndp->ch[7].op[1]) * chmask[DEV_YM2413_SD];
					accum[0] += OpSynthTom (sndp, &sndp->ch[8].op[0], &sndp->ch[8].op[1]) * chmask[DEV_YM2413_TOM];
					accum[0] += OpSynthRym (sndp, &sndp->ch[7].op[0], &sndp->ch[8].op[1]) * chmask[DEV_YM2413_TCY];
				}
			}
			if (sndp->deltatpcm)
			{
				sndp->deltatpcm->synth(sndp->deltatpcm->ctx, accum);
			}
/*		#if USE_FBBUF
			{
				sndp->fbbuf /= 10;
				sndp->fbbuf  = sndp->fbbuf*9 + accum[0]/10;
				accum[0] = sndp->fbbuf;
			}
		#endif
*/
//			sndp->output += accum[0];
//		}
//	}
//	sndp->freqp -= FM_FREQ;
	
	sndp->output = SSR(accum[0],VOLUME_BITS);
//	sndp->output = (sndp->output / count)>>VOLUME_BITS;
	#if 0
		/* NISE DAC */
		if (sndp->output >= 0)
			sndp->output +=  (long)(((unsigned long) sndp->output) & (((1 << 16) - 1) << (23 - 8)));
		else
			sndp->output += -(long)(((unsigned long)-sndp->output +1) & (((1 << 16) - 1) << (23 - 8)));
	#endif
	#if 1
		CalcFbbuf2(sndp->fbbuf,sndp->output);
//		sndp->output += sndp->fbbuf;
	#endif
	p[0] += sndp->output;
	p[1] += sndp->output;
}

static void sndvolume(OPLSOUND *sndp, Int32 volume)
{
	if (sndp->deltatpcm) sndp->deltatpcm->volume(sndp->deltatpcm->ctx, volume);
	volume = (volume << (LOG_BITS - 8)) << 1;
	sndp->common.mastervolume = volume;
}

const static Uint8 op_table[0x20]=
{
	   0,   2,   4,   1,   3,   5,0xff,0xff,
	   6,   8,  10,   7,   9,  11,0xff,0xff,
	  12,  14,  16,  13,  15,  17,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
};

const static Uint8 mul_table[0x10]=
{
	 1+0, 2+0, 4+0, 2+4, 8+0, 8+2, 8+4,16-2,
	16+0,16+2,16+4,16+4,16+8,16+8,32-2,32-2,
};

#define DB2TLL(x) (x * 2 / 375)
const static Uint32 ksl_table[8][16]=
{
	{
		DB2TLL(    0), DB2TLL(    0), DB2TLL(    0), DB2TLL(    0),
		DB2TLL(    0), DB2TLL(    0), DB2TLL(    0), DB2TLL(    0),
		DB2TLL(    0), DB2TLL(    0), DB2TLL(    0), DB2TLL(    0),
		DB2TLL(    0), DB2TLL(    0), DB2TLL(    0), DB2TLL(    0),
	},{
		DB2TLL(    0), DB2TLL(    0), DB2TLL(    0), DB2TLL(    0),
		DB2TLL(    0), DB2TLL(    0), DB2TLL(    0), DB2TLL(    0),
		DB2TLL(    0), DB2TLL(  750), DB2TLL( 1125), DB2TLL( 1500),
		DB2TLL( 1875), DB2TLL( 2250), DB2TLL( 2625), DB2TLL( 3000),
	},{
		DB2TLL(    0), DB2TLL(    0), DB2TLL(    0), DB2TLL(    0),
		DB2TLL(    0), DB2TLL( 1125), DB2TLL( 1875), DB2TLL( 2625),
		DB2TLL( 3000), DB2TLL( 3750), DB2TLL( 4125), DB2TLL( 4500),
		DB2TLL( 4875), DB2TLL( 5250), DB2TLL( 5625), DB2TLL( 6000),
	},{
		DB2TLL(    0), DB2TLL(    0), DB2TLL(    0), DB2TLL( 1875),
		DB2TLL( 3000), DB2TLL( 4125), DB2TLL( 4875), DB2TLL( 5625),
		DB2TLL( 6000), DB2TLL( 6750), DB2TLL( 7125), DB2TLL( 7500),
		DB2TLL( 7875), DB2TLL( 8250), DB2TLL( 8625), DB2TLL( 9000),
	},{
		DB2TLL(    0), DB2TLL(    0), DB2TLL( 3000), DB2TLL( 4875),
		DB2TLL( 6000), DB2TLL( 7125), DB2TLL( 7875), DB2TLL( 8625),
		DB2TLL( 9000), DB2TLL( 9750), DB2TLL(10125), DB2TLL(10500),
		DB2TLL(10875), DB2TLL(11250), DB2TLL(11625), DB2TLL(12000),
	},{
		DB2TLL(    0), DB2TLL( 3000), DB2TLL( 6000), DB2TLL( 7875),
		DB2TLL( 9000), DB2TLL(10125), DB2TLL(10875), DB2TLL(11625),
		DB2TLL(12000), DB2TLL(12750), DB2TLL(13125), DB2TLL(13500),
		DB2TLL(13875), DB2TLL(14250), DB2TLL(14625), DB2TLL(15000),
	},{
		DB2TLL(    0), DB2TLL( 6000), DB2TLL( 9000), DB2TLL(10875),
		DB2TLL(12000), DB2TLL(13125), DB2TLL(13875), DB2TLL(14625),
		DB2TLL(15000), DB2TLL(15750), DB2TLL(16125), DB2TLL(16500),
		DB2TLL(16875), DB2TLL(17250), DB2TLL(17625), DB2TLL(18000),
	},{
		DB2TLL(    0), DB2TLL( 9000), DB2TLL(12000), DB2TLL(13875),
		DB2TLL(15000), DB2TLL(16125), DB2TLL(16875), DB2TLL(17625),
		DB2TLL(18000), DB2TLL(18750), DB2TLL(19125), DB2TLL(19500),
		DB2TLL(19875), DB2TLL(20250), DB2TLL(20625), DB2TLL(21000),
	}
};
#undef DB2TLL

static Uint32 __fastcall rateconvAR(OPLSOUND *sndp, Uint32 rrr, Uint32 ksr)
{
	if (!rrr) return 0;
	rrr = rrr + (ksr >> 2);
	if (rrr >= 15) return AR_PHASEMAX;
	return sndp->common.ratetbla[ksr & 3] >> (CPS_SHIFTE + 1 - rrr);
}

static Uint32 __fastcall rateconv(OPLSOUND *sndp, Uint32 rrr, Uint32 ksr)
{
	if (!rrr) return 0;
	rrr = rrr + (ksr >> 2);
	if (rrr > 15) rrr = 15;
	if (rrr == 0) return EG_KEYOFF;
	return sndp->common.ratetbl[ksr & 3] >> (CPS_SHIFTE + 1 - rrr);
}

static void __fastcall OpUpdateWF(OPLSOUND *sndp, OPL_OP *opp)
{
	opp->sintable = sndp->opltbl->sin_table[opp->wf & sndp->common.wfe];
}

static void __fastcall OpUpdatePG(OPLSOUND *sndp, OPL_CH *chp, OPL_OP *opp)
{
	opp->pg.spd = (((chp->freqh << 8) + chp->freql) * opp->mul * sndp->common.cpsp) >> (CPS_SHIFTP - chp->blk);
}

static void __fastcall OpUpdateEG(OPLSOUND *sndp, OPL_CH *chp, OPL_OP *opp)
{
	Uint32 sr, rr;
	opp->ksr = chp->kcode >> ((opp->flag & FLAG_KSR) ? 0 : 2);
	opp->eg.dr_phasemax = opp->sl << (1 + 2 + EG_SHIFT);	/* 3dB->eg */
	opp->eg.spd[EG_MODE_ATTACK] = rateconvAR(sndp, opp->ar, opp->ksr);
	opp->eg.spd[EG_MODE_DECAY] = rateconv(sndp, opp->dr, opp->ksr);
	if (opp->flag & FLAG_EGT)
	{
		if (opp->eg.mode == EG_MODE_SUSTINE) opp->eg.mode = EG_MODE_SUSHOLD;
		sr = 0;
		rr = opp->rr;
	}
	else
	{
		if (opp->eg.mode == EG_MODE_SUSHOLD) opp->eg.mode = EG_MODE_SUSTINE;
		sr = opp->rr;
		rr = 7;
	}
	if (chp->sus)
	{
		rr = 5;
	}
	opp->eg.spd[EG_MODE_SUSTINE] = rateconv(sndp, sr, opp->ksr);
	opp->eg.spd[EG_MODE_RELEASE] = rateconv(sndp, rr, opp->ksr);
}


static void __fastcall OpUpdateTLL(OPLSOUND *sndp, OPL_CH *chp, OPL_OP *opp)
{
/*	if(!opp->type)
		opp->tll = (opp->tl) << 1;
	else
*/		opp->tll = (opp->tl + ((chp->ksb>>1) >> ksltable[opp->ksl])) << 1;
}

static void __fastcall oplsetopmul(OPLSOUND *sndp, OPL_CH *chp, OPL_OP *opp, Uint32 v)
{
	opp->flag &= ~(FLAG_AME | FLAG_PME | FLAG_EGT | FLAG_KSR);
#if TESTING_OPTIMIZE_AME
	if (v & 0x80) opp->amin = &sndp->lfo[LFO_UNIT_AM].output; else opp->amin = &sndp->common.amzero;
#else
	if (v & 0x80) opp->flag |= FLAG_AME;
#endif
	if (v & 0x40) opp->flag |= FLAG_PME;
	if (v & 0x20) opp->flag |= FLAG_EGT;
	if (v & 0x10) opp->flag |= FLAG_KSR;
	opp->mul = mul_table[v & 0x0f];
	OpUpdateEG(sndp, chp, opp);
	OpUpdatePG(sndp, chp, opp);
}

static void __fastcall oplsetopkstl(OPLSOUND *sndp, OPL_CH *chp, OPL_OP *opp, Uint32 v)
{
//	opp->ksl = (v >> 6) ? (/*3 - */(v >> 6)) : 15;	/* 0 / 1.5 / 3 / 6 db/OCT */
	opp->ksl = v >> 6;
	opp->tl = v & 0x3f;	/* 0.75 db */
	OpUpdateTLL(sndp, chp, opp);
}

static void __fastcall oplsetopardr(OPLSOUND *sndp, OPL_CH *chp, OPL_OP *opp, Uint32 v)
{
	opp->ar = v >> 4;
	opp->dr = v & 0xf;
	OpUpdateEG(sndp, chp, opp);
}

static void __fastcall oplsetopslrr(OPLSOUND *sndp, OPL_CH *chp, OPL_OP *opp, Uint32 v)
{
	opp->sl = v >> 4;
	opp->rr = v & 0xf;
	OpUpdateEG(sndp, chp, opp);
}

static void __fastcall oplsetopwf(OPLSOUND *sndp, OPL_CH *chp, OPL_OP *opp, Uint32 v)
{
	opp->wf = v & 0x3;
	OpUpdateWF(sndp, opp);
}

static void __fastcall oplsetopkey(OPLSOUND *sndp, OPL_OP *opp)
{
	Uint8 nextkey = ((sndp->common.rmode) && opp->rkey) || opp->key;
	if (opp->prevkey ^ nextkey)
	{
		opp->prevkey = nextkey;
		if (nextkey)
		{
			opp->eg.phase = EG_KEYOFF;
			sndp->common.enable = 1;
			opp->eg.mode = EG_MODE_ATTACK;
			opp->enable = 1;
			if(opp->eg.phase > EG_PHASEMAX)opp->eg.phasear = 0;
			else opp->eg.phasear = ((EG_PHASEMAX-opp->eg.phase)>>(EG_SHIFT+1))<<AR_SHIFT;
			//opp->eg.phasearc = 0;
			//opp->input = 0;
#if USE_FBBUF
			opp->fbbuf = 0;
#endif
			opp->pg.phase = 0;
		}
		else if (!opp->modcar && opp->eg.mode != EG_MODE_OFF){
			opp->eg.mode = EG_MODE_RELEASE;
		}
	}
}

static void __fastcall oplsetchfreql(OPLSOUND *sndp, OPL_CH *chp, Uint32 v)
{
	chp->freql = (Uint8)(v & 0xff);
	chp->ksb = ksl_table[chp->blk][(chp->freqh << 2) + (chp->freql >> 6)];
	OpUpdatePG(sndp, chp, &chp->op[0]);
	OpUpdatePG(sndp, chp, &chp->op[1]);
	OpUpdateTLL(sndp, chp, &chp->op[0]);
	OpUpdateTLL(sndp, chp, &chp->op[1]);
}

static void __fastcall oplsetchfreqh(OPLSOUND *sndp, OPL_CH *chp, Uint32 v)
{
	Uint32 key = v & 0x20;
	chp->kcode = (v >> 1) & 15;
	chp->freqh = v & 3;
	chp->blk = (v >> 2) & 7;
	chp->op[0].key = chp->op[1].key = key;
	oplsetopkey(sndp, &chp->op[0]);
	oplsetopkey(sndp, &chp->op[1]);
	chp->sus = 0;
	chp->ksb = ksl_table[chp->blk][(chp->freqh << 2) + (chp->freql >> 6)];
	OpUpdateEG(sndp, chp, &chp->op[0]);
	OpUpdateEG(sndp, chp, &chp->op[1]);
	OpUpdatePG(sndp, chp, &chp->op[0]);
	OpUpdatePG(sndp, chp, &chp->op[1]);
	OpUpdateTLL(sndp, chp, &chp->op[0]);
	OpUpdateTLL(sndp, chp, &chp->op[1]);
}

static void __fastcall oplsetchfbcon(OPLSOUND *sndp, OPL_CH *chp, Uint32 v)
{
	chp->op[0].fb = (v >> 1) & 7;
#if USE_FBBUF
//	chp->op[0].fbbuf = 0;
//	chp->op[1].fbbuf = 0;
#endif
	chp->con = (v >> 0) & 1;
	chp->op[0].modcar = (chp->con) ? 0 : 1;
	OpUpdateEG(sndp, chp, &chp->op[0]);
	chp->op[1].input = 0;
}

static void __fastcall opllsetopvolume(OPLSOUND *sndp, OPL_CH *chp, OPL_OP *opp, Uint32 v)
{
	opp->tl = v;
	OpUpdateTLL(sndp, chp, opp);
}

static void __fastcall opllsetchfreql(OPLSOUND *sndp, OPL_CH *chp, Uint32 v)
{
	chp->freql = v & 0xff;
	chp->ksb = ksl_table[chp->blk][(chp->freqh << 3) + (chp->freql >> 5)];
	OpUpdatePG(sndp, chp, &chp->op[0]);
	OpUpdatePG(sndp, chp, &chp->op[1]);
	OpUpdateTLL(sndp, chp, &chp->op[0]);
	OpUpdateTLL(sndp, chp, &chp->op[1]);
}

static void __fastcall opllsetchfreqh(OPLSOUND *sndp, OPL_CH *chp, Uint32 v)
{
	Uint32 key = v & 0x10;
	chp->kcode = v & 15;
	chp->freqh = v & 1;
	chp->blk = (v >> 1) & 7;
	chp->op[0].key = chp->op[1].key = key;
	oplsetopkey(sndp, &chp->op[0]);
	oplsetopkey(sndp, &chp->op[1]);
	chp->sus = v & 0x20;
	chp->ksb = ksl_table[chp->blk][(chp->freqh << 3) + (chp->freql >> 5)];
	OpUpdateEG(sndp, chp, &chp->op[0]);
	OpUpdateEG(sndp, chp, &chp->op[1]);
	OpUpdatePG(sndp, chp, &chp->op[0]);
	OpUpdatePG(sndp, chp, &chp->op[1]);
	OpUpdateTLL(sndp, chp, &chp->op[0]);
	OpUpdateTLL(sndp, chp, &chp->op[1]);
}

static void __fastcall SetOpTone(OPLSOUND *sndp, OPL_OP *opp, Uint8 *tonep)
{
	opp->flag &= ~(FLAG_AME | FLAG_PME | FLAG_EGT | FLAG_KSR);
#if TESTING_OPTIMIZE_AME
	if (tonep[0] & 0x80) opp->amin = &sndp->lfo[LFO_UNIT_AM].output; else opp->amin = &sndp->common.amzero;
#else
	if (tonep[0] & 0x80) opp->flag |= FLAG_AME;
#endif
	if (tonep[0] & 0x40) opp->flag |= FLAG_PME;
	if (tonep[0] & 0x20) opp->flag |= FLAG_EGT;
	if (tonep[0] & 0x10) opp->flag |= FLAG_KSR;
	opp->mul = mul_table[tonep[0] & 0x0f] << 1;
//	opp->ksl = (tonep[2] >> 6) ? (3 - (tonep[2] >> 6)) : 15;
	opp->ksl = (tonep[2] >> 6)&3;
	opp->ar = tonep[4] >> 4;
	opp->dr = tonep[4] & 0xf;
	opp->sl = tonep[6] >> 4;
	opp->rr = tonep[6] & 0xf;
}
static void __fastcall SetChTone(OPLSOUND *sndp, OPL_CH *chp, Uint8 *tonep, Uint8 *tlofsp)
{
	Uint32 op;
	for (op = 0; op < 2; op++) SetOpTone(sndp, &chp->op[op], &tonep[op]);
	chp->op[0].tl_ofs = (tlofsp[0] ^ 0x80) << (LOG_BITS - 4 + 1);
	chp->op[1].tl_ofs = (tlofsp[1] ^ 0x80) << (LOG_BITS - 4 + 1);
	chp->op[0].tl = tonep[2] & 0x3f;
	chp->op[0].fb = tonep[3] & 0x7;
	chp->op[0].wf = (tonep[3] >> 3) & 1;
	chp->op[1].wf = (tonep[3] >> 4) & 1;
#if USE_FBBUF
//	chp->op[0].fbbuf = 0;
//	chp->op[1].fbbuf = 0;
#endif
	chp->op[1].input = 0;
	OpUpdateWF(sndp, &chp->op[0]);
	OpUpdateWF(sndp, &chp->op[1]);
	OpUpdateEG(sndp, chp, &chp->op[0]);
	OpUpdateEG(sndp, chp, &chp->op[1]);
	OpUpdatePG(sndp, chp, &chp->op[0]);
	OpUpdatePG(sndp, chp, &chp->op[1]);
	OpUpdateTLL(sndp, chp, &chp->op[0]);
	OpUpdateTLL(sndp, chp, &chp->op[1]);
}

static void __fastcall opllsetchtone(OPLSOUND *sndp, OPL_CH *chp, Uint32 tone)
{
	SetChTone(sndp, chp, &sndp->regs[OPLL_INST_WORK + (tone << 3)], &sndp->regs[OPLL_INST_WORK2 + (tone << 1) + 0]);
}

static void __fastcall recovercon(OPLSOUND *sndp, OPL_CH *chp)
{
	chp->op[0].modcar = (chp->con) ? 0 : 1;
	chp->op[0].lvl = chp->con ? 1 : 0;
	chp->op[1].lvl = 1;
	OpUpdateEG(sndp, chp, &chp->op[0]);
	chp->op[1].input = 0;
}

static void __fastcall initrc_common(OPLSOUND *sndp, Uint32 rmode)
{
	if (rmode)
	{
		/* BD */
		sndp->ch[6].op[0].modcar = 1;
		sndp->ch[6].op[0].lvl = 2;
		OpUpdateEG(sndp, &sndp->ch[6], &sndp->ch[6].op[0]);
		sndp->ch[6].op[1].input = 0;
		sndp->ch[6].op[1].lvl = 1;
		OpUpdateEG(sndp, &sndp->ch[6], &sndp->ch[6].op[1]);
		/* HH */
		sndp->ch[7].op[0].input = 0;
		sndp->ch[7].op[0].lvl = 2;
		OpUpdateEG(sndp, &sndp->ch[7], &sndp->ch[7].op[0]);
		/* SD */
		sndp->ch[7].op[1].input = 0;
		sndp->ch[7].op[1].lvl = 4;
		OpUpdateEG(sndp, &sndp->ch[7], &sndp->ch[7].op[1]);
		/* TOM */
		sndp->ch[8].op[0].modcar = 0;
		sndp->ch[8].op[0].lvl = 4;
		OpUpdateEG(sndp, &sndp->ch[8], &sndp->ch[8].op[0]);
		/* CYM */
		sndp->ch[8].op[1].modcar = 0;
		sndp->ch[8].op[1].lvl = 2;
		OpUpdateEG(sndp, &sndp->ch[8], &sndp->ch[8].op[1]);
	}
	else
	{
		recovercon(sndp, &sndp->ch[6]);
		if (!sndp->ch[6].op[0].key) SetOpOff(&sndp->ch[6].op[0]);
		if (!sndp->ch[6].op[1].key) SetOpOff(&sndp->ch[6].op[1]);
		recovercon(sndp, &sndp->ch[7]);
		if (!sndp->ch[7].op[0].key) SetOpOff(&sndp->ch[7].op[0]);
		if (!sndp->ch[7].op[1].key) SetOpOff(&sndp->ch[7].op[1]);
		recovercon(sndp, &sndp->ch[8]);
		if (!sndp->ch[8].op[0].key) SetOpOff(&sndp->ch[8].op[0]);
		if (!sndp->ch[8].op[1].key) SetOpOff(&sndp->ch[8].op[1]);
	}
}

static void __fastcall oplsetrc(OPLSOUND *sndp, Uint32 rc)
{
	sndp->lfo[LFO_UNIT_AM].table = (rc & 0x80) ? sndp->opltbl->am_table1 : sndp->opltbl->am_table2;
	sndp->lfo[LFO_UNIT_PM].table = (rc & 0x40) ? sndp->opltbl->pm_table1 : sndp->opltbl->pm_table2;
	if ((sndp->common.rmode ^ rc) & 0x20)
	{
		if (rc & 0x20)
		{
#if 0
			static Uint8 volini[2] = { 0, 0 };
			static Uint8 bdtone[8] = { 0x04, 0x20, 0x28, 0x00, 0xDF, 0xF8, 0xFF, 0xF8 };
			SetChTone(sndp, &sndp->ch[6], bdtone, volini);
			SetChTone(sndp, &sndp->ch[7], &romtone[0][0x11 << 4], volini);
			SetChTone(sndp, &sndp->ch[8], &romtone[0][0x12 << 4], volini);
#endif
//			sndp->ch[7].op[0].nst = PG_SHIFT + 4;
//			sndp->ch[7].op[1].nst = PG_SHIFT + 6;
//			sndp->ch[8].op[1].nst = PG_SHIFT + 5;

			sndp->ch[7].op[0].nst = DP_BITS;
			sndp->ch[7].op[1].nst = DP_BITS;
			sndp->ch[8].op[1].nst = DP_BITS;
		}
		initrc_common(sndp, rc & 0x20);
	}
	sndp->common.rmode = rc & 0x20;
	sndp->common.rc = rc & 0x1f;

	
	/* BD */
	sndp->ch[6].op[0].rkey = sndp->ch[6].op[1].rkey = rc & 0x10;
	oplsetopkey(sndp, &sndp->ch[6].op[0]);
	oplsetopkey(sndp, &sndp->ch[6].op[1]);
	/* HH */
	sndp->ch[7].op[0].rkey = rc & 0x01;
	oplsetopkey(sndp, &sndp->ch[7].op[0]);
	/* SD */
	sndp->ch[7].op[1].rkey = rc & 0x08;
	oplsetopkey(sndp, &sndp->ch[7].op[1]);
	/* TOM */
	sndp->ch[8].op[0].rkey = rc & 0x04;
	oplsetopkey(sndp, &sndp->ch[8].op[0]);
	/* CYM */
	sndp->ch[8].op[1].rkey = rc & 0x02;
	oplsetopkey(sndp, &sndp->ch[8].op[1]);
}

static void __fastcall opllsetrc(OPLSOUND *sndp, Uint32 rc)
{

	if ((sndp->common.rmode ^ rc) & 0x20)
	{
		if (rc & 0x20)
		{
			opllsetchtone(sndp, &sndp->ch[6], 0x10);
			opllsetchtone(sndp, &sndp->ch[7], 0x11);
			opllsetchtone(sndp, &sndp->ch[8], 0x12);
//			opllsetopvolume(sndp, &sndp->ch[6], &sndp->ch[6].op[1], (sndp->regs[0x36] & 0x0f) << 2);
			opllsetopvolume(sndp, &sndp->ch[7], &sndp->ch[7].op[0], (sndp->regs[0x37] & 0xf0) >> 2);
			opllsetopvolume(sndp, &sndp->ch[7], &sndp->ch[7].op[1], (sndp->regs[0x37] & 0x0f) << 2);
			opllsetopvolume(sndp, &sndp->ch[8], &sndp->ch[8].op[0], (sndp->regs[0x38] & 0xf0) >> 2);
			opllsetopvolume(sndp, &sndp->ch[8], &sndp->ch[8].op[1], (sndp->regs[0x38] & 0x0f) << 2);
			sndp->ch[7].op[0].nst = DP_BITS;
			sndp->ch[7].op[1].nst = DP_BITS;
			sndp->ch[8].op[1].nst = DP_BITS;
		}
		else
		{
			opllsetchtone(sndp, &sndp->ch[6], sndp->regs[0x36]>>4);
			opllsetchtone(sndp, &sndp->ch[7], sndp->regs[0x37]>>4);
			opllsetchtone(sndp, &sndp->ch[8], sndp->regs[0x38]>>4);
		}
		initrc_common(sndp, rc & 0x20);
	}
	sndp->common.rmode = rc & 0x20;
	sndp->common.rc = rc & 0x1f;
	/* BD */
	sndp->ch[6].op[0].rkey = sndp->ch[6].op[1].rkey = rc & 0x10;
	oplsetopkey(sndp, &sndp->ch[6].op[0]);
	oplsetopkey(sndp, &sndp->ch[6].op[1]);
	/* HH */
	sndp->ch[7].op[0].rkey = rc & 0x01;
	oplsetopkey(sndp, &sndp->ch[7].op[0]);
	/* SD */
	sndp->ch[7].op[1].rkey = rc & 0x08;
	oplsetopkey(sndp, &sndp->ch[7].op[1]);
	/* TOM */
	sndp->ch[8].op[0].rkey = rc & 0x04;
	oplsetopkey(sndp, &sndp->ch[8].op[0]);
	/* CYM */
	sndp->ch[8].op[1].rkey = rc & 0x02;
	oplsetopkey(sndp, &sndp->ch[8].op[1]);
}

#define OPLSETOP(func) { \
	Uint32 op = op_table[a & 0x1f]; \
	if (op != 0xff) func(sndp, &sndp->ch[op >> 1], &sndp->ch[op >> 1].op[op & 1], v); \
}

__inline static void oplwritereg(OPLSOUND *sndp, Uint32 a, Uint32 v)
{
	switch (a >> 5)
	{
	default:
		NEVER_REACH
	case 0:
		switch (a & 0x1f)
		{
		case 0x01:
			if (sndp->opl_type == OPL_TYPE_OPL2)
			{
				Uint32 i;
				sndp->common.wfe = (v & 0x20) ? 3 : 0;
				for (i = 0; i < 9; i++)
				{
					OpUpdateWF(sndp, &sndp->ch[i].op[0]);
					OpUpdateWF(sndp, &sndp->ch[i].op[1]);
				}
			}
			break;
		case 0x08:
			/* CSM mode */
		case 0x07:	case 0x09:	case 0x0a:	case 0x0b:	case 0x0c:
		case 0x0d:	case 0x0e:	case 0x0f:	case 0x10:	case 0x11:	case 0x12:
			if (sndp->deltatpcm) sndp->deltatpcm->write(sndp->deltatpcm->ctx, a - 0x07, v);
			break;
		}
		break;
	case 1: OPLSETOP(oplsetopmul); break;
	case 2: OPLSETOP(oplsetopkstl); break;
	case 3: OPLSETOP(oplsetopardr); break;
	case 4: OPLSETOP(oplsetopslrr); break;
	case 7: OPLSETOP(oplsetopwf); break;
	case 5:
		if ((a & 0x3f) == (0x3d))
			oplsetrc(sndp, v);
		else if ((a & 0x1f) < 9)
			oplsetchfreql(sndp, &sndp->ch[a & 0xf], v);
		else if ((a & 0xf) < 9)
			oplsetchfreqh(sndp, &sndp->ch[a & 0xf], v);
		break;
	case 6:
		if ((a & 0x1f) < 9) oplsetchfbcon(sndp, &sndp->ch[a & 0xf], v);
		break;
	}
}

static void oplwrite(OPLSOUND *sndp, Uint32 a, Uint32 v)
{
	if (a & 1)
	{
		sndp->regs[sndp->common.adr] = v;
		sndp->oplregs[sndp->common.adr] = v;
		oplwritereg(sndp, sndp->common.adr, v);
	}
	else
		sndp->common.adr = v;
}

static Uint32 oplread(OPLSOUND *sndp, Uint32 a)
{
	if (a & 1)
		return sndp->regs[sndp->common.adr];
	else
		return 0x80;
}

__inline static void opllwritereg(OPLSOUND *sndp, Uint32 a, Uint32 v)
{
	char b;
	switch (a >> 3)
	{
		default:
			NEVER_REACH
		case 0:
			sndp->regs[OPLL_INST_WORK + (a & 7)] = v;
			for(b=(sndp->regs[0xe]&0x20)?5:8;b>=0;b--){
				if(!(sndp->regs[0x30+b]&0xf0))
					opllsetchtone(sndp, &sndp->ch[b], 0);
			}
			break;
		case 1:
			if (a == 0xe) opllsetrc(sndp, v & 0x3f);
			break;
		case 2:
		case 3:
			a &= 0xf;
			a %= 9;
			if (a < 9) opllsetchfreql(sndp, &sndp->ch[a], v);
			break;
		case 4:
		case 5:
			a &= 0xf;
			a %= 9;
			if (a < 9) opllsetchfreqh(sndp, &sndp->ch[a], v);
			break;
		case 6:
		case 7:
			a &= 0xf;
			a %= 9;
			if (a < 9)
			{
				if ((sndp->common.rmode) && (a >= 6))
				{
					if (a != 6) opllsetopvolume(sndp, &sndp->ch[a], &sndp->ch[a].op[0], (v & 0xf0) >> 2);
				}
				else
				{
					opllsetchtone(sndp, &sndp->ch[a], (v & 0xf0) >> 4);
				}
				opllsetopvolume(sndp, &sndp->ch[a], &sndp->ch[a].op[1], (v & 0xf) << 2);
			}
			break;
	}
}

static void opllwrite(OPLSOUND *sndp, Uint32 a, Uint32 v)
{
	if (a & 1)
	{
		if (sndp->common.adr < 0x40)
		{
			sndp->regs[sndp->common.adr] = v;
			sndp->opllregs[sndp->common.adr] = v;
			opllwritereg(sndp, sndp->common.adr, v);
		}
	}
	else
		sndp->common.adr = v;
}

static Uint32 opllread(OPLSOUND *sndp, Uint32 a)
{
	return 0xFF;
}

static void __fastcall opreset(OPLSOUND *sndp, OPL_OP *opp)
{
	/* XMEMSET(opp, 0, sizeof(OPL_OP)); */
	SetOpOff(opp);
	opp->tl_ofs = 0x80 << (LOG_BITS - 4 + 1);
#if TESTING_OPTIMIZE_AME
	opp->amin = &sndp->common.amzero;
#endif
}

static void __fastcall chreset(OPLSOUND *sndp, OPL_CH *chp, Uint32 clock, Uint32 freq)
{
	Uint32 op;
	XMEMSET(chp, 0, sizeof(OPL_CH));
	for (op = 0; op < 2; op++)
	{
		opreset(sndp, &chp->op[op]);
	}
	recovercon(sndp, chp);
}

static void sndreset(OPLSOUND *sndp, Uint32 clock, Uint32 freq)
{
	Uint32 i, cpse;
#if LOWPASS_FILTER
	Uint32 loop;
	double answer,plus,fb;
#endif
	sndp->freq = freq;
	sndp->freqp = 0;
//	freq = FM_FREQ;
	XMEMSET(&sndp->common, 0, sizeof(sndp->common));
	XMEMSET(&sndp->lfo[LFO_UNIT_AM], 0, sizeof(OPL_LFO));
	sndp->lfo[LFO_UNIT_AM].sps = DivFix(37 * (1 << AMTBL_BITS), freq * 10, LFO_SHIFT);
	sndp->lfo[LFO_UNIT_AM].adrmask = (1 << AMTBL_BITS) - 1;
	sndp->lfo[LFO_UNIT_AM].table = sndp->opltbl->am_table1;
	XMEMSET(&sndp->lfo[LFO_UNIT_PM], 0, sizeof(OPL_LFO));
	sndp->lfo[LFO_UNIT_PM].sps = DivFix(64 * (1 << PMTBL_BITS), freq * 10, LFO_SHIFT);
	sndp->lfo[LFO_UNIT_PM].adrmask = (1 << PMTBL_BITS) - 1;
	sndp->lfo[LFO_UNIT_PM].table = sndp->opltbl->pm_table1;
	sndp->common.cpsp = DivFix(clock, 72 * freq/4, CPS_SHIFTP);
	cpse = DivFix(clock, 72 * freq, CPS_SHIFTE);
	for (i = 0; i < 4; i++)
	{
		sndp->common.ratetbl[i] = (4+i) * cpse;
		sndp->common.ratetbla[i] = 3 * sndp->common.ratetbl[i];
	}
	sndp->common.tll2logtbl = sndp->opltbl->tll2log_table;
	sndp->common.sintablemask = (1 << SINTBL_BITS) - 1;
	for (i = 0; i < 9; i++) chreset(sndp, &sndp->ch[i], clock, freq);
	if (sndp->deltatpcm) sndp->deltatpcm->reset(sndp->deltatpcm->ctx, clock, freq);
	if (sndp->opl_type & OPL_TYPE_OPL)
	{
		XMEMSET(&sndp->regs, 0, 0x100);
		/*
		sndp->common.ar_table = sndp->opltbl->ar_tablepow;
		sndp->common.sintablemask -= (1 << (SINTBL_BITS - 11)) - 1;
		*/
		sndp->common.ar_table = sndp->opltbl->ar_tablelog;
		//sndp->common.wfe = 1;
		//sndp->common.sintablemask -= (1 << (SINTBL_BITS - 8)) - 1;
		
		for (i = 0x0; i < 0x100; i++)
		{
			oplwrite(sndp, 0, i);
			oplwrite(sndp, 1, 0x00);
		}
		for (i = 0xa0; i < 0xa9; i++)
		{
			oplwrite(sndp, 0, 0xa0 + i);
			oplwrite(sndp, 1, 0x40);
			oplwrite(sndp, 0, 0xb0 + i);
			oplwrite(sndp, 1, 0x0e);
		}
	}
	else
	{
		const static Uint8 fmbios_initdata[8] = "\x30\x10\x20\x20\xfb\xb2\xf3\xf3";
		XMEMSET(&sndp->regs, 0, 0x40);
		sndp->common.ar_table = sndp->opltbl->ar_tablelog;
		sndp->common.wfe = 1;
		sndp->common.sintablemask -= (1 << (SINTBL_BITS - 8)) - 1;
		for (i = 0; i < sizeof(fmbios_initdata); i++)
		{
			opllwrite(sndp, 0, i);
			opllwrite(sndp, 1, fmbios_initdata[i]);
		}
		opllwrite(sndp, 0, 0x0e);
		opllwrite(sndp, 1, 0x00);
		opllwrite(sndp, 0, 0x0f);
		opllwrite(sndp, 1, 0x00);
		for (i = 0; i < 9; i++)
		{
			opllwrite(sndp, 0, 0x10 + i);
			opllwrite(sndp, 1, 0x20);
			opllwrite(sndp, 0, 0x20 + i);
			opllwrite(sndp, 1, 0x07);
			opllwrite(sndp, 0, 0x30 + i);
			opllwrite(sndp, 1, 0xb3);
		}
	}
	for (i = 0; i < 9; i++)
	{
		sndp->ch[i].op[0].type = 0;
		sndp->ch[i].op[1].type = 1;
	}
	sndp->rng = 0xffff;


}

static void oplsetinst(OPLSOUND *sndp, Uint32 n, void *p, Uint32 l)
{
	if (sndp->deltatpcm) sndp->deltatpcm->setinst(sndp->deltatpcm->ctx, n, p, l);
}

__inline static Uint32 GetDwordLE(Uint8 *p)
{
	return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}
#define GetDwordLEM(p) (Uint32)((((Uint8 *)p)[0] | (((Uint8 *)p)[1] << 8) | (((Uint8 *)p)[2] << 16) | (((Uint8 *)p)[3] << 24)))

static void opllsetinst(OPLSOUND *sndp, Uint32 n, Uint8 *p, Uint32 l)
{
	Int32 i, j, sb = 9;
	if (n) return;
	if ((GetDwordLE(p) & 0xf0ffffff) == GetDwordLEM("ILL0"))
	{
		if (0 < p[4] && p[4] <= SINTBL_BITS) sb = p[4];
		for(j = 1; j < 16 + 3; j++)
			for(i = 0; i < 8; i++)
				sndp->regs[OPLL_INST_WORK + (j << 3) + i] = p[(j << 4) + i];
		for(j = 0; j < 16 + 3; j++)
		{
			sndp->regs[OPLL_INST_WORK2 + (j << 1) + 0] = p[(j << 4) + 8];
			sndp->regs[OPLL_INST_WORK2 + (j << 1) + 1] = p[(j << 4) + 9];
		}
	}
	else
	{
		for(j = 1; j < 16; j++)
			for(i = 0; i < 8; i++)
				sndp->regs[OPLL_INST_WORK + (j << 3) + i] = p[((j - 1) << 3) + i];
	}
	sndp->common.sintablemask =  (1 <<  SINTBL_BITS)       - 1;
	sndp->common.sintablemask -= (1 << (SINTBL_BITS - sb)) - 1;
}

static void sndrelease(OPLSOUND *sndp)
{
	if (sndp) {
		if (sndp->logtbl) sndp->logtbl->release(sndp->logtbl->ctx);
		if (sndp->opltbl) sndp->opltbl->release(sndp->opltbl->ctx);
		if (sndp->deltatpcm) sndp->deltatpcm->release(sndp->deltatpcm->ctx);
		XFREE(sndp);
	}
}

//ここからレジスタビュアー設定
static Uint8 *regdata;
static Uint8 *regdata2;
Uint32 (*ioview_ioread_DEV_OPL)(Uint32 a);
Uint32 (*ioview_ioread_DEV_OPLL)(Uint32 a);
static Uint32 ioview_ioread_bf(Uint32 a){
	if(a<=0xff)return regdata[a];else return 0x100;
}
static Uint32 ioview_ioread_bf2(Uint32 a){
	if(a<0x40)return regdata2[a];else return 0x100;
}
//ここまでレジスタビュアー設定

KMIF_SOUND_DEVICE *OPLSoundAlloc(Uint32 opl_type)
{
	OPLSOUND *sndp;
	sndp = XMALLOC(sizeof(OPLSOUND));
	if (!sndp) return 0;
	XMEMSET(sndp, 0, sizeof(OPLSOUND));
	sndp->opl_type = (Uint8)opl_type;
	sndp->kmif.ctx = sndp;
	sndp->kmif.release = sndrelease;
	sndp->kmif.volume = sndvolume;
	sndp->kmif.reset = sndreset;
	sndp->kmif.synth = sndsynth;
	if (sndp->opl_type == OPL_TYPE_MSXAUDIO)
	{
		sndp->deltatpcm = YMDELTATPCMSoundAlloc(YMDELTATPCM_TYPE_Y8950 , 0);
	}
	else
		sndp->deltatpcm = 0;
	if (sndp->opl_type & OPL_TYPE_OPL)
	{
		sndp->kmif.write = oplwrite;
		sndp->kmif.read = oplread;
		sndp->kmif.setinst = oplsetinst;
	}
	else
	{
		sndp->kmif.write = opllwrite;
		sndp->kmif.read = opllread;
		sndp->kmif.setinst = opllsetinst;
		switch (sndp->opl_type)
		{
			case OPL_TYPE_OPLL:
			case OPL_TYPE_MSXMUSIC:
			case OPL_TYPE_SMSFMUNIT:
				opllsetinst(sndp, 0, romtone[0], 16 * 19);//YM2413
				break;
			case OPL_TYPE_VRC7:
				opllsetinst(sndp, 0, romtone[2], 16 * 19);//VRC7
				break;
		}												  //romtone[1]を使う必要全然なし
	}
	sndp->logtbl = LogTableAddRef();
	sndp->opltbl = OplTableAddRef();
	if (!sndp->logtbl || !sndp->opltbl)
	{
		sndrelease(sndp);
		return 0;
	}
	//ここからレジスタビュアー設定
	if (sndp->opl_type & OPL_TYPE_OPL){
		regdata = sndp->oplregs;
		ioview_ioread_DEV_OPL = ioview_ioread_bf;
	}else{
		regdata2 = sndp->opllregs;
		ioview_ioread_DEV_OPLL = ioview_ioread_bf2;
	}
	//ここまでレジスタビュアー設定
	return &sndp->kmif;
}
