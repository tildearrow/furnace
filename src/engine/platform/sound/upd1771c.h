#ifndef __UPD1771C_SOUND_H__
#define __UPD1771C_SOUND_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

void upd1771c_reset(void);
void upd1771c_write_packet(uint8_t ind, uint8_t val);
int16_t upd1771c_sound_stream_update(void);
void upd1771c_sound_set_clock(unsigned int clock, unsigned int divi);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
