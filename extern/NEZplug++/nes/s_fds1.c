#include "../../nestypes.h"
#include "../kmsnddev.h"
#include "../../format/audiosys.h"
#include "../../format/handler.h"
#include "../../format/nsf6502.h"
#include "logtable.h"
#include "m_nsf.h"
#include "s_fds.h"

#define NES_BASECYCLES (21477270)
#define CPS_SHIFT (23)
#define PHASE_SHIFT (23)
#define FADEOUT_SHIFT 11/*(11)*/
#define XXX_SHIFT 1 /* 3 */

typedef struct {
	Uint32 wave[0x40];
	Uint32 envspd;
	Int32 envphase;
	Uint32 envout;
	Uint32 outlvl;

	Uint32 phase;
	Uint32 spd;
	Uint32 volume;
	Int32 sweep;

	Uint8 enable;
	Uint8 envmode;
	Uint8 xxxxx;
	Uint8 xxxxx2;
} FDS_FMOP;

typedef struct FDSSOUND {
	Uint32 cps;
	Int32 cycles;
	Uint32 mastervolume;
	Int32 output;

	FDS_FMOP op[2];

	Uint32 waveaddr;
	Uint8 mute;
	Uint8 key;
	Uint8 reg[0x10];
} FDSSOUND;

static Int32 __fastcall FDSSoundRender(void *pNezPlay)
{
	FDS_FMOP *pop;
	FDSSOUND *fdssound = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->fdssound;

	for (pop = &fdssound->op[0]; pop < &fdssound->op[2]; pop++)
	{
		Uint32 vol;
		if (pop->envmode)
		{
			pop->envphase -= fdssound->cps >> (FADEOUT_SHIFT - XXX_SHIFT);
			if (pop->envmode & 0x40)
				while (pop->envphase < 0)
				{
					pop->envphase += pop->envspd;
					pop->volume += (pop->volume < 0x1f);
				}
			else
				while (pop->envphase < 0)
				{
					pop->envphase += pop->envspd;
					pop->volume -= (pop->volume > 0x00);
				}
		}
		vol = pop->volume;
		if (vol)
		{
			vol += pop->sweep;
			if (vol < 0)
				vol = 0;
			else if (vol > 0x3f)
				vol = 0x3f;
		}
		pop->envout = LinearToLog(vol);
	}
	fdssound->op[1].envout += fdssound->mastervolume;

	fdssound->cycles -= fdssound->cps;
	while (fdssound->cycles < 0)
	{
		fdssound->cycles += 1 << CPS_SHIFT;
		fdssound->output = 0;
		for (pop = &fdssound->op[0]; pop < &fdssound->op[2]; pop++)
		{
			if (!pop->spd || !pop->enable)
			{
				fdssound->output = 0;
				continue;
			}
			pop->phase += pop->spd + fdssound->output;
			fdssound->output = LogToLinear(pop->envout + pop->wave[(pop->phase >> (PHASE_SHIFT - XXX_SHIFT)) & 0x3f], pop->outlvl);
		}
	}
	if (fdssound->mute) return 0;
	return fdssound->output;
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
		fdssound->op[1].wave[address - 0x4040] = LinearToLog(((Int32)value & 0x3f) - 0x20);
		/* fdssound->op[1].wave[address - 0x4040] = LinearToLog(((Int32)value & 0x7f) - 0x40); */
	}
	else if (0x4080 <= address && address <= 0x408F)
	{
		int ch = (address < 0x4084);
		FDS_FMOP *pop = &fdssound->op[ch];
		fdssound->reg[address - 0x4080] = (Uint8)value;
		switch (address & 15)
		{
			case 0:	case 4:
				if (value & 0x80)
				{
					pop->volume = (value & 0x3f);
					pop->envmode = 0;
				}
				else
				{
					pop->envspd = ((value & 0x3f) + 1) << CPS_SHIFT;
					pop->envmode = (Uint8)(0x80 | value);
				}
				break;
			case 1:	case 5:
				if (!value) break;
				if ((value & 0x7f) < 0x60)
					pop->sweep = value & 0x7f;
				else
					pop->sweep = ((Int32)value & 0x7f) - 0x80;
				break;
			case 2:	case 6:
				pop->spd &= 0x00000F00 << 7;
				pop->spd |= (value & 0xFF) << 7;
				break;
			case 3:	case 7:
				pop->spd &= 0x000000FF << 7;
				pop->spd |= (value & 0x0F) << (7 + 8);
				pop->enable = !(value & 0x80);
				break;
			case 8:
				{
					static Int8 lfotbl[8] = { 0,1,2,3,-4,-3,-2,-1 };
					Uint32 v = LinearToLog(lfotbl[value & 7]);
					fdssound->op[0].wave[fdssound->waveaddr++] = v;
					fdssound->op[0].wave[fdssound->waveaddr++] = v;
					if (fdssound->waveaddr == 0x40)
					{
						fdssound->waveaddr = 0;
					}
				}
				break;
			case 9:
				fdssound->op[0].outlvl = LOG_LIN_BITS - LIN_BITS - LIN_BITS - 10 - (value & 3);
				break;
			case 10:
				fdssound->op[1].outlvl = LOG_LIN_BITS - LIN_BITS - LIN_BITS - 10 - (value & 3);
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
	Int i;
	FDS_FMOP *pop;
	XMEMSET(&fdssound, 0, sizeof(FDSSOUND));
	fdssound->cps = DivFix(NES_BASECYCLES, 12 * (1 << XXX_SHIFT) * NESAudioFrequencyGet(pNezPlay), CPS_SHIFT);
	for (pop = &fdssound->op[0]; pop < &fdssound->op[2]; pop++)
	{
		pop->enable = 1;
	}
	fdssound->op[0].outlvl = LOG_LIN_BITS - LIN_BITS - LIN_BITS - 10;
	fdssound->op[1].outlvl = LOG_LIN_BITS - LIN_BITS - LIN_BITS - 10;
	for (i = 0; i < 0x40; i++)
	{
		fdssound->op[1].wave[i] = LinearToLog((i < 0x20)?0x1f:-0x20);
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

void FDSSoundInstall1(NEZ_PLAY *pNezPlay)
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
