#include "../../nestypes.h"
#include "../kmsnddev.h"
#include "../../format/audiosys.h"
#include "../../format/handler.h"
#include "../../format/nsf6502.h"
#include "logtable.h"
#include "m_nsf.h"
#include "s_fds.h"
//#include <math.h>
#define FDS_DYNAMIC_BIAS 1
int FDS_RealMode = 3;

#define FM_DEPTH 0 /* 0,1,2 */
#define NES_BASECYCLES (21477270)
#define PGCPS_BITS (32-16-6)
#define EGCPS_BITS (12)
#define VOL_BITS 12
#define RENDERS 4

typedef struct {
	Uint8 spd;
	Uint8 cnt;
	Uint8 mode;
	Uint8 volume;
} FDS_EG;
typedef struct {
	Uint32 spdbase;
	Uint32 spd;
	Uint32 freq;
} FDS_PG;
typedef struct {
	Uint32 phase;
	Uint32 phase2;
	Int8 wave[0x40];
	Uint8 wavereg[0x40];
	Uint8 wavptr;
	Uint32 pt;
	Int8 output;
	Int32 output32;
	Int32 output32bf;
	Uint8 disable;
	Uint8 disable2;
} FDS_WG;
typedef struct {
	FDS_EG eg;
	FDS_PG pg;
	FDS_WG wg;
	Int32 bias;
	Uint8 wavebase;
	Uint8 d[2];
} FDS_OP;

typedef struct FDSSOUND_tag {
	FDS_OP op[2];
	Uint32 phasecps;
	Uint32 envcnt;
	Uint32 envspd;
	Uint32 envcps;
	Uint8 envdisable;
	Uint8 d[3];
	Uint32 lvl;
	Int32 mastervolumel[4];
	Uint32 mastervolume;
	Uint32 srate;
	Uint8 reg[0x10];
	Uint32 count;
	Int32 realout[0x40];
	Int32 lowpass;
	Int32 outbf;
} FDSSOUND;

#if (((-1) >> 1) == -1)
/* RIGHT SHIFT IS SIGNED */
#define SSR(x, y) (((Int32)x) >> (y))
#else
/* RIGHT SHIFT IS UNSIGNED */
#define SSR(x, y) (((x) >= 0) ? ((x) >> (y)) : (-((-(x) - 1) >> (y)) - 1))
#endif


static void FDSSoundPhaseStep(FDS_OP *op , Uint32 spd)
{
	if(op->wg.disable) return;
	op->wg.pt += spd;
	while (op->wg.pt >= (1 << (PGCPS_BITS+16)))
	{
		op->wg.pt -= (1 << (PGCPS_BITS+16));
		op->bias = (op->bias + op->wg.wave[(op->wg.phase) & 0x3f]) & 0x7f;
		if((Uint8)op->wg.wave[(op->wg.phase) & 0x3f] == 64) op->bias = 0;
		op->wg.phase++;
	}
}

static void FDSSoundEGStep(FDS_EG *peg)
{
	if (peg->mode & 0x80) return;
	if (++peg->cnt <= peg->spd) return;
	peg->cnt = 0;
	if (peg->mode & 0x40)
		peg->volume += (peg->volume < 0x1f);
	else
		peg->volume -= (peg->volume > 0);

}


