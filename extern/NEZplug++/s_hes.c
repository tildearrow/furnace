#include "kmsnddev.h"
#include "divfix.h"
#include "s_hes.h"
#include "s_logtbl.h"

#define CPS_SHIFT 16
#define RENDERS 5
#define NOISE_RENDERS 3
#define LOG_KEYOFF (32 << LOG_BITS)
#define LFO_BASE (0x10 << 8)


typedef struct {
	Uint32 cps;				/* cycles per sample */
	Uint32 pt;				/* programmable timer */
	Uint32 wl;				/* wave length */
	Uint32 npt;				/* noise programmable timer */
	Uint32 nvol;			/* noise volume */
	Uint32 rng;				/* random number generator (seed) */
	Uint32 dda;				/* direct D/A output */
	Uint32 tone[0x20];		/* tone waveform */
	Uint8 tonereg[0x20];	/* tone waveform regs*/
	Uint32 lfooutput;		/* lfo output */
	Uint32 nwl;				/* noise wave length */
	Uint8 regs[8 - 2];		/* registers per channel */
	Uint8 st;				/* wave step */
	Uint8 tonep;			/* tone waveform write pointer*/
	Uint8 mute;
	Uint8 edge;
	Uint8 edgeout;
	Uint8 rngold;
	Int32 output[2];
	Uint8 count;
} HES_WAVEMEMORY;

typedef struct {
	Uint32 cps;			/* cycles per sample */
	Uint32 pt;			/* lfo programmable timer */
	Uint32 wl;			/* lfo wave length */
	Uint8 tone[0x20];	/* lfo waveform */
	Uint8 tonep;		/* lfo waveform write pointer */
	Uint8 st;			/* lfo step */
	Uint8 update;
	Uint8 regs[2];
} HES_LFO;

typedef struct {
	KMIF_SOUND_DEVICE kmif;
	KMIF_LOGTABLE *logtbl;
	HES_WAVEMEMORY ch[6];
	HES_LFO lfo;
	HES_WAVEMEMORY *cur;
	struct {
		Int32 mastervolume;
		Uint8 sysregs[2];
	} common;
} HESSOUND;

#define V(a) ((((Uint32)(0x1F - (a)) * (Uint32)(1 << LOG_BITS) * (Uint32)1000) / (Uint32)3800) << 1)
const static Uint32 voltbl[0x20] = {
	V(0x00), V(0x01), V(0x02),V(0x03),V(0x04), V(0x05), V(0x06),V(0x07),
	V(0x08), V(0x09), V(0x0A),V(0x0B),V(0x0C), V(0x0D), V(0x0E),V(0x0F),
	V(0x10), V(0x11), V(0x12),V(0x13),V(0x14), V(0x15), V(0x16),V(0x17),
	V(0x18), V(0x19), V(0x1A),V(0x1B),V(0x1C), V(0x1D), V(0x1E),V(0x1F),
};
const static Uint32 voltbl2[0x20] = {
	V(0x10)+1, V(0x0F)+1, V(0x0E)+1,V(0x0D)+1,V(0x0C)+1, V(0x0B)+1, V(0x0A)+1,V(0x09)+1,
	V(0x08)+1, V(0x07)+1, V(0x06)+1,V(0x05)+1,V(0x04)+1, V(0x03)+1, V(0x02)+1,V(0x01)+1,
	V(0x00), V(0x01), V(0x02),V(0x03),V(0x04), V(0x05), V(0x06),V(0x07),
	V(0x08), V(0x09), V(0x0A),V(0x0B),V(0x0C), V(0x0D), V(0x0E),V(0x0F),
};
const static Uint8 reset_table[] = 
{
	0x54,0x68,0x69,0x73,0x20,0x70,0x72,0x6f,0x67,0x72,0x61,0x6d,0x20,0x75,0x73,0x65,
	0x20,0x4e,0x45,0x5a,0x50,0x6c,0x75,0x67,0x2b,0x2b,0x20,0x69,0x6e,0x20,0x50,0x43,
	0x2d,0x45,0x6e,0x67,0x69,0x6e,0x65,0x2e,0x20,0x20,0x20,0x20,0x20,0x28,0x6d,0x65,
	0x73,0x73,0x61,0x67,0x65,0x20,0x62,0x79,0x20,0x4f,0x66,0x66,0x47,0x61,0x6f,0x29
};

