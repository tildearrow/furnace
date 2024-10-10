#ifndef __UPD1771C_SOUND_H__
#define __UPD1771C_SOUND_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

struct upd1771c_t {
    uint8_t upd1771c_packets[16];
    uint8_t upd1771c_mode;
    uint32_t upd1771c_pos;
    uint8_t upd1771c_off;
    uint8_t upd1771c_posc;
    uint8_t upd1771c_wave;
    uint8_t upd1771c_vol;
    uint8_t upd1771c_period;
    uint8_t upd1771c_npos;
    int upd1771c_repsamp;
};

void upd1771c_reset(struct upd1771c_t *scv);
void upd1771c_write_packet(struct upd1771c_t *scv, uint8_t ind, uint8_t val);
int16_t upd1771c_sound_stream_update(struct upd1771c_t *scv);
void upd1771c_sound_set_clock(struct upd1771c_t *scv, unsigned int clock, unsigned int divi);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