static Int32 __fastcall FDSSoundRender(void *pNezPlay)
{
	FDSSOUND *fdssound = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->fdssound;
	Int32 output;
	Int32 outputbuf=0,count=0;

	/* Frequency Modulator */
	fdssound->op[1].pg.spd = fdssound->op[1].pg.spdbase;
//	if (fdssound->op[1].wg.disable){
//		fdssound->op[0].pg.spd = fdssound->op[0].pg.spdbase;
//	}
//	else
	{
		Int32 v1,v2;
#if FDS_DYNAMIC_BIAS
		/* この式を変に書き換えると、ナゾラーランド第３号の爆走トモちゃんのBGMのFDS音源のピッチが
		   １オクターブ下がる恐れが非常に大きい。 */
/*
	value_1 = Sweep envelope出力値 * Sweep Bias;	// (1)
	value_2 = value_1 / 16;				// (2)

	if( (value_1 % 16) != 0 ) {			// (3)
		if( Sweep Bias >= 0 ) {
			value_2 = value_2 + 2;
		} else {
			value_2 = value_2 - 1;
		}
	}

	if( value_2 > 193 )				// (4)
		value_2 = value_2 - 258;

	Freq = Main Freq * value_2 / 64;		// (5)
	Freq = Main Freq + Freq;			// (6)
	if( Freq < 0 )
		Freq = (Main Freq * 4) + Freq;		// (7)
*/
		v1 = (Int32)((fdssound->op[1].bias & 0x40) ? (fdssound->op[1].bias - 128) : fdssound->op[1].bias) * ((Int32)(fdssound->op[1].eg.volume));
		v2 = v1 / 16;
//		v1 = ((Int32)fdssound->op[1].eg.volume) * (((Int32)(((Uint8)fdssound->op[1].wg.output) & 127)) - 64);
#else
		v1 = 0x10000 + ((Int32)fdssound->op[1].eg.volume) * (((Int32)((((Uint8)fdssound->op[1].wg.output)                      ) & 255)) - 64);
#endif
		if(v1&15){
			if(fdssound->op[1].bias & 0x40){
				v2-=1;
			}else{
				v2+=2;
			}
		}
		if(v2>193)v2-=258;

//		v1 = (((4096 + 1024 + (Int32)v1) & 0xfff)+8)/16 - 64 + (((Int32)v1 & 0xf) ? ((v1 < 0) ? -1 : 2) : 0);
//		v1 = v1<0 ? SSR(v1-8,4) : v1>0 ? SSR(v1+8,4) : 0; //doubleの無い四捨五入
		v1 = ((Int32)(fdssound->op[0].pg.freq * v2) / 64);
		v1 = v1 + (Int32)fdssound->op[0].pg.freq;
		if( v1 < 0 )
			v1 = (fdssound->op[0].pg.freq * 4) + v1;
		fdssound->op[0].pg.spd = v1 * fdssound->phasecps;
	}

	/* Accumulator */
//	output = fdssound->op[0].eg.volume;
//	if (output > 0x20) output = 0x20;
//	output = (fdssound->op[0].wg.output * output * fdssound->mastervolumel[fdssound->lvl]) >> (VOL_BITS - 4);

	/* Envelope Generator */
	if (!fdssound->envdisable && fdssound->envspd)
	{
		fdssound->envcnt += fdssound->envcps;
		while (fdssound->envcnt >= fdssound->envspd)
		{
			fdssound->envcnt -= fdssound->envspd;
			FDSSoundEGStep(&fdssound->op[1].eg);
			FDSSoundEGStep(&fdssound->op[0].eg);
		}
	}
	/* Phase Generator */
	FDSSoundPhaseStep(&fdssound->op[1] , fdssound->op[1].pg.spd);

	/* Wave Generator */
	fdssound->op[0].wg.phase2 += fdssound->op[0].pg.spd;

	output = fdssound->op[0].eg.volume;
	if (output > 0x20) output = 0x20;
	outputbuf += (fdssound->op[0].wg.output32 * output * (fdssound->mastervolumel[fdssound->lvl] >> (VOL_BITS - 2)));
	count++;

	if (fdssound->op[0].wg.disable || fdssound->op[0].wg.disable2);
	else{
		while(fdssound->op[0].wg.phase2 > (1<<(PGCPS_BITS+12))){
			fdssound->op[0].wg.phase2 -= 1<<(PGCPS_BITS+12);
			fdssound->op[0].wg.phase += 1<<(PGCPS_BITS+12);
			fdssound->op[0].wg.output32
				= fdssound->realout[fdssound->op[0].wg.wavereg[(fdssound->op[0].wg.phase >> (PGCPS_BITS+16)) & 0x3f]];
			if (output > 0x20) output = 0x20;
			outputbuf += (fdssound->op[0].wg.output32 * output * (fdssound->mastervolumel[fdssound->lvl] >> (VOL_BITS - 2)));
			count++;
		}
	}

	outputbuf = (fdssound->op[0].pg.freq != 0) ? outputbuf / 3 : 0;
	if(FDS_RealMode & 1)
	{
		fdssound->outbf += (outputbuf/count - fdssound->outbf) *4 / fdssound->lowpass;
		outputbuf = fdssound->outbf;
	}else{
		outputbuf /= count;
	}
	if(!chmask[DEV_FDS_CH1])return 0;
	return outputbuf;
}

