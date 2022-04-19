#include "s_fds.h"
#include "../../format/m_nsf.h"

extern void FDSSoundInstall1(NEZ_PLAY*);
extern void FDSSoundInstall2(NEZ_PLAY*);
extern void FDSSoundInstall3(NEZ_PLAY*);
extern int FDSSoundInstallExt(NEZ_PLAY*);

void FDSSoundInstall(NEZ_PLAY *pNezPlay)
{
	switch (((NSFNSF*)pNezPlay->nsf)->fds_type)
	{
	case 1:
		FDSSoundInstall1(pNezPlay);
		break;
	case 3:
		FDSSoundInstall2(pNezPlay);
		break;
#if 0
	case 0:
		if (FDSSoundInstallExt(pNezPlay)) break;
		/* fall down */
#endif
	default:
	case 2:
		FDSSoundInstall3(pNezPlay);
		break;
	}
}

void FDSSelect(NEZ_PLAY *pNezPlay, unsigned type)
{
	if ((NSFNSF*)pNezPlay->nsf)
		((NSFNSF*)pNezPlay->nsf)->fds_type = type;
}
