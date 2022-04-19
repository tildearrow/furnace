#include "kmsnddev.h"
#include "divfix.h"
#include "s_logtbl.h"
#include "s_psg.h"

#define DCFIX 0/*8*/
#define ANAEX 0
#define CPS_SHIFT 18
#define CPS_ENVSHIFT 12
#define LOG_KEYOFF (31 << (LOG_BITS + 1))

#define RENDERS 7
#define NOISE_RENDERS 3

#define VOLUME_3 1
#define PSG_VOL 1

typedef struct {
#if DCFIX
	Uint32 dcbuf[1 << DCFIX];
	Uint32 dcave;
	Uint32 dcptr;
#endif
	Uint32 cps;
	Uint32 cycles;
#if ANAEX
	Int32 anaex[2];
#endif
	Uint8 regs[3];
	Uint8 adr;
	Uint8 mute;
	Uint8 key;
	Uint32 count;
	Int32 output;
} PSG_SQUARE;

typedef struct {
	Uint32 cps;
	Uint32 cycles;
	Uint32 noiserng;
	Uint8 regs[1];
	Uint8 count;
	Uint32 output;
} PSG_NOISE;

typedef struct {
	Int32 cps;
	Int32 cycles;
} MSX_DA;

typedef struct {
	Uint32 cps;
	Uint32 cycles;
	const Int8 *adr;
	Uint8 regs[3];
	Uint8 adrmask;
} PSG_ENVELOPE;

typedef struct {
	KMIF_SOUND_DEVICE kmif;
	KMIF_LOGTABLE *logtbl;
	PSG_SQUARE square[3];
	PSG_ENVELOPE envelope;
	PSG_NOISE noise;
	MSX_DA da;
	struct {
		Int32 mastervolume;
		Uint32 davolume;
		Uint32 envout;
		Uint8 daenable;
		Uint8 regs[1];
		Uint32 rngout;
		Uint8 adr;
	} common;
	Uint8 type;
	Uint8 regs[0x10];
} PSGSOUND;