#undef V

#if HES_TONE_DEBUG_OPTION_ENABLE
Uint8 HES_tone_debug_option = 0;
#endif
Uint8 HES_noise_debug_option1 = 9;
Uint8 HES_noise_debug_option2 = 10;
Int32 HES_noise_debug_option3 = 3;
Int32 HES_noise_debug_option4 = 508;

static void HESSoundWaveMemoryRender(HESSOUND *sndp, HES_WAVEMEMORY *ch, Int32 *p, Uint8 chn)
{
	Uint32 wl, output, lvol, rvol;
	Int32 outputbf[2]={0,0},count=0;
	if (ch->mute || !(ch->regs[4 - 2] & 0x80)) return;
	lvol = voltbl[(sndp->common.sysregs[1] >> 3) & 0x1E];
	lvol +=	voltbl[(ch->regs[5 - 2] >> 3) & 0x1E];
	lvol += voltbl[ch->regs[4 - 2] & 0x1F];
	rvol = voltbl[(sndp->common.sysregs[1] << 1) & 0x1E];
	rvol += voltbl[(ch->regs[5 - 2] << 1) & 0x1E];
	rvol += voltbl[ch->regs[4 - 2] & 0x1F];

	if (ch->regs[4 - 2] & 0x40)	/* DDA */
	{
		output = ch->dda;
		if(chmask[DEV_HUC6230_CH1+chn]){
			p[0] += LogToLin(sndp->logtbl, lvol + output + sndp->common.mastervolume, LOG_LIN_BITS - LIN_BITS - 17 - 1);
			p[1] += LogToLin(sndp->logtbl, rvol + output + sndp->common.mastervolume, LOG_LIN_BITS - LIN_BITS - 17 - 1);
		}
	}
	else if (ch->regs[7 - 2] & 0x80)	/* NOISE */
	{
		/* if (wl == 0) return; */
		ch->npt += ((ch->cps>>16) * ch->nwl)<<NOISE_RENDERS;
		//----------
		output = LinToLog(sndp->logtbl,(ch->edge * 16)) + (1 << (LOG_BITS + 1)) + 1;
		ch->output[0] = LogToLin(sndp->logtbl, lvol + output + sndp->common.mastervolume, LOG_LIN_BITS - LIN_BITS - 18 - 1);
		ch->output[1] = LogToLin(sndp->logtbl, rvol + output + sndp->common.mastervolume, LOG_LIN_BITS - LIN_BITS - 18 - 1);
		//----------
		while (ch->npt > (0x1000 << 6))
		{
			outputbf[0] += ch->output[0];
			outputbf[1] += ch->output[1];
			count++;

			ch->npt = ch->npt - (0x1000 << 6);
			ch->count++;
			if(ch->count >= 1<<NOISE_RENDERS){
				ch->count = 0;

				ch->rng <<= 1;
				ch->rng |= ((ch->rng>>17)&1) ^ ((ch->rng>>14)&1);
				ch->edge = ((ch->rng>>17)&1);
				//----------
				output = LinToLog(sndp->logtbl,(ch->edge * 16)) + (1 << (LOG_BITS + 1)) + 1;
				ch->output[0] = LogToLin(sndp->logtbl, lvol + output + sndp->common.mastervolume, LOG_LIN_BITS - LIN_BITS - 18 - 1);
				ch->output[1] = LogToLin(sndp->logtbl, rvol + output + sndp->common.mastervolume, LOG_LIN_BITS - LIN_BITS - 18 - 1);
				//----------
			}
		}
		outputbf[0] += ch->output[0];
		outputbf[1] += ch->output[1];
		count++;

		if(chmask[DEV_HUC6230_CH1+chn]){
			p[0] += outputbf[0] / count;
			p[1] += outputbf[1] / count;
		}
		
	}
	else
	{
//		wl = ch->wl + ch->lfooutput;
		/* if (wl <= (LFO_BASE + 16)) wl = (LFO_BASE + 16); */
//		if (wl <= (LFO_BASE + 4)) return;
//		wl = (wl - LFO_BASE) << CPS_SHIFT;
		wl = ch->wl + ch->lfooutput;
		wl &= 0xfff;
		/* if (wl <= (LFO_BASE + 16)) wl = (LFO_BASE + 16); */
		if (wl <= 4) return;
		wl = wl << CPS_SHIFT;
		ch->pt += ch->cps << RENDERS;

		//----------
		output = ch->tone[ch->st & 0x1f];
		ch->output[0] = LogToLin(sndp->logtbl, lvol + output + sndp->common.mastervolume, LOG_LIN_BITS - LIN_BITS - 17 - 1);
		ch->output[1] = LogToLin(sndp->logtbl, rvol + output + sndp->common.mastervolume, LOG_LIN_BITS - LIN_BITS - 17 - 1);
		//----------
		while (ch->pt >= wl)
		{
			outputbf[0] += ch->output[0];
			outputbf[1] += ch->output[1];
			count++;

			ch->count++;
			ch->pt -= wl;
			if(ch->count >= 1<<RENDERS){
				ch->count = 0;

				ch->st++;
				//----------
				output = ch->tone[ch->st & 0x1f];
				ch->output[0] = LogToLin(sndp->logtbl, lvol + output + sndp->common.mastervolume, LOG_LIN_BITS - LIN_BITS - 17 - 1);
				ch->output[1] = LogToLin(sndp->logtbl, rvol + output + sndp->common.mastervolume, LOG_LIN_BITS - LIN_BITS - 17 - 1);
				//----------
			}
		}
		outputbf[0] += ch->output[0];
		outputbf[1] += ch->output[1];
		count++;
		if(chmask[DEV_HUC6230_CH1+chn]){
			p[0] += outputbf[0] / count;
			p[1] += outputbf[1] / count;
		}
	}
}

