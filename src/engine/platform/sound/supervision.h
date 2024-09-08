#ifndef __SUPERVISION_SOUND_H__
#define __SUPERVISION_SOUND_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef float real;
typedef uint8_t BOOL;


typedef struct {
    uint8 reg[4];
    int on;
    uint8 waveform, volume;
    uint16 pos, size;
    uint16 count;
} SVISION_CHANNEL;

typedef struct  {
    uint8 reg[3];
    int on, right, left, play;
    uint8 type; // 6 - 7-Bit, 14 - 15-Bit
    uint16 state;
    uint8 value, volume;
    uint16 count;
    real pos, step;
} SVISION_NOISE;

typedef struct  {
    uint8 reg[5];
    int on, right, left;
    uint32 ca14to16;
    uint16 start;
    uint16 size;
    real pos, step;
} SVISION_DMA;

struct svision_t {
    SVISION_CHANNEL m_channel[2];
    // For clear sound (no grating), sync with m_channel
    SVISION_CHANNEL ch[2];
    SVISION_NOISE m_noise;
    SVISION_DMA m_dma;
    uint8 supervision_dma_mem[65536];
    uint32 decrement_tick;
    uint32 UNSCALED_CLOCK;
    uint8 ch_mask, flags;
};

void supervision_sound_reset(struct svision_t *svision);
void supervision_sound_set_clock(struct svision_t *svision, uint32 clock);
void supervision_sound_stream_update(struct svision_t *svision, uint8 *stream, uint32 len);
void supervision_sound_decrement(struct svision_t *svision);
void supervision_sound_wave_write(struct svision_t *svision, int which, int offset, uint8 data);
void supervision_sound_dma_write(struct svision_t *svision,int offset, uint8 data);
void supervision_sound_noise_write(struct svision_t *svision, int offset, uint8 data);
void supervision_sound_noise_write(struct svision_t *svision, int offset, uint8 data);
void supervision_memorymap_registers_write(struct svision_t *svision, uint32 Addr, uint8 Value);
// 12SN
void supervision_set_mute_mask(struct svision_t *svision, uint8 mask);
void supervision_sound_set_flags(struct svision_t *svision, uint8 flags_set);


#ifdef __cplusplus
} // extern "C"
#endif

#endif

