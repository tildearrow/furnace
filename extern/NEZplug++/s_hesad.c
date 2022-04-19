#include "kmsnddev.h"
#include "s_hesad.h"
#include "opl/s_deltat.h"

#define CPS_SHIFT 16
#define PCE_VOLUME 1 //1
#define ADPCM_VOLUME 50

typedef struct {
	KMIF_SOUND_DEVICE kmif;
	KMIF_SOUND_DEVICE *deltadev;
	struct {
		Int32 mastervolume;
		Int32 cps;
		Int32 pt;
	} common;
	Uint8 pcmbuf[0x10000];
	Uint8 port[0x10];
	Uint8 regs[0x18];
	Uint32 outfreq;
	Uint32 freq;
	Uint16 addr;
	Uint16 writeptr;
	Uint16 readptr;
	Int8 playflag;
	Int8 repeatflag;
	Int32 length;
	Int32 volume;
	Int32 fadetimer;
	Int32 fadecount;
} HESADPCM;


static void HESAdPcmReset(HESADPCM *sndp)
{
	sndp->addr = 0;
	sndp->freq = 0;
	sndp->writeptr = 0;
	sndp->readptr = 0;
	sndp->playflag = 0;
	sndp->repeatflag = 0;
	sndp->length = 0;
	sndp->volume = 0xff;
	sndp->deltadev->write(sndp->deltadev,0,1);
}

static void sndsynth(void *ctx, Int32 *p)
{
	HESADPCM *sndp = ctx;
	Int32 pbf[2];
	pbf[0]=0;pbf[1]=0;
	
	//この時既に、内蔵音源のレンダリングが終了している。
	p[0]=p[0]*PCE_VOLUME;
	p[1]=p[1]*PCE_VOLUME;

	sndp->deltadev->synth(sndp->deltadev,pbf);

	sndp->common.pt += sndp->common.cps;

	//1ms
	while(sndp->common.pt > 100000){
		sndp->common.pt -= 100000;

		if(sndp->fadecount > 0 && sndp->fadetimer){
			sndp->fadecount--;
			sndp->volume = 0xff * sndp->fadecount / sndp->fadetimer;
		}
		if(sndp->fadecount < 0 && sndp->fadetimer){
			sndp->fadecount++;
			sndp->volume = 0xff - (0xff * sndp->fadecount / sndp->fadetimer);
		}
		
	}
//	if(sndp->common.pt > 500)p[0]+=80000;
	p[0]+=(pbf[0] * ADPCM_VOLUME * sndp->volume / 0xff);
	p[1]+=(pbf[1] * ADPCM_VOLUME * sndp->volume / 0xff);
}

static void sndreset(void *ctx, Uint32 clock, Uint32 freq)
{
	HESADPCM *sndp = ctx;
	XMEMSET(&sndp->pcmbuf, 0, sizeof(sndp->pcmbuf));
	XMEMSET(&sndp->port, 0, sizeof(sndp->port));
	HESAdPcmReset(sndp);
	sndp->outfreq = freq;
	sndp->fadetimer = 0;
	sndp->fadecount = 0;
	sndp->common.cps = 100000000/freq;
	sndp->common.pt = 0;
	sndp->volume = 0xff;
	sndp->deltadev->reset(sndp->deltadev,clock,freq);
	sndp->deltadev->write(sndp->deltadev,1,0);
	sndp->deltadev->write(sndp->deltadev,0xb,0xff);
//	sndp->deltadev->setinst(sndp->deltadev,0,sndp->pcmbuf,0x100);

}

