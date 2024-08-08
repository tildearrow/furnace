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
enum {FALSE, TRUE};

void supervision_sound_reset(void);
void supervision_sound_set_clock(uint32 clock);
void supervision_sound_stream_update(uint8 *stream, uint32 len);
void supervision_sound_decrement(void);
void supervision_sound_wave_write(int which, int offset, uint8 data);
void supervision_sound_dma_write(int offset, uint8 data);
void supervision_sound_noise_write(int offset, uint8 data);
void supervision_sound_noise_write(int offset, uint8 data);
void supervision_memorymap_registers_write(uint32 Addr, uint8 Value);
// 12SN
void supervision_set_mute_mask(uint32 mask);
void supervision_sound_set_flags(uint32 flags_set);

extern uint8 supervision_dma_mem[65536];

#ifdef __cplusplus
} // extern "C"
#endif

#endif