static void HESSoundLfoStep(HESSOUND *sndp)
{
	if (sndp->lfo.update & 1)
	{
		sndp->lfo.update &= ~1;
		sndp->lfo.wl = sndp->ch[1].regs[2 - 2] + ((sndp->ch[1].regs[3 - 2] & 0xf) << 8);
		sndp->lfo.wl *= sndp->lfo.regs[0];
		sndp->lfo.wl <<= CPS_SHIFT;
	}
	if (sndp->lfo.wl <= (16 << CPS_SHIFT))
	{
//		sndp->ch[0].lfooutput = LFO_BASE;
		sndp->ch[0].lfooutput = 0;
		return;
	}
	sndp->lfo.pt += sndp->lfo.cps;
	while (sndp->lfo.pt >= sndp->lfo.wl)
	{
		sndp->lfo.pt -= sndp->lfo.wl;
		sndp->lfo.st++;
	}
	sndp->ch[0].lfooutput = sndp->lfo.tone[sndp->lfo.st & 0x1f];
	sndp->ch[0].lfooutput -= 0x10;
	switch (sndp->lfo.regs[1] & 3)
	{
		case 0:
//			sndp->ch[0].lfooutput = LFO_BASE;
			sndp->ch[0].lfooutput = 0;
			break;
		case 1:
//			sndp->ch[0].lfooutput += LFO_BASE - (0x10 << 0);
			break;
		case 2:
			sndp->ch[0].lfooutput <<= 4;
//			sndp->ch[0].lfooutput += LFO_BASE - (0x10 << 4);
//			sndp->ch[0].lfooutput += LFO_BASE - (0x10 << 4);
			break;
		case 3:
			sndp->ch[0].lfooutput <<= 8;
			/*sndp->ch[0].lfooutput += LFO_BASE - (0x10 << 8);*/
			break;
	}
}

static void sndsynth(void *ctx, Int32 *p)
{
	HESSOUND *sndp = ctx;
	HESSoundWaveMemoryRender(sndp, &sndp->ch[5], p, 5);
	HESSoundWaveMemoryRender(sndp, &sndp->ch[4], p, 4);
	HESSoundWaveMemoryRender(sndp, &sndp->ch[3], p, 3);
	HESSoundWaveMemoryRender(sndp, &sndp->ch[2], p, 2);
	if (sndp->lfo.regs[1] & 0x80)
	{
		HESSoundWaveMemoryRender(sndp, &sndp->ch[1], p, 1);
//		sndp->ch[0].lfooutput = LFO_BASE;
		sndp->ch[0].lfooutput = 0;
	}
	else
	{
		HESSoundWaveMemoryRender(sndp, &sndp->ch[1], p, 1);
		HESSoundLfoStep(sndp);
	}
	HESSoundWaveMemoryRender(sndp, &sndp->ch[0], p, 0);
}