const static Int8 env_pulse[] = 
{
	0x1F,+02,0x1E,+02,0x1D,+02,0x1C,+02,0x1B,+02,0x1A,+02,0x19,+02,0x18,+02,
	0x17,+02,0x16,+02,0x15,+02,0x14,+02,0x13,+02,0x12,+02,0x11,+02,0x10,+02,
	0x0F,+02,0x0E,+02,0x0D,+02,0x0C,+02,0x0B,+02,0x0A,+02,0x09,+02,0x08,+02,
	0x07,+02,0x06,+02,0x05,+02,0x04,+02,0x03,+02,0x02,+02,0x01,+02,0x00,+00,
};
const static Int8 env_pulse_hold[] = 
{
	0x1F,+02,0x1E,+02,0x1D,+02,0x1C,+02,0x1B,+02,0x1A,+02,0x19,+02,0x18,+02,
	0x17,+02,0x16,+02,0x15,+02,0x14,+02,0x13,+02,0x12,+02,0x11,+02,0x10,+02,
	0x0F,+02,0x0E,+02,0x0D,+02,0x0C,+02,0x0B,+02,0x0A,+02,0x09,+02,0x08,+02,
	0x07,+02,0x06,+02,0x05,+02,0x04,+02,0x03,+02,0x02,+02,0x01,+02,0x00,+02,
	0x1F,+00,
};
const static Int8 env_saw[] = 
{
	0x1F,+02,0x1E,+02,0x1D,+02,0x1C,+02,0x1B,+02,0x1A,+02,0x19,+02,0x18,+02,
	0x17,+02,0x16,+02,0x15,+02,0x14,+02,0x13,+02,0x12,+02,0x11,+02,0x10,+02,
	0x0F,+02,0x0E,+02,0x0D,+02,0x0C,+02,0x0B,+02,0x0A,+02,0x09,+02,0x08,+02,
	0x07,+02,0x06,+02,0x05,+02,0x04,+02,0x03,+02,0x02,+02,0x01,+02,0x00,-62,
};
const static Int8 env_tri[] = 
{
	0x1F,+02,0x1E,+02,0x1D,+02,0x1C,+02,0x1B,+02,0x1A,+02,0x19,+02,0x18,+02,
	0x17,+02,0x16,+02,0x15,+02,0x14,+02,0x13,+02,0x12,+02,0x11,+02,0x10,+02,
	0x0F,+02,0x0E,+02,0x0D,+02,0x0C,+02,0x0B,+02,0x0A,+02,0x09,+02,0x08,+02,
	0x07,+02,0x06,+02,0x05,+02,0x04,+02,0x03,+02,0x02,+02,0x01,+02,0x00,+02,
	0x00,+02,0x01,+02,0x02,+02,0x03,+02,0x04,+02,0x05,+02,0x06,+02,0x07,+02,
	0x08,+02,0x09,+02,0x0A,+02,0x0B,+02,0x0C,+02,0x0D,+02,0x0E,+02,0x0F,+02,
	0x10,+02,0x11,+02,0x12,+02,0x13,+02,0x14,+02,0x15,+02,0x16,+02,0x17,+02,
	0x18,+02,0x19,+02,0x1A,+02,0x1B,+02,0x1C,+02,0x1D,+02,0x1E,+02,0x1F,-126,
};
const static Int8 env_xpulse[] = 
{
	0x00,+02,0x01,+02,0x02,+02,0x03,+02,0x04,+02,0x05,+02,0x06,+02,0x07,+02,
	0x08,+02,0x09,+02,0x0A,+02,0x0B,+02,0x0C,+02,0x0D,+02,0x0E,+02,0x0F,+02,
	0x10,+02,0x11,+02,0x12,+02,0x13,+02,0x14,+02,0x15,+02,0x16,+02,0x17,+02,
	0x18,+02,0x19,+02,0x1A,+02,0x1B,+02,0x1C,+02,0x1D,+02,0x1E,+02,0x1F,+00,
};
const static Int8 env_xpulse_hold[] = 
{
	0x00,+02,0x01,+02,0x02,+02,0x03,+02,0x04,+02,0x05,+02,0x06,+02,0x07,+02,
	0x08,+02,0x09,+02,0x0A,+02,0x0B,+02,0x0C,+02,0x0D,+02,0x0E,+02,0x0F,+02,
	0x10,+02,0x11,+02,0x12,+02,0x13,+02,0x14,+02,0x15,+02,0x16,+02,0x17,+02,
	0x18,+02,0x19,+02,0x1A,+02,0x1B,+02,0x1C,+02,0x1D,+02,0x1E,+02,0x1F,+02,
	0x00,+00,
};
const static Int8 env_xsaw[] = 
{
	0x00,+02,0x01,+02,0x02,+02,0x03,+02,0x04,+02,0x05,+02,0x06,+02,0x07,+02,
	0x08,+02,0x09,+02,0x0A,+02,0x0B,+02,0x0C,+02,0x0D,+02,0x0E,+02,0x0F,+02,
	0x10,+02,0x11,+02,0x12,+02,0x13,+02,0x14,+02,0x15,+02,0x16,+02,0x17,+02,
	0x18,+02,0x19,+02,0x1A,+02,0x1B,+02,0x1C,+02,0x1D,+02,0x1E,+02,0x1F,-62,
};

const static Int8 env_xtri[] = 
{
	0x00,+02,0x01,+02,0x02,+02,0x03,+02,0x04,+02,0x05,+02,0x06,+02,0x07,+02,
	0x08,+02,0x09,+02,0x0A,+02,0x0B,+02,0x0C,+02,0x0D,+02,0x0E,+02,0x0F,+02,
	0x10,+02,0x11,+02,0x12,+02,0x13,+02,0x14,+02,0x15,+02,0x16,+02,0x17,+02,
	0x18,+02,0x19,+02,0x1A,+02,0x1B,+02,0x1C,+02,0x1D,+02,0x1E,+02,0x1F,+02,
	0x1F,+02,0x1E,+02,0x1D,+02,0x1C,+02,0x1B,+02,0x1A,+02,0x19,+02,0x18,+02,
	0x17,+02,0x16,+02,0x15,+02,0x14,+02,0x13,+02,0x12,+02,0x11,+02,0x10,+02,
	0x0F,+02,0x0E,+02,0x0D,+02,0x0C,+02,0x0B,+02,0x0A,+02,0x09,+02,0x08,+02,
	0x07,+02,0x06,+02,0x05,+02,0x04,+02,0x03,+02,0x02,+02,0x01,+02,0x00,-126,
};

