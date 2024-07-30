#include "sid3.h"

#define SAFETY_HEADER if(sid3 == NULL) return;

SID3* sid3_create()
{
    SID3* sid3 = (SID3*)malloc(sizeof(SID3));
    return sid3;
}

void sid3_reset(SID3* sid3)
{
    SAFETY_HEADER

    memset(sid3, 0, sizeof(SID3));
}

void sid3_write(SID3* sid3, uint8_t address, uint8_t data)
{
    SAFETY_HEADER
}

void sid3_clock(SID3* sid3)
{
    SAFETY_HEADER
}

void sid3_set_is_muted(SID3* sid3, uint8_t ch, bool mute)
{
    SAFETY_HEADER
}

void sid3_free(SID3* sid3)
{
    SAFETY_HEADER

    free(sid3);
}