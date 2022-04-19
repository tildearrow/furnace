#include "../../nestypes.h"
#include "../kmsnddev.h"
#include "../../format/audiosys.h"
#include "../../format/handler.h"
#include "../../format/nsf6502.h"
#include "logtable.h"
#include "m_nsf.h"
#include "s_n106.h"

#define NES_BASECYCLES (21477270)
#define CPS_SHIFT (16)
#define PHASE_SHIFT (16+2)
#define  N106SOUND_DATA 0x4800
#define  N106SOUND_ADDR 0xF800

#define REAL_RENDERS 6
#define REAL_OFS_BASE 200000
#define REAL_OFS_COUNT 16
#define RENDERS 4
#define NAMCO106_VOL Namco106_Volume/16
#define CPSF_SHIFT 4
int Namco106_Realmode = 0;
int Namco106_Volume = 16;

typedef struct {
	Uint32 logvol;
	Uint32 cycles;
	Uint32 cycles2;
	Uint32 spd;
	Uint32 phase;
	Uint32 tlen;

	Uint8 update;
	Uint8 freql;
	Uint8 freqm;
	Uint8 freqh;
	Uint8 vreg;
	Uint8 tadr;
	Uint8 nazo;
	Uint8 mute;
	Int32 output;
	Uint32 count;
} N106_WM;

typedef struct {
	Uint32 cps;
	Uint32 cpsf;
	Int32 output;
	Int32 offset;
	Uint32 ofscount;
	Uint32 ofscps;
	Uint8 outputfg;
	Uint32 mastervolume;

	N106_WM ch[8];

	Uint8 addressauto;
	Uint8 address;
	Uint8 chinuse;

	Uint32 tone[0x100];	/* TONE DATA */
	Uint8 data[0x80];
} N106SOUND;

static Uint32 DivFix(Uint32 p1, Uint32 p2, Uint32 fix)
{
	Uint32 ret;
	ret = p1 / p2;
	p1 %= p2;/* p1 = p1 - p2 * ret; */
	//while(p1 >= p2)p1 -= p2;
	while (fix--)
	{
		//p1 += p1;
		//ret += ret;
		p1 <<= 1;
		ret <<= 1;
		if (p1 >= p2)
		{
			p1 -= p2;
			ret++;
		}
	}
	return ret;
}

__inline static void UPDATE(N106_WM *chp)
{
	if (chp->update & 3)
	{
		Uint32 freq;
		freq  = ((int)chp->freql);
		freq += ((int)chp->freqm) << 8;
		freq += ((int)chp->freqh) << 16;
		chp->spd = freq & 0x3ffff;
	}
	if (chp->update & 2)
	{
		Uint32 tlen;
		tlen = (0x100 - (chp->freqh & 0xfc)) << PHASE_SHIFT;
		if (chp->tlen != tlen)
		{
			chp->tlen = tlen;
			chp->phase = 0;
		}
	}
	if (chp->update & 4)
	{
		chp->logvol = LinearToLog((chp->vreg & 0x0f) << 2);
	}
	chp->update = 0;
}

static Int32 N106SoundRenderReal2(void* pNezPlay)
{
	N106SOUND *n106s = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->n106s;
	N106_WM *chp;

	Int32 outputbuf=0,count=0,accum=0,chpn,real2=((1<<REAL_RENDERS) / n106s->chinuse);
	Uint32 cyclesspd = n106s->chinuse << CPS_SHIFT;

	//リアルモード
	/*波形は、1chずつ出力される。
	  波形データ、基準は"8"。volを下げると、8に向かって+-が減衰していく。
	  波形データ8を再生中は高周波ノイズは出ない。8からの差とノイズの大きさは比例する。
	*/
	for (chp = &n106s->ch[0],chpn = 0
		; chp < &n106s->ch[8]; chp++,chpn++)
	{
		accum = 0;
		count = 0;
		if (chp->mute || !chp->logvol) continue;
		if (chp->update) UPDATE(chp);
		chp->cycles2 += n106s->cps << REAL_RENDERS;
		chp->output = LogToLinear(n106s->tone[((chp->phase >> PHASE_SHIFT) + chp->tadr) & 0xff] + chp->logvol + n106s->mastervolume, LOG_LIN_BITS - LIN_BITS - LIN_BITS - 11);
		while (chp->cycles2 >= n106s->cpsf){
			if (((Int32)chp->count / real2) + 8 - n106s->chinuse == chpn)accum += chp->output;
			count++;
			chp->cycles2 -= n106s->cpsf;
			chp->count++;
			if(chp->count >= (1<<REAL_RENDERS)){
				chp->count = 0;
				chp->cycles += n106s->cpsf;

				chp->phase += chp->spd * (chp->cycles / cyclesspd);
				chp->cycles %= cyclesspd;
				//while(chp->cycles >= cyclesspd)chp->cycles -= cyclesspd;
				chp->phase %= chp->tlen;
				//while(chp->phase >= chp->tlen)chp->phase -= chp->tlen;
				chp->output = LogToLinear(n106s->tone[((chp->phase >> PHASE_SHIFT) + chp->tadr) & 0xff] + chp->logvol + n106s->mastervolume, LOG_LIN_BITS - LIN_BITS - LIN_BITS - 11);
			}
		}
		if (((Int32)chp->count / real2) == chpn)accum += chp->output;
		count++;
		if (chmask[DEV_N106_CH1+chpn])outputbuf += accum / count;
	}
/*	n106s->ofscount += n106s->ofscps;
	while(n106s->ofscount >= REAL_OFS_COUNT){
		n106s->ofscount -= REAL_OFS_COUNT;
		n106s->offset += (outputbuf - n106s->offset) / 64; 
	}
	return (outputbuf - n106s->offset) * NAMCO106_VOL;
*/	return outputbuf * NAMCO106_VOL;
}