#if 1
const static Int8 *(env_table[16]) =
{
	env_pulse,       env_pulse,       env_pulse,       env_pulse,
	env_xpulse_hold, env_xpulse_hold, env_xpulse_hold, env_xpulse_hold,
	env_saw,         env_pulse,       env_tri,         env_pulse_hold,
	env_xsaw,        env_xpulse,      env_xtri,        env_xpulse_hold,
};
#else
const static Int8 *(env_table[16]) =
{
	env_pulse,	env_pulse,	env_pulse,	env_pulse,
	env_xpulse,	env_xpulse,	env_xpulse,	env_xpulse,
	env_saw,	env_pulse,	env_tri,	env_pulse_hold,
	env_xsaw,	env_xpulse,	env_xtri,	env_xpulse_hold,
};
#endif


const static Uint32 voltbl[2][32] = {
//0 : PSG_TYPE_AY_3_8910 : PSG
#define V(a) ((((a * 5 * (1 << (LOG_BITS - 1))) / 13)+((0 * (1 << (LOG_BITS - 1))) / 3)) << 1)
	LOG_KEYOFF, V(0x1e), V(0x1d),V(0x1c),V(0x1b), V(0x1a), V(0x19), V(0x18),
	   V(0x17), V(0x16), V(0x15),V(0x14),V(0x13), V(0x12), V(0x11), V(0x10),
	   V(0x0f), V(0x0e), V(0x0d),V(0x0c),V(0x0b), V(0x0a), V(0x09), V(0x08),
	   V(0x07), V(0x06), V(0x05),V(0x04),V(0x03), V(0x02), V(0x01), V(0x00),
//1 : PSG_TYPE_YM2149 : SSG (YM2149などの、YAMAHAのPSG互換系)
#undef V
#define V(a) (((a * (1 << (LOG_BITS - 1))) / 2) << 1)
	LOG_KEYOFF, V(0x1e), V(0x1d),V(0x1c),V(0x1b), V(0x1a), V(0x19), V(0x18),
	   V(0x17), V(0x16), V(0x15),V(0x14),V(0x13), V(0x12), V(0x11), V(0x10),
	   V(0x0f), V(0x0e), V(0x0d),V(0x0c),V(0x0b), V(0x0a), V(0x09), V(0x08),
	   V(0x07), V(0x06), V(0x05),V(0x04),V(0x03), V(0x02), V(0x01), V(0x00),

};
#undef V

__inline static Uint32 PSGSoundNoiseStep(PSGSOUND *sndp)
{
	Uint32 spd;
	Int32 outputbuf=0,count=0;

	spd = sndp->noise.regs[0] & 0x1F;
	spd = spd ? (spd << CPS_SHIFT) : (1 << (CPS_SHIFT-2));

	sndp->noise.cycles += sndp->noise.cps << NOISE_RENDERS;
	sndp->noise.output = (sndp->noise.noiserng >> 16) & 1;
	while (sndp->noise.cycles >= spd)
	{
		//outputbuf += sndp->noise.output;
		//count++;

		sndp->noise.count++;
		sndp->noise.cycles -= spd;

		if(sndp->noise.count >= 1<<NOISE_RENDERS){
			sndp->noise.count = 0;
			sndp->noise.noiserng += sndp->noise.noiserng + (1 & ((sndp->noise.noiserng >> 13) ^ (sndp->noise.noiserng >> 16)));
			sndp->noise.output = (sndp->noise.noiserng >> 16) & 1;
		}
	}
	outputbuf += sndp->noise.output;
	count++;
	return (outputbuf << 8) / count;
}

