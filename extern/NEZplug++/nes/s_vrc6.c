#include "../../nestypes.h"
#include "../kmsnddev.h"
#include "../../format/audiosys.h"
#include "../../format/handler.h"
#include "../../format/nsf6502.h"
#include "logtable.h"
#include "m_nsf.h"
#include "s_vrc6.h"
//#include <stdio.h>

#define NES_BASECYCLES (21477270)
#define CPS_SHIFT 16
#define RENDERS 7
typedef struct {
	Uint32 cps;
	Int32 cycles;

	Uint32 spd;
	Int32 output;

	Uint8 regs[3];
	Uint8 update;
	Uint8 adr;
	Uint8 mute;
	Uint32 ct;
} VRC6_SQUARE;

typedef struct {
	Uint32 cps;
	Int32 cycles;

	Uint32 spd;
	Uint32 output;
	Uint32 outputbf;

	Uint8 regs[3];
	Uint8 update;
	Uint8 adr;
	Uint8 mute;
	Uint32 ct;
} VRC6_SAW;

typedef struct {
	VRC6_SQUARE square[2];
	VRC6_SAW saw;
	Uint32 mastervolume;
} VRC6SOUND;

/* ------------ */
/*  VRC6 SOUND  */
/* ------------ */

static Int32 VRC6SoundSquareRender(void* pNezPlay, VRC6_SQUARE *ch)
{
	VRC6SOUND *vrc6s = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->vrc6s;
	Int32 outputbuf=0,count=0;
	if (ch->update)
	{
		if (ch->update & (2 | 4))
		{
			ch->spd = ((((ch->regs[2] & 0x0F) << 8) + ch->regs[1] + 1) << CPS_SHIFT) >> RENDERS;
		}
		ch->update = 0;
	}

	if (ch->spd < (8 << CPS_SHIFT >> RENDERS)) return 0;

	ch->cycles += ch->cps;
	ch->output = LinearToLog(ch->regs[0] & 0x0F) + vrc6s->mastervolume;
	ch->output = LogToLinear(ch->output, LOG_LIN_BITS - LIN_BITS - 16 - 1);
	ch->output *= (!(ch->regs[0] & 0x80) && (ch->adr < ((ch->regs[0] >> 4) + 1))) ? 0 : 1;
	while (ch->cycles > 0)
	{
		outputbuf += ch->output;
		count++;

		ch->cycles -= ch->spd;
		ch->ct++;

		if(ch->ct >= (1<<RENDERS)){
			ch->ct = 0;
			ch->adr++;
			ch->adr &= 0xF;

			ch->output = LinearToLog(ch->regs[0] & 0x0F) + vrc6s->mastervolume;
			ch->output = LogToLinear(ch->output, LOG_LIN_BITS - LIN_BITS - 16 - 1);
			ch->output *= (!(ch->regs[0] & 0x80) && (ch->adr < ((ch->regs[0] >> 4) + 1))) ? 0 : 1;
		}
	}
	outputbuf += ch->output;
	count++;

	if (ch->mute || !(ch->regs[2] & 0x80)) return 0;

	return outputbuf /count;
}

static Int32 VRC6SoundSawRender(void *pNezPlay, VRC6_SAW *ch)
{
	VRC6SOUND *vrc6s = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->vrc6s;
	Int32 outputbuf=0,count=0;

	if (ch->update)
	{
		if (ch->update & (2 | 4))
		{
			ch->spd = (((ch->regs[2] & 0x0F) << 8) + ch->regs[1] + 1) << CPS_SHIFT;
		}
		ch->update = 0;
	}

	if (ch->spd < (8 << CPS_SHIFT)) return 0;

	ch->cycles -= ch->cps << 6;

	ch->outputbf = LinearToLog((ch->output >> 3) & 0x1F) + vrc6s->mastervolume;
	ch->outputbf = LogToLinear(ch->outputbf, LOG_LIN_BITS - LIN_BITS - 16 - 1);
	while (ch->cycles < 0)
	{
		outputbuf += ch->outputbf;
		count++;

		ch->cycles += ch->spd;
		ch->ct++;

		if(ch->ct >= (1<<6)){
			ch->ct = 0;
			ch->output += (ch->regs[0] & 0x3F);
			if (7 == ++ch->adr)
			{
				ch->adr = 0;
				ch->output = 0;
			}
			ch->outputbf = LinearToLog((ch->output >> 3) & 0x1F) + vrc6s->mastervolume;
			ch->outputbf = LogToLinear(ch->outputbf, LOG_LIN_BITS - LIN_BITS - 16 - 1);
		}
	}
	outputbuf += ch->outputbf;
	count++;

	if (ch->mute || !(ch->regs[2] & 0x80)) return 0;

	return outputbuf /count;
}

static Int32 __fastcall VRC6SoundRender(void* pNezPlay)
{
	VRC6SOUND *vrc6s = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->vrc6s;
	Int32 accum = 0;
	accum += VRC6SoundSquareRender(pNezPlay, &vrc6s->square[0]) * chmask[DEV_VRC6_SQ1];
	accum += VRC6SoundSquareRender(pNezPlay, &vrc6s->square[1]) * chmask[DEV_VRC6_SQ2];
	accum += VRC6SoundSawRender(pNezPlay, &vrc6s->saw) * chmask[DEV_VRC6_SAW];
	return accum;
}

const static NES_AUDIO_HANDLER s_vrc6_audio_handler[] = {
	{ 1, VRC6SoundRender, }, 
	{ 0, 0, }, 
};