static Int32 N106SoundRenderReal(void* pNezPlay)
{
	N106SOUND *n106s = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->n106s;
	N106_WM *chp;

	Int32 outputbuf=0,count=0,accum=0,chpn;
	Uint32 cyclesspd = n106s->chinuse << CPS_SHIFT;

	//リアルモード
	for (chp = &n106s->ch[8 - n106s->chinuse],chpn = 8 - n106s->chinuse
		; chp < &n106s->ch[8]; chp++,chpn++)
	{
		accum = 0;
		count = 0;
		if (chp->mute || !chp->logvol) continue;
		if (chp->update) UPDATE(chp);
		chp->cycles2 += n106s->cps << REAL_RENDERS;
		chp->output = LogToLinear(n106s->tone[((chp->phase >> PHASE_SHIFT) + chp->tadr) & 0xff] + chp->logvol + n106s->mastervolume, LOG_LIN_BITS - LIN_BITS - LIN_BITS - 9);
		while (chp->cycles2 >= n106s->cpsf){
			accum += chp->output;
			count++;
			chp->cycles2 -= n106s->cpsf;
			chp->count++;
			if(chp->count >= (1<<REAL_RENDERS)){
				chp->count = 0;
				chp->cycles += n106s->cpsf;

				chp->phase += chp->spd * (chp->cycles / cyclesspd);
				chp->cycles %= cyclesspd;
				//while(chp->cycles >= cyclesspd)chp->cycles -= cyclesspd;
				chp->phase %= chp->tlen;
				//while(chp->phase >= chp->tlen)chp->phase -= chp->tlen;
				chp->output = LogToLinear(n106s->tone[((chp->phase >> PHASE_SHIFT) + chp->tadr) & 0xff] + chp->logvol + n106s->mastervolume, LOG_LIN_BITS - LIN_BITS - LIN_BITS - 9);
			}
		}
		accum += chp->output;
		count++;
		if(chmask[DEV_N106_CH1+chpn])outputbuf += accum / count;
	}
	return outputbuf * NAMCO106_VOL;
}