__inline static Int32 PSGSoundEnvelopeStep(PSGSOUND *sndp)
{
	Uint32 spd;
	spd = (sndp->envelope.regs[1] << 8) + sndp->envelope.regs[0];

	if (!spd) spd = 1; // 0の時は1と同じ動作になる 

	spd <<= CPS_ENVSHIFT;
	sndp->envelope.cycles += sndp->envelope.cps;
	while (sndp->envelope.cycles >= spd)
	{
		sndp->envelope.cycles -= spd;
		sndp->envelope.adr += sndp->envelope.adr[1];
	}

	if (sndp->envelope.adr[0] & sndp->envelope.adrmask)
	{
#ifdef VOLUME_3
		return LogToLin(sndp->logtbl, voltbl[sndp->type][sndp->envelope.adr[0] & sndp->envelope.adrmask] + sndp->common.mastervolume, LOG_LIN_BITS - 21);
#else
		return LogToLin(sndp->logtbl, ((sndp->envelope.adrmask - (sndp->envelope.adr[0] & sndp->envelope.adrmask)) << (LOG_BITS - 2 + 1)) + sndp->common.mastervolume, LOG_LIN_BITS - 21);
#endif
	}
	else
		return 0;
}

__inline static Uint32 PSGSoundSquareSub(PSGSOUND *sndp, PSG_SQUARE *chp)
{
	Int32 volume, bit = 1;
	Uint32 spd;
	Int32 outputbuf=0,count=0;

	if (chp->regs[2] & 0x10)
		volume = sndp->common.envout;
	else if (chp->regs[2] & 0xF)
	{
#ifdef VOLUME_3
		volume = LogToLin(sndp->logtbl, voltbl[sndp->type][(chp->regs[2] & 0xF)<<1] + sndp->common.mastervolume, LOG_LIN_BITS - 21);
#else
		volume = LogToLin(sndp->logtbl, ((0xF - (chp->regs[2] & 0xF)) << (LOG_BITS - 1 + 1)) + sndp->common.mastervolume, LOG_LIN_BITS - 21);
#endif
	}
	else
		volume = 0;

	spd = ((chp->regs[1] & 0x0F) << 8) + chp->regs[0];

	/* if (!spd) return 0; */

	chp->output = (chp->adr & 1) ? volume : 0;
	if (spd /*> 7*/)
	{
		spd <<= CPS_SHIFT;
		chp->cycles += chp->cps<<RENDERS;
		while (chp->cycles >= spd)
		{
			outputbuf += chp->output;
			count++;

			chp->count++;
			chp->cycles -= spd;

			if(chp->count >= 1<<RENDERS){
				chp->count = 0;
				chp->adr++;
				chp->output = (chp->adr & 1) ? volume : 0;
			}
		}
	}else{
		chp->output = volume;
	}
	outputbuf += chp->output;
	count++;

	if (chp->mute){
		return 0;
	}

	switch(chp->key & 3){
	case 0:
		return volume;
	case 1:
		return outputbuf / count;
	case 2:
		return volume * sndp->common.rngout / 256;
	case 3:
		return (outputbuf / count) * sndp->common.rngout / 256;
	}
}

__inline static void MSXSoundDaStep(PSGSOUND *sndp)
{
	sndp->da.cycles += sndp->da.cps;
	while (sndp->da.cycles > 0)
	{
		sndp->da.cycles -= 1<<22;
		sndp->common.davolume = sndp->common.davolume *15 /16;
		if(sndp->common.daenable)sndp->common.davolume += ((1<<16) - sndp->common.davolume)/16;
	}
}



#if (((-1) >> 1) == -1)
#define SSR(x, y) (((Int32)x) >> (y))
#else
#define SSR(x, y) (((x) >= 0) ? ((x) >> (y)) : (-((-(x) - 1) >> (y)) - 1))
#endif

static Int32 PSGSoundSquare(PSGSOUND *sndp, PSG_SQUARE *chp)
{
	Int32 out;
	out = PSGSoundSquareSub(sndp, chp);
#if DCFIX
	chp->dcptr = (chp->dcptr + 1) & ((1 << DCFIX) - 1);
	chp->dcave -= chp->dcbuf[chp->dcptr];
	chp->dcave += out;
	chp->dcbuf[chp->dcptr] = out;
	out = out - (chp->dcave >> DCFIX);
#endif
#if ANAEX
	out = ((chp->anaex[0] << ANAEX) + ((out - chp->anaex[0]) + ((out - chp->anaex[1]) >> 3))) >> ANAEX;
	chp->anaex[1] = chp->anaex[0];
	chp->anaex[0] = out;
	out += out;
#endif
	return out + out;
}

