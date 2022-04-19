#include "../../nestypes.h"
#include "../kmsnddev.h"
#include "../../format/audiosys.h"
#include "../../format/handler.h"
#include "../../format/nsf6502.h"
#include "logtable.h"
#include "m_nsf.h"
#include "s_fds.h"

#define NES_BASECYCLES (21477270)
#define CPS_BITS (16)
#define CPF_BITS (12)

typedef struct {
	Uint32 phase;
	Uint32 count;
	Uint8 spd;			/* fader rate */
	Uint8 disable;		/* fader disable */
	Uint8 direction;	/* fader direction */
	Int8 volume;		/* volume */
} FADER;

typedef struct {
	Uint32 cps;		/* cycles per sample */
	Uint32 cpf;		/* cycles per frame */
	Uint32 phase;	/* wave phase */
	Uint32 spd;		/* wave speed */
	Uint32 pt;		/* programmable timer */
	Int32 ofs1;
	Int32 ofs2;
	Int32 ofs3;
	Int32 input;
	FADER fd;
	Uint8 fp;		/* frame position; */
	Uint8 lvl;
	Uint8 disable;
	Uint8 disable2;
	Int8 wave[0x40];
} FDS_FMOP;

typedef struct FDSSOUND {
	Uint32 mastervolume;
	FDS_FMOP op[2];
	Uint8 mute;
	Uint8 reg[0x10];
	Uint8 waveaddr;
} FDSSOUND;

Uint8 NSF_fds_debug_option1 = 1;
Uint8 NSF_fds_debug_option2 = 0;

static Int32 FDSSoundOperatorRender(FDS_FMOP *op)
{
	Int32 spd;
	if (op->disable) return 0;

	if (!op->disable2 && !op->fd.disable)
	{
		Uint32 fdspd;
		op->fd.count += op->cpf;
		fdspd = ((Uint32)op->fd.spd + 1) << (CPF_BITS + 11);
		if (op->fd.direction)
		{
			while (op->fd.count >= fdspd)
			{
				op->fd.count -= fdspd;
				op->fd.volume += (op->fd.volume < 0x3f);
			}
		}
		else
		{
			while (op->fd.count >= fdspd)
			{
				op->fd.count -= fdspd;
				op->fd.volume -= (op->fd.volume > 0);
			}
		}
	}

	op->pt += op->cps;
	if (op->spd <= 0x20) return 0;
	spd = op->spd + op->input + op->ofs1;
	/* if (spd > (1 << 24)) spd = (1 << 24); */
	op->phase += spd * (op->pt >> CPS_BITS);
	op->pt &= ((1 << CPS_BITS) - 1);

	return op->ofs3 + ((Int32)op->wave[(op->phase >> 24) & 0x3f] + op->ofs2) * (Int32)op->fd.volume;
}

static Int32 __fastcall FDSSoundRender(void *pNezPlay)
{
	FDSSOUND *fdssound = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->fdssound;
	fdssound->op[1].input = 0;
	fdssound->op[0].input = FDSSoundOperatorRender(&fdssound->op[1]) << (11 - fdssound->op[1].lvl);
	return (FDSSoundOperatorRender(&fdssound->op[0]) << (9 + 2 - fdssound->op[0].lvl));
}

const static NES_AUDIO_HANDLER s_fds_audio_handler[] =
{
	{ 1, FDSSoundRender, }, 
	{ 0, 0, }, 
};

static void __fastcall FDSSoundVolume(void *pNezPlay, Uint volume)
{
	FDSSOUND *fdssound = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->fdssound;
	fdssound->mastervolume = (volume << (LOG_BITS - 8)) << 1;
}

const static NES_VOLUME_HANDLER s_fds_volume_handler[] = {
	{ FDSSoundVolume, }, 
	{ 0, }, 
};

