#include "kmsnddev.h"
#include "divfix.h"
#include "s_logtbl.h"
#include "s_scc.h"


#define CPS_SHIFT 17
#define RENDERS 6

#define LOG_KEYOFF (31 << (LOG_BITS + 1))

typedef struct {
	Uint32 cycles;
	Uint32 spd;
	Uint32 tone[32];
	Uint8 tonereg[32];
	Uint32 volume;
	Uint8 regs[3];
	Uint8 adr;
	Uint8 mute;
	Uint8 key;
	Uint8 pad4[2];
	Uint8 count;
	Uint32 output;
} SCC_CH;

typedef struct {
	KMIF_SOUND_DEVICE kmif;
	KMIF_LOGTABLE *logtbl;
	SCC_CH ch[5];
	Uint32 majutushida;
	struct {
		Uint32 cps;
		Int32 mastervolume;
		Uint8 mode;
		Uint8 enable;
	} common;
	Uint8 regs[0x10];
} SCCSOUND;

__inline static Int32 SCCSoundChSynth(SCCSOUND *sndp, SCC_CH *ch)
{
	Int32 outputbuf=0,count=0;
	if (ch->spd <= (9 << CPS_SHIFT)) return 0;

	ch->cycles += sndp->common.cps<<RENDERS;
	ch->output = LogToLin(sndp->logtbl, ch->volume + sndp->common.mastervolume + ch->tone[ch->adr & 0x1F], LOG_LIN_BITS - LIN_BITS - LIN_BITS - 9);
	while (ch->cycles >= ch->spd)
	{
		outputbuf += ch->output;
		count++;

		ch->cycles -= ch->spd;
		ch->count++;
		if(ch->count >= 1<<RENDERS){
			ch->count = 0;
			ch->adr++;
			ch->output = LogToLin(sndp->logtbl, ch->volume + sndp->common.mastervolume + ch->tone[ch->adr & 0x1F], LOG_LIN_BITS - LIN_BITS - LIN_BITS - 9);
		}
	}
	outputbuf += ch->output;
	count++;

	if (ch->mute || !ch->key) return 0;
	return outputbuf / count;
}

__inline static void SCCSoundChReset(SCC_CH *ch, Uint32 clock, Uint32 freq)
{
	XMEMSET(ch, 0, sizeof(SCC_CH));
}

static void sndsynth(void *ctx, Int32 *p)
{
	SCCSOUND *sndp = ctx;
	if (sndp->common.enable)
	{
		Uint32 ch;
		Int32 accum = 0;
		for (ch = 0; ch < 5; ch++) accum += SCCSoundChSynth(sndp, &sndp->ch[ch]) * chmask[DEV_SCC_CH1 + ch];
		accum += LogToLin(sndp->logtbl, sndp->common.mastervolume + sndp->majutushida, LOG_LIN_BITS - LIN_BITS - 14);
		p[0] += accum;
		p[1] += accum;

	}
}

static void sndvolume(void *ctx, Int32 volume)
{
	SCCSOUND *sndp = ctx;
	volume = (volume << (LOG_BITS - 8)) << 1;
	sndp->common.mastervolume = volume;
}

static Uint32 sndread(void *ctx, Uint32 a)
{
	return 0;
}

static void sndwrite(void *ctx, Uint32 a, Uint32 v)
{
	SCCSOUND *sndp = ctx;
	if (a == 0xbffe || a == 0xbfff)
	{
		sndp->common.mode = v;
	}
	else if ((0x9800 <= a && a <= 0x985F) || (0xB800 <= a && a <= 0xB89F))
	{
		Uint32 tone = LinToLog(sndp->logtbl, ((Int32)(v ^ 0x80)) - 0x80);
		sndp->ch[(a & 0xE0) >> 5].tone[a & 0x1F] = tone;
		sndp->ch[(a & 0xE0) >> 5].tonereg[a & 0x1F] = v;
	}
	else if (0x9860 <= a && a <= 0x987F)
	{
		Uint32 tone = LinToLog(sndp->logtbl, ((Int32)(v ^ 0x80)) - 0x80);
		sndp->ch[3].tone[a & 0x1F] = sndp->ch[4].tone[a & 0x1F] = tone;
		sndp->ch[3].tonereg[a & 0x1F] = sndp->ch[4].tonereg[a & 0x1F] = v;
	}
	else if ((0x9880 <= a && a <= 0x988F) || (0xB8A0 <= a && a <= 0xB8AF))
	{
		Uint32 port = a & 0x0F;
		sndp->regs[port]=v;
		if (0x0 <= port && port <= 0x9)
		{
			SCC_CH *ch = &sndp->ch[port >> 1];
			ch->regs[port & 0x1] = v;
			ch->spd = (((ch->regs[1] & 0x0F) << 8) + ch->regs[0] + 1) << CPS_SHIFT;
		}
		else if (0xA <= port && port <= 0xE)
		{
			SCC_CH *ch = &sndp->ch[port - 0xA];
			ch->regs[2] = v;
			ch->volume = LinToLog(sndp->logtbl, ch->regs[2] & 0xF);
		}
		else
		{
			Uint32 i;
			if (v & 0x1f) sndp->common.enable = 1;
			for (i = 0; i < 5; i++) sndp->ch[i].key = (v & (1 << i));
		}
	}
	else if (0x5000 <= a && a <= 0x5FFF)
	{
		sndp->common.enable = 1;
		sndp->majutushida = LinToLog(sndp->logtbl, ((Int32)(v ^ 0x00)) - 0x80);
	}
}

static void sndreset(void *ctx, Uint32 clock, Uint32 freq)
{
	SCCSOUND *sndp = ctx;
	Uint32 ch;
	XMEMSET(&sndp->common,  0, sizeof(sndp->common));
	sndp->common.cps = DivFix(clock, freq, CPS_SHIFT);
	for (ch = 0; ch < 5; ch++) SCCSoundChReset(&sndp->ch[ch], clock, freq);
	for (ch = 0x9800; ch < 0x988F; ch++) sndwrite(sndp, ch, 0);
	sndp->majutushida = LOG_KEYOFF;
}

static void sndrelease(void *ctx)
{
	SCCSOUND *sndp = ctx;
	if (sndp) {
		if (sndp->logtbl) sndp->logtbl->release(sndp->logtbl->ctx);
		XFREE(sndp);
	}
}

static void setinst(void *ctx, Uint32 n, void *p, Uint32 l){}

//ここからレジスタビュアー設定
static SCCSOUND *sndpr;
Uint32 (*ioview_ioread_DEV_SCC)(Uint32 a);
static Uint32 ioview_ioread_bf(Uint32 a){
	if(a<=0x9f)
		return sndpr->ch[(a&0xe0)>>5].tonereg[a&0x1f];
	if(a>=0xb0&&a<=0xbf)
		return sndpr->regs[a&0xf];
	return 0x100;
}
//ここまでレジスタビュアー設定

KMIF_SOUND_DEVICE *SCCSoundAlloc(void)
{
	SCCSOUND *sndp;
	sndp = XMALLOC(sizeof(SCCSOUND));
	if (!sndp) return 0;
	XMEMSET(sndp, 0, sizeof(SCCSOUND));
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
	ioview_ioread_DEV_SCC = ioview_ioread_bf;
	//ここまでレジスタビュアー設定
	return &sndp->kmif;
}