const static NES_AUDIO_HANDLER s_fds_audio_handler[] =
{
	{ 1, FDSSoundRender, }, 
	{ 0, 0, }, 
};

static void __fastcall FDSSoundVolume(void *pNezPlay, Uint volume)
{
	FDSSOUND *fdssound = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->fdssound;
	volume += 196;
	fdssound->mastervolume = (volume << (LOG_BITS - 15 + LIN_BITS)) << 1;
	fdssound->mastervolumel[0] = LogToLinear(fdssound->mastervolume, LOG_LIN_BITS - LIN_BITS - VOL_BITS) * 2;
	fdssound->mastervolumel[1] = LogToLinear(fdssound->mastervolume, LOG_LIN_BITS - LIN_BITS - VOL_BITS) * 4 / 3;
	fdssound->mastervolumel[2] = LogToLinear(fdssound->mastervolume, LOG_LIN_BITS - LIN_BITS - VOL_BITS) * 2 / 2;
	fdssound->mastervolumel[3] = LogToLinear(fdssound->mastervolume, LOG_LIN_BITS - LIN_BITS - VOL_BITS) * 8 / 10;
}

const static NES_VOLUME_HANDLER s_fds_volume_handler[] = {
	{ FDSSoundVolume, }, 
	{ 0, }, 
};

static const Uint8 wave_delta_table[8] = {
	0,(1 << FM_DEPTH),(2 << FM_DEPTH),(4 << FM_DEPTH),
	64,256 - (4 << FM_DEPTH),256 - (2 << FM_DEPTH),256 - (1 << FM_DEPTH),
};

