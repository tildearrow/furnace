#ifndef S_FDS_H__
#define S_FDS_H__

#include "nezplug.h"

#ifdef __cplusplus
extern "C" {
#endif

void FDSSoundInstall(NEZ_PLAY*);
void FDSSelect(NEZ_PLAY*, unsigned type);

#ifdef __cplusplus
}
#endif
#endif /* S_FDS_H__ */
