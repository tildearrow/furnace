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
#define SID3_REGISTERS_PER_CHANNEL 64
#define SID3_REGISTERS_PER_FILTER 8
#define SID3_NUM_REGISTERS (SID3_NUM_CHANNELS * SID3_REGISTERS_PER_CHANNEL)
#define SID3_MAX_VOL 255

#define SID3_WAVETABLE_LENGTH 256

#define SID3_NUM_WAVEFORM_BITS 5

#define SID3_NUM_UNIQUE_SPECIAL_WAVES 28
#define SID3_NUM_SPECIAL_WAVES (SID3_NUM_UNIQUE_SPECIAL_WAVES * 2 + 2) /* usual and 2x vol clipped + 2x vol clipped tri and saw... */
#define SID3_SPECIAL_WAVE_LENGTH 16384

#define SID3_EXPONENTIAL_LUT_LENGTH ((0xff0000 >> 8) + 1)

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

enum Filter_modes
{
    SID3_FILTER_LP = 1,
    SID3_FILTER_HP = 2,
    SID3_FILTER_BP = 4,

    SID3_FILTER_MODES_MASK = 7,

    SID3_FILTER_ENABLE = 8,
    SID3_FILTER_CHANNEL_INPUT = 16, //add channel input to filter input
    SID3_FILTER_OUTPUT = 32, //output sound from this filter to channel output
};

enum Registers
{
    SID3_REGISTER_FLAGS = 0,

    SID3_REGISTER_ADSR_A = 1,
    SID3_REGISTER_ADSR_D = 2,
    SID3_REGISTER_ADSR_S = 3,
    SID3_REGISTER_ADSR_SR = 4,
    SID3_REGISTER_ADSR_R = 5,

    SID3_REGISTER_WAVEFORM = 6,

    SID3_REGISTER_PW_HIGH = 7,
    SID3_REGISTER_PW_LOW = 8,

    SID3_REGISTER_SPECIAL_WAVE = 9,

    SID3_REGISTER_FREQ_HIGH = 10,
    SID3_REGISTER_FREQ_MID = 11,
    SID3_REGISTER_FREQ_LOW = 12,

    SID3_REGISTER_ADSR_VOL = 13,

    SID3_REGISTER_MIXMODE = 14,

    SID3_REGISTER_RING_MOD_SRC = 15,
    SID3_REGISTER_SYNC_SRC = 16,

    SID3_REGISTER_FILT_BASE = 17,
    SID3_REGISTER_FILT_MODE = 17,
    SID3_REGISTER_FILT_CUTOFF_HIGH = 18,
    SID3_REGISTER_FILT_CUTOFF_LOW = 19,
    SID3_REGISTER_FILT_RESONANCE = 20,
    SID3_REGISTER_FILT_DISTORTION = 21,
    SID3_REGISTER_FILT_CONNECTION = 22,
    SID3_REGISTER_FILT_OUTPUT_VOLUME = 23,

    SID3_REGISTER_AFTER_FILT_1ST_REG = SID3_REGISTER_FILT_BASE + SID3_REGISTERS_PER_FILTER * SID3_NUM_FILTERS,
    SID3_REGISTER_PHASE_MOD_SRC = SID3_REGISTER_AFTER_FILT_1ST_REG,

    SID3_REGISTER_PAN_LEFT = SID3_REGISTER_AFTER_FILT_1ST_REG + 1,
    SID3_REGISTER_PAN_RIGHT = SID3_REGISTER_AFTER_FILT_1ST_REG + 2,

    SID3_REGISTER_NOISE_FREQ_HIGH = SID3_REGISTER_AFTER_FILT_1ST_REG + 3,
    SID3_REGISTER_NOISE_FREQ_MID = SID3_REGISTER_AFTER_FILT_1ST_REG + 4,
    SID3_REGISTER_NOISE_FREQ_LOW = SID3_REGISTER_AFTER_FILT_1ST_REG + 5,

    SID3_REGISTER_NOISE_LFSR_HIGHEST = SID3_REGISTER_AFTER_FILT_1ST_REG + 6,
    SID3_REGISTER_NOISE_LFSR_HIGH = SID3_REGISTER_AFTER_FILT_1ST_REG + 7,
    SID3_REGISTER_NOISE_LFSR_MID = SID3_REGISTER_AFTER_FILT_1ST_REG + 8,
    SID3_REGISTER_NOISE_LFSR_LOW = SID3_REGISTER_AFTER_FILT_1ST_REG + 9,
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
} sid3_filter;

typedef struct
{
    sid3_filter filt[SID3_NUM_FILTERS];
    uint8_t connection_matrix[SID3_NUM_FILTERS];
} sid3_filters_block;

typedef struct
{
    uint8_t a, d, s, sr, r, vol, state;

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
    uint8_t clock_filter;

    uint8_t panning_left, panning_right;
    bool invert_left, invert_right; //invert channel signal

    int32_t output_before_filter;
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

    uint8_t ring_mod_src;
    uint8_t hard_sync_src;
    uint8_t phase_mod_source;

    sid3_filters_block filt;
    uint8_t clock_filter;

    uint8_t panning_left, panning_right;
    bool invert_left, invert_right; //invert channel signal

    int32_t output_before_filter;
} sid3_wavetable_chan;

typedef struct
{
    uint16_t special_waves[SID3_NUM_SPECIAL_WAVES][SID3_SPECIAL_WAVE_LENGTH]; //sine wave and OPL3-ish waves + OPZ and YMF825 waves!

    uint16_t env_counter_to_exponential_output[SID3_EXPONENTIAL_LUT_LENGTH];
    uint32_t exponential_output_to_envelope_counter[SID3_EXPONENTIAL_LUT_LENGTH];

    sid3_channel chan[SID3_NUM_CHANNELS - 1];
    sid3_wavetable_chan wave_chan;

    int32_t output_l, output_r;

    int32_t channel_signals_before_ADSR[SID3_NUM_CHANNELS];
    int32_t channel_output[SID3_NUM_CHANNELS];
    int32_t wave_channel_signal_before_ADSR;
    int32_t wave_channel_output;

    //emulation-only helpers
    bool muted[SID3_NUM_CHANNELS];
} SID3;

SID3* sid3_create();
void sid3_reset(SID3* sid3);
void sid3_write(SID3* sid3, uint16_t address, uint8_t data);
void sid3_clock(SID3* sid3);
void sid3_set_is_muted(SID3* sid3, uint8_t ch, bool mute);
void sid3_free(SID3* sid3);

#ifdef __cplusplus
};
#endif
#endif
