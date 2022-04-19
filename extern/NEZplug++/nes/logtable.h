#include "../nestypes.h"

#define LOG_BITS 12
#define LIN_BITS 8
#define LOG_LIN_BITS 30

Uint32 LinearToLog(Int32 l);
Int32 LogToLinear(Uint32 l, Uint32 sft);
void LogTableInitialize(void);
