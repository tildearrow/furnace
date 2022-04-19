#include "../../nestypes.h"
#include "../kmsnddev.h"
#include "../../format/audiosys.h"
#include "../../format/handler.h"
#include "../../format/nsf6502.h"
#include "logtable.h"
#include "m_nsf.h"
#include "s_fme7.h"
#include "../s_psg.h"

#define BASECYCLES_ZX  (3579545)/*(1773400)*/
#define BASECYCLES_AMSTRAD  (2000000)
#define BASECYCLES_MSX (3579545)
#define BASECYCLES_NES (21477270)
#define FME7_VOL 4/5

typedef struct {
	KMIF_SOUND_DEVICE *psgp;
	Uint8 adr;
} PSGSOUND;

static Int32 __fastcall PSGSoundRender(void* pNezPlay)
{
	PSGSOUND *psgs = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->psgs;
	Int32 b[2] = {0, 0};
	psgs->psgp->synth(psgs->psgp->ctx, b);
	return b[0]*FME7_VOL;
}

static void __fastcall PSGSoundRender2(void* pNezPlay, Int32 *d)
{
	PSGSOUND *psgs = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->psgs;
	Int32 b[2] = {0, 0};
	psgs->psgp->synth(psgs->psgp->ctx, b);
	d[0] += b[0]*FME7_VOL;
	d[1] += b[0]*FME7_VOL;
}

const static NES_AUDIO_HANDLER s_psg_audio_handler[] = {
	{ 1, PSGSoundRender}, 
//	{ 3, PSGSoundRender, PSGSoundRender2, }, 
	{ 0, 0, 0, }, 
};

static void __fastcall PSGSoundVolume(void* pNezPlay, Uint volume)
{
	PSGSOUND *psgs = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->psgs;
	psgs->psgp->volume(psgs->psgp->ctx, volume);
}

const static NES_VOLUME_HANDLER s_psg_volume_handler[] = {
	{ PSGSoundVolume, }, 
	{ 0, }, 
};

static Uint __fastcall PSGSoundReadData(void *pNezPlay, Uint address)
{
	PSGSOUND *psgs = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->psgs;
	return psgs->psgp->read(psgs->psgp->ctx, 0);
}

static void __fastcall PSGSoundWrireAddr(void *pNezPlay, Uint address, Uint value)
{
	PSGSOUND *psgs = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->psgs;
	psgs->adr = (Uint8)value;
	psgs->psgp->write(psgs->psgp->ctx, 0, value);
}
static void __fastcall PSGSoundWrireData(void *pNezPlay, Uint address, Uint value)
{
	PSGSOUND *psgs = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->psgs;
	psgs->psgp->write(psgs->psgp->ctx, 1, value);
}

static NES_WRITE_HANDLER s_fme7_write_handler[] =
{
	{ 0xC000, 0xC000, PSGSoundWrireAddr, },
	{ 0xE000, 0xE000, PSGSoundWrireData, },
	{ 0,      0,      0, },
};

static void __fastcall FME7SoundReset(void* pNezPlay)
{
	PSGSOUND *psgs = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->psgs;
	psgs->psgp->reset(psgs->psgp->ctx, BASECYCLES_NES / 12, NESAudioFrequencyGet(pNezPlay));
}

const static NES_RESET_HANDLER s_fme7_reset_handler[] = {
	{ NES_RESET_SYS_NOMAL, FME7SoundReset, }, 
	{ 0,                   0, }, 
};

static void __fastcall PSGSoundTerm(void* pNezPlay)
{
	PSGSOUND *psgs = ((NSFNSF*)((NEZ_PLAY*)pNezPlay)->nsf)->psgs;
	if (psgs->psgp)
	{
		psgs->psgp->release(psgs->psgp->ctx);
		psgs->psgp = 0;
	}
	XFREE(psgs);
}

const static NES_TERMINATE_HANDLER s_psg_terminate_handler[] = {
	{ PSGSoundTerm, }, 
	{ 0, }, 
};

void FME7SoundInstall(NEZ_PLAY* pNezPlay)
{
	PSGSOUND *psgs;
	psgs = XMALLOC(sizeof(PSGSOUND));
	if (!psgs) return;
	XMEMSET(psgs, 0, sizeof(PSGSOUND));
	((NSFNSF*)pNezPlay->nsf)->psgs = psgs;

	psgs->psgp = PSGSoundAlloc(PSG_TYPE_YM2149); //エンベロープ31段階あったんでYM2149系でしょう。
	if (!psgs->psgp) return;

	LogTableInitialize();
	NESTerminateHandlerInstall(&pNezPlay->nth, s_psg_terminate_handler);
	NESVolumeHandlerInstall(pNezPlay, s_psg_volume_handler);

	NESAudioHandlerInstall(pNezPlay, s_psg_audio_handler);
	NESWriteHandlerInstall(pNezPlay, s_fme7_write_handler);
	NESResetHandlerInstall(pNezPlay->nrh, s_fme7_reset_handler);
}