static void __fastcall VRC6SoundVolume(void* pNezPlay, Uint volume)
{
	VRC6SOUND *vrc6s = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->vrc6s;
	volume += 64;
	vrc6s->mastervolume = (volume << (LOG_BITS - 8)) << 1;
}

const static NES_VOLUME_HANDLER s_vrc6_volume_handler[] = {
	{ VRC6SoundVolume, },
	{ 0, }, 
};

static void __fastcall VRC6SoundWrite9000(void *pNezPlay, Uint address, Uint value)
{
	//FILE *f;
	if(address >=0x9000 && address <= 0x9002){//なして9010・9030書いたときもここ通ってんだべ？
		VRC6SOUND *vrc6s = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->vrc6s;
		vrc6s->square[0].regs[address & 3] = (Uint8)value;
		vrc6s->square[0].update |= 1 << (address & 3); 
	}
	//f=fopen("R:\\debug.out","a+");
	//fprintf(f,"%04X: %02X\r\n",address, value);
	//fclose(f);
}
static void __fastcall VRC6SoundWriteA000(void *pNezPlay, Uint address, Uint value)
{
	VRC6SOUND *vrc6s = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->vrc6s;
	vrc6s->square[1].regs[address & 3] = (Uint8)value;
	vrc6s->square[1].update |= 1 << (address & 3); 
}
static void __fastcall VRC6SoundWriteB000(void *pNezPlay, Uint address, Uint value)
{
	VRC6SOUND *vrc6s = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->vrc6s;
	vrc6s->saw.regs[address & 3] = (Uint8)value;
	vrc6s->saw.update |= 1 << (address & 3); 
}

static NES_WRITE_HANDLER s_vrc6_write_handler[] =
{
	{ 0x9000, 0x9002, VRC6SoundWrite9000, },
	{ 0xA000, 0xA002, VRC6SoundWriteA000, },
	{ 0xB000, 0xB002, VRC6SoundWriteB000, },
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

static void VRC6SoundSquareReset(void *pNezPlay, VRC6_SQUARE *ch)
{
	ch->cps = DivFix(NES_BASECYCLES, 12 * NESAudioFrequencyGet(pNezPlay), CPS_SHIFT);
}

static void VRC6SoundSawReset(void *pNezPlay, VRC6_SAW *ch)
{
	ch->cps = DivFix(NES_BASECYCLES, 24 * NESAudioFrequencyGet(pNezPlay), CPS_SHIFT);
}

static void __fastcall VRC6SoundReset(void* pNezPlay)
{
	VRC6SOUND *vrc6s = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->vrc6s;
	XMEMSET(vrc6s, 0, sizeof(VRC6SOUND));
	VRC6SoundSquareReset(pNezPlay, &vrc6s->square[0]);
	VRC6SoundSquareReset(pNezPlay, &vrc6s->square[1]);
	VRC6SoundSawReset(pNezPlay, &vrc6s->saw);
}

const static NES_RESET_HANDLER s_vrc6_reset_handler[] = {
	{ NES_RESET_SYS_NOMAL, VRC6SoundReset, }, 
	{ 0,                   0, }, 
};

static void __fastcall VRC6SoundTerm(void* pNezPlay)
{
	VRC6SOUND *vrc6s = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->vrc6s;
	if (vrc6s)
		XFREE(vrc6s);
}

const static NES_TERMINATE_HANDLER s_vrc6_terminate_handler[] = {
	{ VRC6SoundTerm, }, 
	{ 0, }, 
};

//ここからレジスタビュアー設定
Uint8 *vrc6_regdata;
Uint8 *vrc6_regdata2;
Uint8 *vrc6_regdata3;
Uint32 (*ioview_ioread_DEV_VRC6)(Uint32 a);
static Uint32 ioview_ioread_bf(Uint32 a){
	if(a>=0x00 && a<=0x02)return vrc6_regdata [a     ];else
	if(a>=0x10 && a<=0x12)return vrc6_regdata2[a-0x10];else
	if(a>=0x20 && a<=0x22)return vrc6_regdata3[a-0x20];else return 0x100;
}
//ここまでレジスタビュアー設定

void VRC6SoundInstall(NEZ_PLAY *pNezPlay)
{
	VRC6SOUND *vrc6s;
	vrc6s = XMALLOC(sizeof(VRC6SOUND));
	if (!vrc6s) return;
	XMEMSET(vrc6s, 0, sizeof(VRC6SOUND));
	((NSFNSF*)pNezPlay->nsf)->vrc6s = vrc6s;

	LogTableInitialize();
	NESAudioHandlerInstall(pNezPlay, s_vrc6_audio_handler);
	NESVolumeHandlerInstall(pNezPlay, s_vrc6_volume_handler);
	NESTerminateHandlerInstall(&pNezPlay->nth, s_vrc6_terminate_handler);
	NESWriteHandlerInstall(pNezPlay, s_vrc6_write_handler);
	NESResetHandlerInstall(pNezPlay->nrh, s_vrc6_reset_handler);

	//ここからレジスタビュアー設定
	vrc6_regdata  = vrc6s->square[0].regs;
	vrc6_regdata2 = vrc6s->square[1].regs;
	vrc6_regdata3 = vrc6s->saw.regs;
	ioview_ioread_DEV_VRC6 = ioview_ioread_bf;
	//ここまでレジスタビュアー設定
}