static void HESSoundChReset(HESSOUND *sndp, HES_WAVEMEMORY *ch, Uint32 clock, Uint32 freq)
{
	int i;
	XMEMSET(ch, 0, sizeof(HES_WAVEMEMORY));
	ch->cps = DivFix(clock, 6 * freq, CPS_SHIFT);
	ch->nvol = LinToLog(sndp->logtbl, 10);
	ch->dda = LOG_KEYOFF;
	ch->rng = 1;
	ch->edge = 0;
	ch->lfooutput = LFO_BASE;
	for (i = 0; i < 0x20; i++) ch->tone[i] = LOG_KEYOFF;
}

static void HESSoundLfoReset(HES_LFO *ch, Uint32 clock, Uint32 freq)
{
	XMEMSET(ch, 0, sizeof(HES_LFO));
	ch->cps = DivFix(clock, 6 * freq, CPS_SHIFT);
	/* ch->regs[1] = 0x80; */
}


static void sndreset(void *ctx, Uint32 clock, Uint32 freq)
{
	HESSOUND *sndp = ctx;
	Uint32 ch;
	XMEMSET(&sndp->common, 0, sizeof(sndp->common));
	sndp->cur = sndp->ch;
	sndp->common.sysregs[1] = 0xFF;
	for (ch = 0; ch < 6; ch++) HESSoundChReset(sndp, &sndp->ch[ch], clock, freq);
	HESSoundLfoReset(&sndp->lfo, clock, freq);
}

