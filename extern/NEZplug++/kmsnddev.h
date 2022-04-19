/* libnezp by Mamiya */

#ifndef KMSNDDEV_H__
#define KMSNDDEV_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "../nestypes.h"

typedef struct {
	void *ctx;
	void (*release)(void *ctx);
	void (*reset)(void *ctx, Uint32 clock, Uint32 freq);
	void (*synth)(void *ctx, Int32 *p);
	void (*volume)(void *ctx, Int32 v);
	void (*write)(void *ctx, Uint32 a, Uint32 v);
	Uint32 (*read)(void *ctx, Uint32 a);
	void (*setinst)(void *ctx, Uint32 n, void *p, Uint32 l);
#if 0
	void (*setrate)(void *ctx, Uint32 clock, Uint32 freq);
	void (*getinfo)(void *ctx, KMCH_INFO *cip, );
	void (*volume2)(void *ctx, Uint8 *volp, Uint32 numch);
	/* 0x00(mute),0x70(x1/2),0x80(x1),0x90(x2) */
#endif
} KMIF_SOUND_DEVICE;

//チャンネルマスク用
enum{//順番を変えたら恐ろしいことになる
	DEV_2A03_SQ1,
	DEV_2A03_SQ2,
	DEV_2A03_TR,
	DEV_2A03_NOISE,
	DEV_2A03_DPCM,

	DEV_FDS_CH1,

	DEV_MMC5_SQ1,
	DEV_MMC5_SQ2,
	DEV_MMC5_DA,

	DEV_VRC6_SQ1,
	DEV_VRC6_SQ2,
	DEV_VRC6_SAW,

	DEV_N106_CH1,
	DEV_N106_CH2,
	DEV_N106_CH3,
	DEV_N106_CH4,
	DEV_N106_CH5,
	DEV_N106_CH6,
	DEV_N106_CH7,
	DEV_N106_CH8,

	DEV_DMG_SQ1,
	DEV_DMG_SQ2,
	DEV_DMG_WM,
	DEV_DMG_NOISE,

	DEV_HUC6230_CH1,
	DEV_HUC6230_CH2,
	DEV_HUC6230_CH3,
	DEV_HUC6230_CH4,
	DEV_HUC6230_CH5,
	DEV_HUC6230_CH6,

	DEV_AY8910_CH1,
	DEV_AY8910_CH2,
	DEV_AY8910_CH3,

	DEV_SN76489_SQ1,
	DEV_SN76489_SQ2,
	DEV_SN76489_SQ3,
	DEV_SN76489_NOISE,

	DEV_SCC_CH1,
	DEV_SCC_CH2,
	DEV_SCC_CH3,
	DEV_SCC_CH4,
	DEV_SCC_CH5,

	DEV_YM2413_CH1,
	DEV_YM2413_CH2,
	DEV_YM2413_CH3,
	DEV_YM2413_CH4,
	DEV_YM2413_CH5,
	DEV_YM2413_CH6,
	DEV_YM2413_CH7,
	DEV_YM2413_CH8,
	DEV_YM2413_CH9,
	DEV_YM2413_BD,
	DEV_YM2413_HH,
	DEV_YM2413_SD,
	DEV_YM2413_TOM,
	DEV_YM2413_TCY,

	DEV_ADPCM_CH1,

	DEV_MSX_DA,

	DEV_MAX,
};
extern Uint8 chmask[0x80];

#ifdef __cplusplus
}
#endif
#endif /* KMSNDDEV_H__ */


