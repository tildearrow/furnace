#ifndef S_PSG_H__
#define S_PSG_H__

#include "kmsnddev.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
	PSG_TYPE_AY_3_8910	= 0,	/* YAMAHA AY-3-8910 */
	PSG_TYPE_YM2149		= 1		/* YAMAHA YM2149 */
};

KMIF_SOUND_DEVICE *PSGSoundAlloc(Uint32 psg_type);

#ifdef __cplusplus
}
#endif

#endif /* S_PSG_H__ */
