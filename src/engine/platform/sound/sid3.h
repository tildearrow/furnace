#ifndef SID3_H
#define SID3_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define SID3_NUM_CHANNELS 7
#define SID3_NUM_FILTERS 4
#define SID3_REGISTERS_PER_CHANNEL 32
#define SID3_NUM_REGISTERS (SID3_NUM_CHANNELS * SID3_REGISTERS_PER_CHANNEL)
#define SID3_MAX_VOL 255

#define SID3_WAVETABLE_LENGTH 256

#define SID3_NUM_SINE_WAVES 8
#define SID3_SINE_WAVE_LENGTH 4096

typedef struct
{
    int input;
    int output;

    // State of filter.
    float Vhp; // highpass
    float Vbp; // bandpass
    float Vlp; // lowpass

    // Cutoff frequency, resonance.
    float w0, w0_ceil_1;
    float _1024_div_Q;

    uint16_t cutoff;
    uint8_t resonance;
} sid3_filter;

typedef struct
{
    sid3_filter filt[SID3_NUM_FILTERS];
    uint32_t connection_matrix;
    uint8_t distortion_level;
} sid3_filters_block;

typedef struct
{
    uint8_t a, d, s, sr, r, vol;
} sid3_channel_adsr;

typedef struct
{
    uint32_t accumulator;
    uint32_t frequency;

    uint32_t noise_accumulator;
    uint32_t noise_frequency;

    uint32_t lfsr, lfsr_taps;

    uint16_t waveform;
    uint16_t pw;

    uint8_t mix_mode;

    sid3_channel_adsr adsr;

    uint16_t flags;
    uint8_t ring_mod_src;
    uint8_t hard_sync_src;
    uint8_t phase_mod_source;

    sid3_filters_block filt;

    uint8_t panning;
} sid3_channel;

typedef struct
{
    uint32_t accumulator;
    uint32_t frequency;

    uint16_t streamed_sample;
    uint8_t wavetable[SID3_WAVETABLE_LENGTH];

    sid3_channel_adsr adsr;

    sid3_filters_block filt;

    uint8_t panning;
} sid3_wavetable_chan;

typedef struct
{
    uint16_t sine_waves[SID3_NUM_SINE_WAVES][SID3_SINE_WAVE_LENGTH];

    sid3_channel chan[SID3_NUM_CHANNELS - 1];
    sid3_wavetable_chan wave_chan;

    int output_l, output_r;

    //emulation-only helpers
    int channel_output[SID3_NUM_CHANNELS];
    bool muted[SID3_NUM_CHANNELS];
} SID3;

SID3* sid3_create();
void sid3_reset(SID3* sid3);
void sid3_write(SID3* sid3, uint8_t address, uint8_t data);
void sid3_clock(SID3* sid3);
void sid3_set_is_muted(SID3* sid3, uint8_t ch, bool mute);
void sid3_free(SID3* sid3);

#ifdef __cplusplus
};
#endif
#endif