static void sndsynth(void *ctx, Int32 *p)
{
	PSGSOUND *sndp = ctx;
	Int32 accum = 0;
	sndp->common.rngout = PSGSoundNoiseStep(sndp);
	sndp->common.envout = PSGSoundEnvelopeStep(sndp);
	accum += PSGSoundSquare(sndp, &sndp->square[0]) * chmask[DEV_AY8910_CH1];
	accum += PSGSoundSquare(sndp, &sndp->square[1]) * chmask[DEV_AY8910_CH2];
	accum += PSGSoundSquare(sndp, &sndp->square[2]) * chmask[DEV_AY8910_CH3];
	MSXSoundDaStep(sndp);
	if (chmask[DEV_MSX_DA])
		accum += LogToLin(sndp->logtbl,sndp->common.mastervolume, LOG_LIN_BITS-7)
		* (sndp->common.daenable ? (sndp->common.davolume*7 + (1<<16))/7 : sndp->common.davolume);
#ifdef VOLUME_3
	accum = accum * PSG_VOL;
#endif
	p[0] += accum;
	p[1] += accum;
}

static void sndvolume(void *ctx, Int32 volume)
{
	PSGSOUND *sndp = ctx;
	volume = (volume << (LOG_BITS - 8)) << 1;
	sndp->common.mastervolume = volume;
}

__inline static Uint32 sndreadreg(PSGSOUND *sndp, Uint32 a)
{
	switch (a)
	{
		case 0x0: case 0x1:
		case 0x2: case 0x3:
		case 0x4: case 0x5:
			return sndp->square[a >> 1].regs[a & 1];
		case 0x6:
			return sndp->noise.regs[0];
		case 0x7:
			return sndp->common.regs[0];
		case 0x8: case 0x9: case 0xA:
			return sndp->square[a - 0x8].regs[2];
		case 0xB: case 0xC: case 0xD:
			return sndp->envelope.regs[a - 0xB];
	}
	return 0;
}

__inline static void sndwritereg(PSGSOUND *sndp, Uint32 a, Uint32 v)
{
	sndp->regs[a&0xf]=v;
	switch (a)
	{
		case 0x0: case 0x1:
		case 0x2: case 0x3:
		case 0x4: case 0x5:
			sndp->square[a >> 1].regs[a & 1] = v;
			break;
		case 0x6:
			sndp->noise.regs[0] = v;
			break;
		case 0x7:
			sndp->common.regs[0] = v;
			{
				Uint32 ch;
				for (ch = 0; ch < 3; ch++)
				{
					sndp->square[ch].key = 0;
					if (!(v & (1 << ch))) sndp->square[ch].key |= 1;
					if (!(v & (8 << ch))) sndp->square[ch].key |= 2;
				}
			}
			break;
		case 0x8: case 0x9: case 0xA:
			sndp->square[a - 0x8].regs[2] = v;
			break;
		case 0xD:
			sndp->envelope.cycles = 0;
			sndp->envelope.adr = env_table[sndp->regs[0xd] & 0xF];
		case 0xB: case 0xC:
			sndp->envelope.regs[a - 0xB] = v;
			break;
	}
}

static Uint32 sndread(void *ctx, Uint32 a)
{
	PSGSOUND *sndp = ctx;
	return sndreadreg(sndp, sndp->common.adr);
}

static void sndwrite(void *ctx, Uint32 a, Uint32 v)
{
	PSGSOUND *sndp = ctx;
	switch (a & 0x3)
	{
		case 0:
			sndp->common.adr = v;
			break;
		case 1:
			sndwritereg(sndp, sndp->common.adr, v);
			break;
		case 2:
			sndp->common.daenable = v&1;
			break;
	}
}

static void PSGSoundSquareReset(PSG_SQUARE *ch, Uint32 clock, Uint32 freq)
{
	XMEMSET(ch, 0, sizeof(PSG_SQUARE));
	ch->cps = DivFix(clock, 16 * freq, CPS_SHIFT);
}