static Int32 N106SoundRenderNormal(void* pNezPlay)
{
	N106SOUND *n106s = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->n106s;
	N106_WM *chp;

	Int32 outputbuf=0,count=0,accum=0,chpn;
	Uint32 cyclesspd = n106s->chinuse << CPS_SHIFT;
	//従来の方法
	for (chp = &n106s->ch[8 - n106s->chinuse],chpn = 8 - n106s->chinuse
		; chp < &n106s->ch[8]; chp++,chpn++)
	{
		if (chp->mute || !chp->logvol) continue;
		accum = 0;
		count = 0;
		if (chp->update) UPDATE(chp);
		chp->cycles += n106s->cps << RENDERS;
		chp->output = LogToLinear(n106s->tone[((chp->phase >> PHASE_SHIFT) + chp->tadr) & 0xff] + chp->logvol + n106s->mastervolume, LOG_LIN_BITS - LIN_BITS - LIN_BITS - 9);
		while (chp->cycles >= cyclesspd)
		{
			accum += chp->output;
			count++;
			chp->cycles -= cyclesspd;
//			chp->count++;
//			if(chp->count >= (1<<RENDERS)){
				chp->count = 0;
				chp->phase += chp->spd >> RENDERS;
				//chp->phase %= chp->tlen;
				if(chp->phase >= chp->tlen)
					do{
						chp->phase -= chp->tlen;
					}while(chp->phase >= chp->tlen);
				chp->output = LogToLinear(n106s->tone[((chp->phase >> PHASE_SHIFT) + chp->tadr) & 0xff] + chp->logvol + n106s->mastervolume, LOG_LIN_BITS - LIN_BITS - LIN_BITS - 9);
//			}
		}
		accum += chp->output;
		count++;
		if(chmask[DEV_N106_CH1+chpn])outputbuf += accum / count;
	}
	return outputbuf * NAMCO106_VOL;
}
static Int32 __fastcall N106SoundRender(void* pNezPlay)
{
	N106SOUND *n106s = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->n106s;
	switch(Namco106_Realmode){
	case 1:
		return n106s->chinuse < 8 ? N106SoundRenderReal(pNezPlay) : N106SoundRenderReal2(pNezPlay);
	case 2:
		return N106SoundRenderReal(pNezPlay);
	case 3:
		return N106SoundRenderReal2(pNezPlay);
	default:
		return N106SoundRenderNormal(pNezPlay);
	}
}

const static NES_AUDIO_HANDLER s_n106_audio_handler[] = {
	{ 1, N106SoundRender, }, 
	{ 0, 0, }, 
};

static void __fastcall N106SoundVolume(void* pNezPlay, Uint volume)
{
	N106SOUND *n106s = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->n106s;
	n106s->mastervolume = (volume << (LOG_BITS - 8)) << 1;
}

const static NES_VOLUME_HANDLER s_n106_volume_handler[] = {
	{ N106SoundVolume, }, 
	{ 0, }, 
};

static void __fastcall N106SoundWriteAddr(void *pNezPlay, Uint address, Uint value)
{
	N106SOUND *n106s = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->n106s;
	n106s->address     = (Uint8)(value & 0x7f);
	n106s->addressauto = (value & 0x80) ? 1 : 0;
}

static void __fastcall N106SoundWriteData(void *pNezPlay, Uint address, Uint value)
{
	N106SOUND *n106s = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->n106s;
	n106s->data[n106s->address] = (Uint8)value;
	n106s->tone[n106s->address * 2]     = LinearToLog(((Int32)(value & 0xf) << 2) - 0x20);
	n106s->tone[n106s->address * 2 + 1] = LinearToLog(((Int32)(value >>  4) << 2) - 0x20);
	if (n106s->address >= 0x40)
	{
		N106_WM *chp = &n106s->ch[(n106s->address - 0x40) >> 3];
		switch (n106s->address & 7)
		{
/*
$78 0-7：周波数レジスタ加算値（0-7bit）
$79 0-7：周波数レジスタ代入値（0-7bit）
$7a 0-7：周波数レジスタ加算値（8-15bit）
$7b 0-7：周波数レジスタ代入値（8-15bit）
$7c 0-1：周波数レジスタ加算値（16-17bit）
$7c 2-7：サンプル数（256-8*nサンプル）
$7d 0-7：周波数レジスタ代入値（16-24bit）
$7e 0-7：再生波形位置
$7f 0-3：音量
$7f 4-6：使用チャンネル数（$7Fのみ。ほかのchでは意味なし）			
*/
			case 0:
				chp->update |= 1;
				chp->freql = (Uint8)value;
				break;
			case 1:
				chp->phase &= (0xffff01<<(PHASE_SHIFT-16))-1;
				chp->phase |= value<<(PHASE_SHIFT-16);
				chp->phase %= chp->tlen;
				break;
			case 2:
				chp->update |= 1;
				chp->freqm = (Uint8)value;
				break;
			case 3:
				chp->phase &= (0xff01<<(PHASE_SHIFT-8))-1;
				chp->phase |= value<<(PHASE_SHIFT-8);
				chp->phase %= chp->tlen;
				break;
			case 4:
				chp->update |= 2;
				chp->freqh = (Uint8)value;
				break;
			case 5:
				chp->phase = value << PHASE_SHIFT;
				chp->phase %= chp->tlen;
				break;
			case 6:
				chp->tadr = (Uint8)(value & 0xff);
				break;
			case 7:
				chp->update |= 4;
				chp->vreg = (Uint8)value;
				chp->nazo = (Uint8)((value >> 4) & 0x07);
				if (chp == &n106s->ch[7]){
					n106s->chinuse = 1 + chp->nazo;
					n106s->cpsf = DivFix(NES_BASECYCLES, NES_BASECYCLES / n106s->chinuse / CPSF_SHIFT, CPS_SHIFT);
				}
				break;
		}
	}
	if (n106s->addressauto)
	{
		n106s->address = (n106s->address + 1) & 0x7f;
	}
}