static void __fastcall FDSSoundWrite(void *pNezPlay, Uint address, Uint value)
{
	FDSSOUND *fdssound = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->fdssound;
	if (0x4040 <= address && address <= 0x407F)
	{
		fdssound->op[0].wg.wavereg[address - 0x4040] = value & 0x3f;
		fdssound->op[0].wg.wave[address - 0x4040] = ((int)(value & 0x3f)) - 0x20;
	}
	else if (0x4080 <= address && address <= 0x408F)
	{
		FDS_OP *pop = &fdssound->op[(address & 4) >> 2];
		fdssound->reg[address - 0x4080] = (Uint8)value;
		switch (address & 0xf)
		{
			case 0:
			case 4:
				pop->eg.mode = (Uint8)(value & 0xc0);
				if (pop->eg.mode & 0x80)
				{
					pop->eg.volume = (Uint8)(value & 0x3f);
				}
				else
				{
					pop->eg.spd = (Uint8)(value & 0x3f);
				}
				break;
			case 5:
#if 1
				fdssound->op[1].bias = (Uint8)(value & 0x7f);
#else
				fdssound->op[1].bias = (((value & 0x7f) ^ 0x40) - 0x40) & 255;
#endif
#if 0
				fdssound->op[1].wg.phase = 0;
#endif
				pop->wg.pt = 0;
				pop->wg.phase = 0;
				pop->wavebase = 0;
				break;
			case 2:	case 6:
				pop->pg.freq &= 0x00000F00;
				pop->pg.freq |= (value & 0xFF) << 0;
				pop->pg.spdbase = pop->pg.freq * fdssound->phasecps;
				if(pop->pg.freq==0){
					//pop->wg.phase = 0;
				}
				break;
			case 3:
				fdssound->envdisable = (Uint8)(value & 0x40);
//				pop->pg.spdbase = pop->pg.freq * fdssound->phasecps;
				if (value & 0x80){
					pop->wg.phase = 0;
//					pop->wg.wavptr = 0;
				}
			case 7:
				pop->pg.freq &= 0x000000FF;
				pop->pg.freq |= (value & 0x0F) << 8;
				pop->pg.spdbase = pop->pg.freq * fdssound->phasecps;
				pop->wg.disable = (Uint8)(value & 0x80);
				if (fdssound->op[1].wg.disable){
					//fdssound->op[1].bias = 0;
				}
				break;
			case 8:
				if (fdssound->op[1].wg.disable)
				{
					Int32 idx = value & 7;
					fdssound->op[1].wg.wavereg[fdssound->op[1].wg.wavptr + 0] = idx;
					fdssound->op[1].wg.wavereg[fdssound->op[1].wg.wavptr + 1] = idx;
#if FDS_DYNAMIC_BIAS
					fdssound->op[1].wg.wave[fdssound->op[1].wg.wavptr + 0] = wave_delta_table[idx];
					fdssound->op[1].wg.wave[fdssound->op[1].wg.wavptr + 1] = wave_delta_table[idx];
					fdssound->op[1].wg.wavptr = (fdssound->op[1].wg.wavptr + 2) & 0x3f;
#else
					fdssound->op[1].wavebase += wave_delta_table[idx];
					fdssound->op[1].wg.wave[fdssound->op[1].wg.wavptr + 0] = (fdssound->op[1].wavebase + fdssound->op[1].bias + 64) & 255;
					fdssound->op[1].wavebase += wave_delta_table[idx];
					fdssound->op[1].wg.wave[fdssound->op[1].wg.wavptr + 1] = (fdssound->op[1].wavebase + fdssound->op[1].bias + 64) & 255;
					fdssound->op[1].wg.wavptr = (fdssound->op[1].wg.wavptr + 2) & 0x3f;
#endif
				}
				break;
			case 9:
				fdssound->lvl = (value & 3);
				fdssound->op[0].wg.disable2 = (Uint8)(value & 0x80);
				break;
			case 10:
				fdssound->envspd = value << EGCPS_BITS;
				break;
		}
	}
}

static NES_WRITE_HANDLER s_fds_write_handler[] =
{
	{ 0x4040, 0x408F, FDSSoundWrite, },
	{ 0,      0,      0, },
};

static Uint __fastcall FDSSoundRead(void *pNezPlay, Uint address)
{
	FDSSOUND *fdssound = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->fdssound;
	if (0x4040 <= address && address <= 0x407f)
	{
		return fdssound->op[0].wg.wave[address & 0x3f] + 0x20;
	}
	if (0x4080 <= address && address <= 0x408f)
	{
		return fdssound->reg[address & 0xf];
	}
	if (0x4090 == address)
		return fdssound->op[0].eg.volume | 0x40;
	if (0x4092 == address) /* 4094? */
		return fdssound->op[1].eg.volume | 0x40;
//	if (0x4094 == address) /* 4094? */
//		return fdssound->op[1].wg.freq;
	return 0;
}

static NES_READ_HANDLER s_fds_read_handler[] =
{
	{ 0x4040, 0x409F, FDSSoundRead, },
	{ 0,      0,      0, },
};

static Uint32 DivFix(Uint32 p1, Uint32 p2, Uint32 fix)
{
	Uint32 ret;
	ret = p1 / p2;
	p1  = p1 % p2;/* p1 = p1 - p2 * ret; */
	while (fix--)
	{
		p1 += p1;
		ret += ret;
		if (p1 >= p2)
		{
			p1 -= p2;
			ret++;
		}
	}
	return ret;
}