static void PSGSoundNoiseReset(PSG_NOISE *ch, Uint32 clock, Uint32 freq)
{
	XMEMSET(ch, 0, sizeof(PSG_NOISE));
	ch->cps = DivFix(clock, 2 * 16 * freq, CPS_SHIFT);
	ch->noiserng = 0x8fec;
}

static void PSGSoundEnvelopeReset(PSG_ENVELOPE *ch, Uint32 clock, Uint32 freq)
{
	XMEMSET(ch, 0, sizeof(PSG_ENVELOPE));
	ch->cps = DivFix(clock * 2, 2 * 16 * freq, CPS_ENVSHIFT);
	ch->adr = env_table[0];
}

static void MSXSoundDaReset(MSX_DA *ch, Uint32 clock, Uint32 freq)
{
	XMEMSET(ch, 0, sizeof(MSX_DA));	ch->cps = DivFix(clock * 2, 16 * freq, CPS_SHIFT);

}

static void sndreset(void *ctx, Uint32 clock, Uint32 freq)
{
	PSGSOUND *sndp = ctx;
	const static Uint8 initdata[] = { 0,0,0,0,0,0, 0, 0x38, 0x0,0x0,0x0, 0,0,0 };
	Uint32 i;
	XMEMSET(&sndp->common, 0, sizeof(sndp->common));
	PSGSoundNoiseReset(&sndp->noise, clock, freq);
	PSGSoundEnvelopeReset(&sndp->envelope, clock, freq);
	PSGSoundSquareReset(&sndp->square[0], clock, freq);
	PSGSoundSquareReset(&sndp->square[1], clock, freq);
	PSGSoundSquareReset(&sndp->square[2], clock, freq);
	MSXSoundDaReset(&sndp->da, clock, freq);
	if (sndp->type == PSG_TYPE_AY_3_8910)
	{
		sndp->envelope.adrmask = 0x1e;
	}
	else
	{
		sndp->envelope.adrmask = 0x1f;
	}
	for (i = 0; i < sizeof(initdata); i++)
	{
		sndwrite(sndp, 0, i);			/* address */
		sndwrite(sndp, 1, initdata[i]);	/* data */
	}
}

static void sndrelease(void *ctx)
{
	PSGSOUND *sndp = ctx;
	if (sndp) {
		if (sndp->logtbl) sndp->logtbl->release(sndp->logtbl->ctx);
		XFREE(sndp);
	}
}

static void setinst(void *ctx, Uint32 n, void *p, Uint32 l){}

//ここからレジスタビュアー設定
static PSGSOUND *sndpr;
Uint32 (*ioview_ioread_DEV_AY8910)(Uint32 a);
Uint32 (*ioview_ioread_DEV_MSX)(Uint32 a);
static Uint32 ioview_ioread_bf(Uint32 a){
	if(a<=0xd)return sndpr->regs[a];else return 0x100;
}
static Uint32 ioview_ioread_bf2(Uint32 a){
	if(a==0x0)return sndpr->common.daenable;else return 0x100;
}
//ここまでレジスタビュアー設定

KMIF_SOUND_DEVICE *PSGSoundAlloc(Uint32 psg_type)
{
	PSGSOUND *sndp;
	sndp = XMALLOC(sizeof(PSGSOUND));
	if (!sndp) return 0;
	XMEMSET(sndp, 0, sizeof(PSGSOUND));
	sndp->type = psg_type;
	sndp->kmif.ctx = sndp;
	sndp->kmif.release = sndrelease;
	sndp->kmif.reset = sndreset;
	sndp->kmif.synth = sndsynth;
	sndp->kmif.volume = sndvolume;
	sndp->kmif.write = sndwrite;
	sndp->kmif.read = sndread;
	sndp->kmif.setinst = setinst;
	sndp->logtbl = LogTableAddRef();
	if (!sndp->logtbl)
	{
		sndrelease(sndp);
		return 0;
	}
	//ここからレジスタビュアー設定
	sndpr = sndp;
	ioview_ioread_DEV_AY8910 = ioview_ioread_bf;
	ioview_ioread_DEV_MSX = ioview_ioread_bf2;
	//ここまでレジスタビュアー設定
	return &sndp->kmif;
}