static NES_WRITE_HANDLER s_n106_write_handler[] =
{
	{ N106SOUND_DATA, N106SOUND_DATA, N106SoundWriteData, },
	{ N106SOUND_ADDR, N106SOUND_ADDR, N106SoundWriteAddr, },
	{ 0,              0,              0, },
};

static Uint __fastcall N106SoundReadData(void *pNezPlay, Uint address)
{
	N106SOUND *n106s = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->n106s;
	Uint ret = n106s->data[n106s->address];
	if (n106s->addressauto)
	{
		n106s->address = (n106s->address + 1) & 0x7f;
	}
	return ret;
}

static Uint __fastcall N106SoundReadDataDebug(void *pNezPlay, Uint address)
{
	N106SOUND *n106s = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->n106s;
	return n106s->data[address & 0x7f];
}

static NES_READ_HANDLER s_n106_read_handler[] =
{
	{ N106SOUND_DATA, N106SOUND_DATA, N106SoundReadData, },
//	{ 0x3000		, 0x307f		, N106SoundReadDataDebug, },
	{ 0,              0,              0, },
};

static void __fastcall N106SoundReset(void* pNezPlay)
{
	N106SOUND *n106s = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->n106s;
	int i,j;
	XMEMSET(n106s, 0, sizeof(N106SOUND));

	//波形データの初期化
	for (j = 0; j < 0xff; j++)
		n106s->tone[j] =  LinearToLog((0) - 0x20);

	for (i = 0; i < 8; i++)
	{
		n106s->ch[i].tlen = 0x10 << PHASE_SHIFT;
		n106s->ch[i].logvol = LinearToLog(0);
	}
	n106s->addressauto = 1;
	n106s->chinuse = 8;
	n106s->cps = DivFix(NES_BASECYCLES, 45 * NESAudioFrequencyGet(pNezPlay), CPS_SHIFT);
	n106s->cpsf = DivFix(NES_BASECYCLES, NES_BASECYCLES / n106s->chinuse / CPSF_SHIFT, CPS_SHIFT);
	n106s->output = 0;
	n106s->outputfg = 0;

	n106s->ofscps = REAL_OFS_BASE * REAL_OFS_COUNT / NESAudioFrequencyGet(pNezPlay);
}

const static NES_RESET_HANDLER s_n106_reset_handler[] = {
	{ NES_RESET_SYS_NOMAL, N106SoundReset, }, 
	{ 0,                   0, }, 
};


static void __fastcall N106SoundTerm(void* pNezPlay)
{
	N106SOUND *n106s = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->n106s;
	if (n106s)
		XFREE(n106s);
}

const static NES_TERMINATE_HANDLER s_n106_terminate_handler[] = {
	{ N106SoundTerm, }, 
	{ 0, }, 
};

//ここからレジスタビュアー設定
Uint8 *n106_regdata;
Uint32 (*ioview_ioread_DEV_N106)(Uint32 a);
static Uint32 ioview_ioread_bf(Uint32 a){
	if(a<=0x7f)return n106_regdata[a];else return 0x100;
}
//ここまでレジスタビュアー設定

void N106SoundInstall(NEZ_PLAY *pNezPlay)
{
	N106SOUND *n106s;
	n106s = XMALLOC(sizeof(N106SOUND));
	if (!n106s) return;
	XMEMSET(n106s, 0, sizeof(N106SOUND));
	((NSFNSF*)pNezPlay->nsf)->n106s = n106s;

	LogTableInitialize();
	NESAudioHandlerInstall(pNezPlay, s_n106_audio_handler);
	NESVolumeHandlerInstall(pNezPlay, s_n106_volume_handler);
	NESTerminateHandlerInstall(&pNezPlay->nth, s_n106_terminate_handler);
	NESReadHandlerInstall(pNezPlay, s_n106_read_handler);
	NESWriteHandlerInstall(pNezPlay, s_n106_write_handler);
	NESResetHandlerInstall(pNezPlay->nrh, s_n106_reset_handler);

	//ここからレジスタビュアー設定
	n106_regdata = n106s->data;
	ioview_ioread_DEV_N106 = ioview_ioread_bf;
	//ここまでレジスタビュアー設定

}