static void sndwrite(void *ctx, Uint32 a, Uint32 v)
{
	HESADPCM *sndp = ctx;
	sndp->port[a & 15] = v;
	sndp->regs[a & 15] = v;
	switch(a & 15)
	{
	  case 0x8:
		// port low
		sndp->addr &= 0xff00;
		sndp->addr |= v;
		break;
	  case 0x9:
		// port high
		sndp->addr &= 0xff;
		sndp->addr |= v << 8;
		break;
	  case 0xA:
		// write buffer
		sndp->pcmbuf[sndp->writeptr++] = v;
		break;
	  case 0xB:
		// DMA busy?
		break;
	  case 0xC:
		break;
	  case 0xD:
		if (v & 0x80)
		{
			// reset
			HESAdPcmReset(sndp);
		}
		if ((v & 0x03) == 0x03)
		{
			// set write pointer
			sndp->writeptr = sndp->addr;
			sndp->regs[0x10]=sndp->writeptr&0xff;
			sndp->regs[0x11]=sndp->writeptr>>8;
		}
		if (v & 0x08)
		{
			// set read pointer
			sndp->readptr = sndp->addr ? sndp->addr-1 : sndp->addr;
			sndp->regs[0x12]=sndp->readptr&0xff;
			sndp->regs[0x13]=sndp->readptr>>8;
		}
		if (v & 0x10)
		{
			sndp->length = sndp->addr;
			sndp->regs[0x14]=sndp->length&0xff;
			sndp->regs[0x15]=sndp->length>>8;
		}
		sndp->repeatflag = ((v & 0x20) == 0x20);
		sndp->playflag = ((v & 0x40) == 0x40);
		if(sndp->playflag){
			sndp->deltadev->write(sndp->deltadev,2,sndp->readptr & 0xff);
			sndp->deltadev->write(sndp->deltadev,3,(sndp->readptr >> 8) & 0xff);
			sndp->deltadev->write(sndp->deltadev,4,(sndp->length + sndp->readptr) & 0xff);
			sndp->deltadev->write(sndp->deltadev,5,((sndp->length + sndp->readptr) >> 8) & 0xff);
			sndp->deltadev->write(sndp->deltadev,0,1);
			sndp->deltadev->write(sndp->deltadev,0,(0x80|(sndp->repeatflag>>1)));
		}
		break;
	  case 0xE:
		// set freq
		sndp->freq = 7111 / (16 - (v & 15));
		sndp->deltadev->write(sndp->deltadev,0x9,sndp->freq & 0xff);
		sndp->deltadev->write(sndp->deltadev,0xa,(sndp->freq >> 8) & 0xff);
		break;
	  case 0xF:
		// fade out
		switch(v & 15)
		{
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
			sndp->fadetimer = 0;
			sndp->fadecount = sndp->fadetimer;
			sndp->volume = 0xff;
			break;
		case 0x8:
			sndp->fadetimer = -100;
			sndp->fadecount = sndp->fadetimer;
			break;
		case 0xa:
			sndp->fadetimer = 5000;
			sndp->fadecount = sndp->fadetimer;
			break;
		case 0xc:
			sndp->fadetimer = -100;
			sndp->fadecount = sndp->fadetimer;
			break;
		case 0xe:
			sndp->fadetimer = 1500;
			sndp->fadecount = sndp->fadetimer;
			break;
		}

		break;
	}
}

static Uint32 sndread(void *ctx, Uint32 a)
{
	HESADPCM *sndp = ctx;
	switch(a & 15)
	  {
		case 0xa:
			return sndp->pcmbuf[sndp->readptr++];
		case 0xb:
			return sndp->port[0xb] & ~1;
		case 0xc:
			if (!sndp->playflag)
			{
				sndp->port[0xc] |= 1;
				sndp->port[0xc] &= ~8;
			} else {
				sndp->port[0xc] &= ~1;
				sndp->port[0xc] |= 8;
			}
		  return sndp->port[0xc];
		case 0xd:
		  return 0;
//		case 0xe:
//		  return sndp->volume;
		default:
		  return 0xff;
	}
}

#define LOG_BITS 12
static void sndvolume(void *ctx, Int32 volume)
{
	HESADPCM *sndp = ctx;
	volume = (volume << (LOG_BITS - 8)) << 1;
	sndp->common.mastervolume = volume;

	sndp->deltadev->volume(sndp->deltadev,volume);
}

static void sndrelease(void *ctx)
{
	HESADPCM *sndp = ctx;
	
	sndp->deltadev->release(sndp->deltadev);

	if (sndp)
		XFREE(sndp);
}

static void setinst(void *ctx, Uint32 n, void *p, Uint32 l){}

//ここからレジスタビュアー設定
static Uint8 *regdata;
extern Uint32 (*ioview_ioread_DEV_ADPCM)(Uint32 a);
static Uint32 ioview_ioread_bf(Uint32 a){
	if(a>=0x8 && a<=0x15)return regdata[a];else return 0x100;
}
//ここまでレジスタビュアー設定

KMIF_SOUND_DEVICE *HESAdPcmAlloc(void)
{
	HESADPCM *sndp;
	sndp = XMALLOC(sizeof(HESADPCM));
	if (!sndp) return 0;
	XMEMSET(sndp, 0, sizeof(HESADPCM));
	sndp->kmif.ctx = sndp;
	sndp->kmif.release = sndrelease;
	sndp->kmif.reset = sndreset;
	sndp->kmif.synth = sndsynth;
	sndp->kmif.volume = sndvolume;
	sndp->kmif.write = sndwrite;
	sndp->kmif.read = sndread;
	sndp->kmif.setinst = setinst;
	
	//ここからレジスタビュアー設定
	regdata = sndp->regs;
	ioview_ioread_DEV_ADPCM = ioview_ioread_bf;
	//ここまでレジスタビュアー設定

	//発声部分
	sndp->deltadev = YMDELTATPCMSoundAlloc(3,sndp->pcmbuf);
	return &sndp->kmif;
}