static void sndwrite(void *ctx, Uint32 a, Uint32 v)
{
	HESSOUND *sndp = ctx;
	switch (a & 0xF)
	{
		case 0:	// register select
			sndp->common.sysregs[0] = v & 7;
			if (sndp->common.sysregs[0] <= 5)
				sndp->cur = &sndp->ch[sndp->common.sysregs[0]];
			else
				sndp->cur = 0;
			break;
		case 1:	// main volume
			sndp->common.sysregs[1] = v;
			break;
		case 2:	// frequency low
		case 3:	// frequency high
			if (sndp->cur)
			{
				sndp->cur->regs[a - 2] = v;
				sndp->cur->wl = sndp->cur->regs[2 - 2] + ((sndp->cur->regs[3 - 2] & 0xf) << 8);
				if (sndp->cur == &sndp->ch[1]) sndp->lfo.update |= 1;
			}
			break;
		case 4:	// ON, DDA, AL
/*			if (sndp->cur && !(v & 0x80)) {
					sndp->cur->st = 0;
			}
*/		case 5:	// LAL, RAL
			if (sndp->cur) sndp->cur->regs[a - 2] = v;
			break;
		case 6:	// wave data
			if (sndp->cur)
			{
				Uint32 tone;
				Int32 data = v & 0x1f;
				sndp->cur->regs[6 - 2] = v;
#if HES_TONE_DEBUG_OPTION_ENABLE
				switch (HES_tone_debug_option)
				{
					default:
					case 0:
						tone = LinToLog(sndp->logtbl, data - 0x10) + (1 << (LOG_BITS + 1));
						//tone =data - 0x10;
						break;
					case 1:
						tone = voltbl2[data];
						break;
					case 2:
						tone = LinToLog(sndp->logtbl, data) + (1 << (LOG_BITS + 1));
						break;
				}
#else
				//tone = LinToLog(sndp->logtbl, data - 0x10) + (1 << (LOG_BITS + 1));
				tone = LinToLog(sndp->logtbl, -data) + (1 << (LOG_BITS + 1));
#endif
				if (sndp->cur->regs[4 - 2] & 0x40)
				{
					sndp->cur->dda = tone;
#if 1
					if (sndp->cur == &sndp->ch[1]) sndp->lfo.tonep = 0x0;
					sndp->cur->tonep = 0;
#endif
					//sndp->cur->st = 0;
				}
				if ((sndp->cur->regs[4 - 2] & 0x80) == 0)
				{
					sndp->cur->dda = LOG_KEYOFF;
					sndp->cur->st = 0;
					if (sndp->cur == &sndp->ch[1])
					{
						//sndp->lfo.tone[sndp->lfo.tonep] = data ^ 0x10;
						sndp->lfo.tone[sndp->lfo.tonep] = data;
						if (++sndp->lfo.tonep == 0x20) sndp->lfo.tonep = 0;
					}
					sndp->cur->tone[sndp->cur->tonep] = tone;
					sndp->cur->tonereg[sndp->cur->tonep] = data;
					if (++sndp->cur->tonep == 0x20) sndp->cur->tonep = 0;
				}
			}
			break;
		case 7:	// noise on, noise frq
			if (sndp->cur && sndp->common.sysregs[0] >= 4)
			{
				Uint32 nwl;
				sndp->cur->regs[7 - 2] = v;
				switch (HES_noise_debug_option1)
				{
				case 1:
					/* v0.9.3beta7 old linear frequency */
					/* HES_noise_debug_option1=1 */
					/* HES_noise_debug_option2=HES_noise_debug_option(default:5) */
					/* HES_noise_debug_option3=512 */
					/* HES_noise_debug_option5=0 */
					nwl = HES_noise_debug_option4 << (CPS_SHIFT - 10);
					nwl += (((v & 0x1f) << 9) + HES_noise_debug_option3) << (CPS_SHIFT - 9 + HES_noise_debug_option2);
					break;
				case 2:
					/* linear frequency */
					nwl = HES_noise_debug_option4 << (CPS_SHIFT - 10);
					nwl += (v & 0x1f) << (CPS_SHIFT - 10 + HES_noise_debug_option2 - HES_noise_debug_option3);
					break;
				default:
				case 3:
					/* positive logarithmic frequency */
					nwl = HES_noise_debug_option4 << (CPS_SHIFT - 10);
					nwl -= LogToLin(sndp->logtbl, ((0 & 0x1f) ^ 0x1f) << (LOG_BITS - HES_noise_debug_option3 + 1), LOG_LIN_BITS - CPS_SHIFT - HES_noise_debug_option2);
					nwl += LogToLin(sndp->logtbl, ((v & 0x1f) ^ 0x1f) << (LOG_BITS - HES_noise_debug_option3 + 1), LOG_LIN_BITS - CPS_SHIFT - HES_noise_debug_option2);
					break;
				case 4:
					/* negative logarithmic frequency */
					nwl = HES_noise_debug_option4 << (CPS_SHIFT - 10);
					nwl += LogToLin(sndp->logtbl, ((0 & 0x1f) ^ 0x00) << (LOG_BITS - HES_noise_debug_option3 + 1), LOG_LIN_BITS - CPS_SHIFT - HES_noise_debug_option2);
					nwl -= LogToLin(sndp->logtbl, ((v & 0x1f) ^ 0x00) << (LOG_BITS - HES_noise_debug_option3 + 1), LOG_LIN_BITS - CPS_SHIFT - HES_noise_debug_option2);
					break;
				case 5:
					/* positive logarithmic frequency (reverse) */
					nwl = HES_noise_debug_option4 << (CPS_SHIFT - 10);
					nwl -= LogToLin(sndp->logtbl, ((0 & 0x1f) ^ 0x1f) << (LOG_BITS - HES_noise_debug_option3 + 1), LOG_LIN_BITS - CPS_SHIFT - HES_noise_debug_option2);
					nwl += LogToLin(sndp->logtbl, ((v & 0x1f) ^ 0x00) << (LOG_BITS - HES_noise_debug_option3 + 1), LOG_LIN_BITS - CPS_SHIFT - HES_noise_debug_option2);
					break;
				case 6:
					/* negative logarithmic frequency (reverse) */
					nwl = HES_noise_debug_option4 << (CPS_SHIFT - 10);
					nwl += LogToLin(sndp->logtbl, ((0 & 0x1f) ^ 0x00) << (LOG_BITS - HES_noise_debug_option3 + 1), LOG_LIN_BITS - CPS_SHIFT - HES_noise_debug_option2);
					nwl -= LogToLin(sndp->logtbl, (((v & 0x1f) ^ 0x1f)+1) << (LOG_BITS - HES_noise_debug_option3 + 1), LOG_LIN_BITS - CPS_SHIFT - HES_noise_debug_option2);
					break;
				case 7:
					/* v0.9.3beta8 old logarithmic frequency type B */
					nwl = HES_noise_debug_option4 << (CPS_SHIFT - 10);
					nwl += LogToLin(sndp->logtbl, ((v & 0x1f) ^ 0x1f) << (LOG_BITS - HES_noise_debug_option3 + 1), LOG_LIN_BITS - CPS_SHIFT - HES_noise_debug_option2);
					break;
				case 8:
					/* v0.9.3beta8 old logarithmic frequency type C */
					/* HES_noise_debug_option1=3 */
					/* HES_noise_debug_option2=13 */
					/* HES_noise_debug_option3=2 */
					/* HES_noise_debug_option4=0 */
					nwl = HES_noise_debug_option4;
					nwl += LogToLin(sndp->logtbl, (v & 0x1f) << (LOG_BITS - HES_noise_debug_option3 + 1), LOG_LIN_BITS - CPS_SHIFT - HES_noise_debug_option2);
					break;
				case 9:
					/* v0.9.4.8 +2 +5 */
//					nwl = LogToLin(sndp->logtbl, ((((v & 0x1f)<<4) - (0x0c<<4))<<5) + ((((v & 0x1f)<<4) - (0x0c<<4))<<3) + ((((v & 0x1f)<<4) - (0x0c<<4))<<1), LOG_LIN_BITS - CPS_SHIFT - HES_noise_debug_option2);
					if(0x1f - (v & 0x1f))
						nwl = 0x1000 / ((0x1f - (v & 0x1f))) ;
					else
						nwl = 0x4000 ;
					break;
				}
				sndp->cur->npt = 0;
				sndp->cur->nwl = nwl;
			}
			break;
		case 8:	// LFO frequency
			sndp->lfo.regs[a - 8] = v;
			sndp->lfo.update |= 1;
			break;
		case 9:	// LFO on, LFO control
			sndp->lfo.regs[a - 8] = v;
			if(v&0x80){
				//LFO Reset
				sndp->lfo.st = sndp->lfo.pt = 0;
			}
			break;
	}
}

