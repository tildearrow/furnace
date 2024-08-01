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

#define SID3_NUM_WAVEFORM_BITS 5

#define SID3_NUM_UNIQUE_SPECIAL_WAVES 28
#define SID3_NUM_SPECIAL_WAVES (SID3_NUM_UNIQUE_SPECIAL_WAVES * 2 + 2) /* usual and 2x vol clipped + 2x vol clipped tri and saw... */
#define SID3_SPECIAL_WAVE_LENGTH 16384

#define SID3_ACC_BITS 30
#define SID3_ACC_MASK ((1UL << SID3_ACC_BITS) - 1)

enum Flags
{
    SID3_CHAN_ENABLE_GATE = 1,
    SID3_CHAN_ENABLE_RING_MOD = 2,
    SID3_CHAN_ENABLE_HARD_SYNC = 4,
    SID3_CHAN_ENABLE_PHASE_MOD = 8,
    SID3_CHAN_PHASE_RESET = 16,
    SID3_CHAN_ENV_RESET = 32,
    SID3_CHAN_NOISE_PHASE_RESET = 64,
    SID3_CHAN_1_BIT_NOISE = 128,
};

enum Waveforms
{
    SID3_WAVE_TRIANGLE = 1,
    SID3_WAVE_SAW = 2,
    SID3_WAVE_PULSE = 4,
    SID3_WAVE_NOISE = 8,
    SID3_WAVE_SPECIAL = 16,
};

enum Mixmodes
{
    SID3_MIX_8580 = 0,
    SID3_MIX_AND = 1,
    SID3_MIX_OR = 2,
    SID3_MIX_XOR = 3,
    SID3_MIX_SUM = 4,
};

typedef struct
{
    int32_t input;
    int32_t output;

    // State of filter.
    float Vhp; // highpass
    float Vbp; // bandpass
    float Vlp; // lowpass

    // Cutoff frequency, resonance.
    float w0, w0_ceil_1;
    float _1024_div_Q;

    uint16_t cutoff;
    uint8_t resonance;
    uint8_t distortion_level;

    uint8_t mode;

    uint8_t output_volume;

    bool channel_output; //output to the channel master output
} sid3_filter;

typedef struct
{
    sid3_filter filt[SID3_NUM_FILTERS];
    uint32_t connection_matrix;
} sid3_filters_block;

typedef struct
{
    uint8_t a, d, s, sr, r, vol, state;

    uint32_t rate_counter;
    uint32_t rate_period;
    uint32_t envelope_counter;
    uint32_t envelope_speed;
    bool hold_zero;
} sid3_channel_adsr;

typedef struct
{
    uint32_t accumulator;
    uint8_t sync_bit;
    uint32_t frequency;

    uint32_t noise_accumulator;
    uint32_t noise_frequency;

    uint32_t lfsr, lfsr_taps;

    uint8_t waveform;
    uint8_t special_wave;

    uint16_t pw;

    uint8_t mix_mode;

    sid3_channel_adsr adsr;

    uint8_t flags;

    uint8_t ring_mod_src;
    uint8_t hard_sync_src;
    uint8_t phase_mod_source;

    sid3_filters_block filt;

    uint8_t panning_left, panning_right;
    bool invert_left, invert_right; //invert channel signal
} sid3_channel;

typedef struct
{
    uint32_t accumulator;
    uint8_t sync_bit;
    uint32_t frequency;

    uint16_t streamed_sample;
    uint8_t wavetable[SID3_WAVETABLE_LENGTH];

    uint8_t wave_address;

    sid3_channel_adsr adsr;

    uint8_t flags;
    uint8_t mode;

    sid3_filters_block filt;

    uint8_t panning_left, panning_right;
    bool invert_left, invert_right; //invert channel signal
} sid3_wavetable_chan;

typedef struct
{
    uint16_t special_waves[SID3_NUM_SPECIAL_WAVES][SID3_SPECIAL_WAVE_LENGTH]; //sine wave and OPL3-ish waves + OPZ and YMF825 waves!

    sid3_channel chan[SID3_NUM_CHANNELS - 1];
    sid3_wavetable_chan wave_chan;

    int32_t output_l, output_r;

    int32_t channel_output[SID3_NUM_CHANNELS];

    //emulation-only helpers
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