static void __fastcall FDSSoundReset(void *pNezPlay)
{
	FDSSOUND *fdssound = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->fdssound;
	Int32 i;
	XMEMSET(fdssound, 0, sizeof(FDSSOUND));
	fdssound->srate = NESAudioFrequencyGet(pNezPlay);
	fdssound->envcps = DivFix(NES_BASECYCLES, 12 * fdssound->srate, EGCPS_BITS + 5 - 9 + 1);
	fdssound->envspd = 0xe8 << EGCPS_BITS;
	fdssound->envdisable = 1;
	fdssound->phasecps = DivFix(NES_BASECYCLES, 12 * fdssound->srate, PGCPS_BITS);
	for (i = 0; i < 0x40; i++)
	{
		fdssound->op[0].wg.wave[i] = 0;
//		fdssound->op[0].wg.wave[i] = (i < 0x20) ? 0x1f : -0x20;
		fdssound->op[1].wg.wave[i] = 64;
	}
	fdssound->op[1].wg.pt = 0;

	//リアル出力計算
#define BIT(x) ((i&(1<<x))>>x)
	for (i = 0; i < 0x40; i++)
	{
		if(FDS_RealMode & 2)
			/* FDS音源出力の際、NOT回路のIC（BU4069UB）によるローパスフィルタをかけているので、波形が上下逆になる。 */
			fdssound->realout[i]=-(i*4+(BIT(0)+BIT(1)+BIT(2)+BIT(3)+BIT(4)+BIT(5))*(0+(i*3)/0x5) - 239);
		else
			fdssound->realout[i]=i*7                                               - 239;
	}
	//ローパス計算
//	fdssound->lowpass = sqrt(fdssound->srate / 500.0);
	fdssound->lowpass = (Int32)(fdssound->srate / 11025.0 *4);
	if(fdssound->lowpass<4)fdssound->lowpass=4;
}

const static NES_RESET_HANDLER s_fds_reset_handler[] =
{
	{ NES_RESET_SYS_NOMAL, FDSSoundReset, }, 
	{ 0,                   0, }, 
};

static void __fastcall FDSSoundTerm(void* pNezPlay)
{
	FDSSOUND *fdssound = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->fdssound;
	if (fdssound)
		XFREE(fdssound);
}

const static NES_TERMINATE_HANDLER s_fds_terminate_handler[] = {
	{ FDSSoundTerm, }, 
	{ 0, }, 
};

//ここからレジスタビュアー設定
Uint8 *fds_regdata;
Uint8 *fds_regdata2;
Uint8 *fds_regdata3;
Uint32 (*ioview_ioread_DEV_FDS)(Uint32 a);
static Uint32 ioview_ioread_bf(Uint32 a){
	if(         a<=0x0f)return fds_regdata[a];
	if(a>=0x20&&a<=0x5f)return fds_regdata2[a-0x20];
	if(a>=0x70&&a<=0x8f)return fds_regdata3[(a-0x70)*2];
	else return 0x100;

}
//ここまでレジスタビュアー設定

void FDSSoundInstall3(NEZ_PLAY *pNezPlay)
{
	FDSSOUND *fdssound;
	fdssound = XMALLOC(sizeof(FDSSOUND));
	if (!fdssound) return;
	XMEMSET(fdssound, 0, sizeof(FDSSOUND));
	((NSFNSF*)pNezPlay->nsf)->fdssound = fdssound;

	LogTableInitialize();
	NESAudioHandlerInstall(pNezPlay, s_fds_audio_handler);
	NESVolumeHandlerInstall(pNezPlay, s_fds_volume_handler);
	NESTerminateHandlerInstall(&pNezPlay->nth, s_fds_terminate_handler);
	NESReadHandlerInstall(pNezPlay, s_fds_read_handler);
	NESWriteHandlerInstall(pNezPlay, s_fds_write_handler);
	NESResetHandlerInstall(pNezPlay->nrh, s_fds_reset_handler);

	//ここからレジスタビュアー設定
	fds_regdata = fdssound->reg;
	fds_regdata2 = fdssound->op[0].wg.wavereg;
	fds_regdata3 = fdssound->op[1].wg.wavereg;
	ioview_ioread_DEV_FDS = ioview_ioread_bf;
	//ここまでレジスタビュアー設定
}
