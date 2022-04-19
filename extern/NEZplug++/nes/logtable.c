#include "../nestypes.h"
#include "logtable.h"
#include <math.h>

static Uint32 lineartbl[(1 << LIN_BITS) + 1];
static Uint32 logtbl[1 << LOG_BITS];
Uint32 LinearToLog(Int32 l)
{
	return (l < 0) ? (lineartbl[-l] + 1) : lineartbl[l];
}

Int32 LogToLinear(Uint32 l, Uint32 sft)
{
	Int32 ret;
	Uint32 ofs;
	l += sft << (LOG_BITS + 1);
	sft = l >> (LOG_BITS + 1);
	if (sft >= LOG_LIN_BITS) return 0;
	ofs = (l >> 1) & ((1 << LOG_BITS) - 1);
	ret = logtbl[ofs] >> sft;
	return (l & 1) ? -ret : ret;
}

void LogTableInitialize(void)
{
	static volatile Uint32 initialized = 0;
	Uint32 i;
	double a;
	if (initialized) return;
	initialized = 1;
	for (i = 0; i < (1 << LOG_BITS); i++)
	{
		a = (1 << LOG_LIN_BITS) / pow(2, i / (double)(1 << LOG_BITS));
		logtbl[i] = (Uint32)a;
	}
	lineartbl[0] = LOG_LIN_BITS << LOG_BITS;
	for (i = 1; i < (1 << LIN_BITS) + 1; i++)
	{
		Uint32 ua;
		a = i << (LOG_LIN_BITS - LIN_BITS);
		ua = (Uint32)((LOG_LIN_BITS - (log(a) / log(2))) * (1 << LOG_BITS));
		lineartbl[i] = ua << 1;
	}
}