static Uint32 sndread(void *ctx, Uint32 a)
{
	HESSOUND *sndp = ctx;
	a &= 0xF;
	switch (a & 0xF)
	{
		case 0:	case 1:
			return sndp->common.sysregs[a];
		case 2:	case 3:	case 4:	case 5:	case 6:	case 7:
			if (sndp->cur) return sndp->cur->regs[a - 2];
			return 0;
		case 8:	case 9:
			return sndp->lfo.regs[a - 8];
	}
	return 0;
}

static void sndvolume(void *ctx, Int32 volume)
{
	HESSOUND *sndp = ctx;
	volume = (volume << (LOG_BITS - 8)) << 1;
	sndp->common.mastervolume = volume;
}

static void sndrelease(void *ctx)
{
	HESSOUND *sndp = ctx;
	if (sndp) {
		if (sndp->logtbl) sndp->logtbl->release(sndp->logtbl->ctx);
		XFREE(sndp);
	}
}

static void setinst(void *ctx, Uint32 n, void *p, Uint32 l){}

//ここからレジスタビュアー設定
static HESSOUND *sndpr;
Uint32 (*ioview_ioread_DEV_HUC6230)(Uint32 a);
Uint32 pce_ioview_ioread_bf(Uint32 a){
	if(a<=0x1)return sndpr->common.sysregs[a];
	if(a>=0x2 && a<=0x7)if (sndpr->cur) return sndpr->cur->regs[a - 2];
	if(a>=0x8 && a<=0x9)return sndpr->lfo.regs[a - 8];
	if(a>=0x20 && a<0x7f){
		if((a&0xf)>=0x2 && (a&0xf)<=0x7)
			return sndpr->ch[(a>>4)-2].regs[(a&0xf)-2];
	}
	if(a>=0x100 && a<=0x1bf){
		return sndpr->ch[(a>>5)-8].tonereg[(a&0x1f)];
	}
	return 0x100;
}
//ここまでレジスタビュアー設定

KMIF_SOUND_DEVICE *HESSoundAlloc(void)
{
	HESSOUND *sndp;
	sndp = XMALLOC(sizeof(HESSOUND));
	if (!sndp) return 0;
	XMEMSET(sndp, 0, sizeof(HESSOUND));
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
	ioview_ioread_DEV_HUC6230 = pce_ioview_ioread_bf;
	//ここまでレジスタビュアー設定

	return &sndp->kmif;
}