static void __fastcall FDSSoundWrite(void *pNezPlay, Uint address, Uint value)
{
	FDSSOUND *fdssound = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->fdssound;
	if (0x4040 <= address && address <= 0x407F)
	{
		fdssound->op[0].wave[address - 0x4040] = ((Int32)(value & 0x3f)) - 0x20;
	}
	else if (0x4080 <= address && address <= 0x408F)
	{
		int ch = (address >= 0x4084);
		FDS_FMOP *pop = &fdssound->op[ch];
		fdssound->reg[address - 0x4080] = (Uint8)value;
		switch (address & 15)
		{
			case 0:	case 4:
				pop->fd.disable = (Uint8)(value & 0x80);
				if (pop->fd.disable)
				{
					pop->fd.volume = (value & 0x3f);
				}
				else
				{
					pop->fd.direction = (Uint8)(value & 0x40);
					pop->fd.spd = (Uint8)(value & 0x3f);
				}
				break;
			case 5:
				{
					Int32 dat;
					if ((value & 0x7f) < 0x60)
						dat = value & 0x7f;
					else
						dat = ((Int32)(value & 0x7f)) - 0x80;
					switch (NSF_fds_debug_option1)
					{
						default:
						case 1: pop->ofs1 = dat << NSF_fds_debug_option2; break;
						case 2: pop->ofs2 = dat >> NSF_fds_debug_option2; break;
						case 3: pop->ofs3 = dat << NSF_fds_debug_option2; break;
					}
				}
				break;
			case 2:	case 6:
				pop->spd &= 0x000FF0000;
				pop->spd |= (value & 0xFF) << 8;
				break;
			case 3:
				pop->spd &= 0x0000FF00;
				pop->spd |= (value & 0xF) << 16;
				pop->disable = (Uint8)(value & 0x80);
				break;
			case 7:
				pop->spd &= 0x0000FF00;
				pop->spd |= (value & 0x7F) << 16;
				pop->disable = (Uint8)(value & 0x80);
				break;
			case 8:
				{
					static Int8 lfotbl[8] = { 0,2,4,6,-8,-6,-4,-2 };
					Int8 v = lfotbl[value & 7];
					fdssound->op[1].wave[fdssound->waveaddr++] = v;
					fdssound->op[1].wave[fdssound->waveaddr++] = v;
					if (fdssound->waveaddr == 0x40) fdssound->waveaddr = 0;
				}
				break;
			case 9:
				fdssound->op[0].lvl = (Uint8)(value & 3);
				fdssound->op[0].disable2 = (Uint8)(value & 0x80);
				break;
			case 10:
				fdssound->op[1].lvl = (Uint8)(value & 3);
				fdssound->op[1].disable2 = (Uint8)(value & 0x80);
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
	if (0x4090 <= address && address <= 0x409F)
	{
		return fdssound->reg[address - 0x4090];
	}
	return 0;
}

static NES_READ_HANDLER s_fds_read_handler[] =
{
	{ 0x4090, 0x409F, FDSSoundRead, },
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
	Uint32 i, cps, cpf;
	XMEMSET(&fdssound, 0, sizeof(FDSSOUND));
	cps = DivFix(NES_BASECYCLES, 12 * NESAudioFrequencyGet(pNezPlay), CPS_BITS);
	cpf = DivFix(NES_BASECYCLES, 12 * NESAudioFrequencyGet(pNezPlay), CPF_BITS);
	fdssound->op[0].cps = fdssound->op[1].cps = cps;
	fdssound->op[0].cpf = fdssound->op[1].cpf = cpf;
	fdssound->op[0].lvl = fdssound->op[1].lvl = 0;
#if 0
	fdssound->op[0].fd.disable = fdssound->op[1].fd.disable = 1;
	fdssound->op[0].disable = fdssound->op[1].disable = 1;
#endif

	for (i = 0; i < 0x40; i++)
	{
		fdssound->op[0].wave[i] = (i < 0x20) ? 0x1f : -0x20;
		fdssound->op[1].wave[i] = 0;
	}
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

void FDSSoundInstall2(NEZ_PLAY *pNezPlay)
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
}
