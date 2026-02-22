/* sgu.c/sgu.h - SGU-1 Sound Generator Unit 1
 *
 * Copyright (C) 2025 Tomasz "smokku" Sterna
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define _USE_MATH_DEFINES
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

#include "./sgu.h"

#define minval(a, b)   (((a) < (b)) ? (a) : (b))
#define maxval(a, b)   (((a) > (b)) ? (a) : (b))
#define clamp(v, a, b) minval((b), maxval((a), (v)))

// SGU implemented on RP2350 MCU uses hardware specific shortcuts
#ifdef SGU_ON_MCU
#include <pico.h>
// PCM sample memory (signed 8-bit)
static int8_t __uninitialized_ram(pcm_mem)[SGU_PCM_RAM_SIZE];
#else
#include <stdlib.h>

#define __uninitialized_ram(x) x

static inline int32_t __builtin_arm_ssat(int32_t val, unsigned bits)
{
    const int32_t max = (1 << (bits - 1)) - 1;
    const int32_t min = -(1 << (bits - 1));
    return val > max ? max : val < min ? min
                                       : val;
}
static inline uint32_t __builtin_arm_usat(int32_t val, unsigned bits)
{
    const uint32_t max = (1u << bits) - 1;
    return val < 0 ? 0 : (uint32_t)val > max ? max
                                             : (uint32_t)val;
}

#ifdef __clang__
#pragma clang diagnostic ignored "-Wunknown-attributes"
#endif

#if defined(PICO_BUILD)
#error "Are you building for microcontroller? You need to define SGU_ON_MCU."
#endif
#endif

// precomputed waveforms (1024 samples each)
static int16_t __attribute__((aligned(2)))
__uninitialized_ram(sine_lut)[SGU_WAVEFORM_LENGTH];
static int16_t __attribute__((aligned(2)))
__uninitialized_ram(triangle_lut)[SGU_WAVEFORM_LENGTH];
static int16_t __attribute__((aligned(2)))
__uninitialized_ram(sawtooth_lut)[SGU_WAVEFORM_LENGTH];
// precomputed envelope attenuation to volume
static uint16_t __attribute__((aligned(2)))
__uninitialized_ram(env_gain_lut)[0x400];
// Panning gain lookup tables
static uint8_t __uninitialized_ram(pan_gain_lut_l)[256];
static uint8_t __uninitialized_ram(pan_gain_lut_r)[256];

static inline uint32_t sgu_clz32(uint32_t value)
{
#if defined(_MSC_VER)
    unsigned long index;
    return _BitScanReverse(&index, value) ? (31u - (uint32_t)index) : 32u;
#elif defined(__GNUC__) || defined(__clang__)
    return value ? (uint32_t)__builtin_clz(value) : 32u;
#else
    if (value == 0)
        return 32u;
    uint32_t count = 0;
    while ((value & 0x80000000u) == 0u)
    {
        value <<= 1;
        count++;
    }
    return count;
#endif
}

//-------------------------------------------------
//  svf_saturate - cheap soft saturation for analog warmth
//  Prevents harsh digital clipping, emulates analog op-amp behavior.
//  Uses piecewise linear approximation (no division, no float).
//  Knee at ±24576, 4:1 compression ratio above knee, max at ±32767.
//-------------------------------------------------
static inline int32_t svf_saturate(int32_t x)
{
    if (x > 24576)
        return 24576 + ((x - 24576) >> 2); // Compress above knee
    if (x < -24576)
        return -24576 + ((x + 24576) >> 2); // Compress below knee
    return x;
}

// uint32_t EG_CLOCK_DIVIDER: The clock divider of the envelope generator
static const uint32_t EG_CLOCK_DIVIDER = 3;

// "quiet" value, used to optimize when we can skip doing work
static const uint32_t EG_QUIET = 0x380;

//-------------------------------------------------
//  opl_key_scale_atten - converts an
//  OPL concatenated block (3 bits) and fnum
//  (10 bits) into an attenuation offset; values
//  here are for 6dB/octave, in 0.75dB units
//  (matching total level LSB)
//-------------------------------------------------
static inline uint32_t opl_key_scale_atten(uint32_t block, uint32_t fnum_4msb)
{
    // this table uses the top 4 bits of FNUM and are the maximal values
    // (for when block == 7). Values for other blocks can be computed by
    // subtracting 8 for each block below 7.
    static uint8_t const fnum_to_atten[16] = {0, 24, 32, 37, 40, 43, 45, 47, 48, 50, 51, 52, 53, 54, 55, 56};
    int32_t result = fnum_to_atten[fnum_4msb] - 8 * (block ^ 7);
    return (uint32_t)__builtin_arm_usat(result, 31);
}

//-------------------------------------------------
//  detune_adjustment - given a 5-bit key code
//  value and a 3-bit detune parameter, return a
//  6-bit signed phase displacement; this table
//  has been verified against Nuked's equations,
//  but the equations are rather complicated, so
//  we'll keep the simplicity of the table
//-------------------------------------------------
static inline int32_t detune_adjustment(uint32_t detune, uint32_t keycode)
{
    // Detune uses following encoding:
    //   0 = -3 (strongest negative)
    //   1 = -2
    //   2 = -1
    //   3 =  0 (no detune)
    //   4 = +1
    //   5 = +2
    //   6 = +3 (strongest positive)
    //   7 =  0 (no detune, degenerate)
    static uint8_t const s_detune_adjustment[32][4] = {
        // clang-format off
        { 0,  0,  1,  2 },  { 0,  0,  1,  2 },  { 0,  0,  1,  2 },  { 0,  0,  1,  2 },
        { 0,  1,  2,  2 },  { 0,  1,  2,  3 },  { 0,  1,  2,  3 },  { 0,  1,  2,  3 },
        { 0,  1,  2,  4 },  { 0,  1,  3,  4 },  { 0,  1,  3,  4 },  { 0,  1,  3,  5 },
        { 0,  2,  4,  5 },  { 0,  2,  4,  6 },  { 0,  2,  4,  6 },  { 0,  2,  5,  7 },
        { 0,  2,  5,  8 },  { 0,  3,  6,  8 },  { 0,  3,  6,  9 },  { 0,  3,  7, 10 },
        { 0,  4,  8, 11 },  { 0,  4,  8, 12 },  { 0,  4,  9, 13 },  { 0,  5, 10, 14 },
        { 0,  5, 11, 16 },  { 0,  6, 12, 17 },  { 0,  6, 13, 19 },  { 0,  7, 14, 20 },
        { 0,  8, 16, 22 },  { 0,  8, 16, 22 },  { 0,  8, 16, 22 },  { 0,  8, 16, 22 }
        // clang-format on
    };
    int32_t result = s_detune_adjustment[keycode][detune & 3];
    return (detune & 0b100) ? -result : result;
}

// Combined freq16 decode: computes keycode (0..31), KSL block, and fnum_4msb
// from a single CLZ, merging keycode_from_freq16_32 and freq16_to_ksl_params.
static inline void freq16_decode(uint16_t freq16,
                                 uint32_t *keycode, uint32_t *block_out, uint32_t *fnum_4msb_out)
{
    if (freq16 < 0x0100)
    {
        *keycode = 0;
        *block_out = 0;
        *fnum_4msb_out = 0;
        return;
    }
    uint32_t msb = 31u - sgu_clz32((uint32_t)freq16); // 8..15
    uint32_t block = msb - 8u;                        // 0..7
    uint32_t mant2 = (freq16 >> (msb - 2)) & 3u;      // 0..3
    *keycode = (block << 2) | mant2;                  // 0..31
    *block_out = block;
    *fnum_4msb_out = (freq16 >> (msb - 4)) & 0x0F; // top 4 bits after implicit leading 1
}

// SGU uses SID semantics with Fclk = 1,000,000
// with sample rate 48,000.
static inline uint32_t sgu_phase_step_from_freq_clamped(int32_t f)
{
    // USAT: single-cycle unsigned saturate to 0..65535
    uint32_t freq = (uint32_t)__builtin_arm_usat(f, 16);

    // Fclk=1_000_000, Fs=48_000 => factor = 16000/3
    uint32_t phase_step = freq * 16000u; // <= ~1.05e9 fits in 32-bit unsigned
    return (phase_step + 1u) / 3u;
}

static inline uint32_t sgu_phase_step_freq16(uint16_t freq16, int32_t lfo_raw_pm)
{
    // shift=10 => depth=1 gives ~±13.5 cents peak on the max PM step
    const int32_t delta = ((int32_t)freq16 * lfo_raw_pm) >> 10;
    return sgu_phase_step_from_freq_clamped((int32_t)freq16 + delta);
}

//-------------------------------------------------
//  compute_eg_sustain - compute the sustain level
//  shifted up to envelope values
//-------------------------------------------------
static inline uint32_t compute_eg_sustain(uint8_t op_reg[])
{
    // 4-bit sustain level, but 15 means 31 so effectively 5 bits
    uint32_t eg_sustain = SGU_OP3_SL(op_reg[3]);
    eg_sustain |= (eg_sustain + 1) & 0x10;
    eg_sustain <<= 5;
    return eg_sustain;
}

//-------------------------------------------------
//  compute_multiplier - compute the x.1 multiplier value
//  from the raw 4-bit multiple register value
//-------------------------------------------------
static inline uint32_t compute_multiplier(uint32_t mul)
{
    // output values are x2 of OPL's x.1 domain:
    // {0,1,2,3,4,5,6,7,8,9,10,10,12,12,15,15} * 2, with 0->1.
    static const uint8_t s_multiplier_table[16] = {
        1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 20, 24, 24, 30, 30};
    return s_multiplier_table[mul & 0x0F];
}

static inline uint16_t compute_fixed_base_freq16(uint32_t mul)
{
    // base = 8 + (mul * 247 + 7) / 15 for mul 0..15
    static const uint8_t s_fixed_base_table[16] = {
        8, 24, 41, 57, 74, 90, 107, 123, 140, 156, 173, 189, 206, 222, 239, 255};
    return s_fixed_base_table[mul & 0x0F];
}

//-------------------------------------------------
//  compute_phase_step_from_base - compute ratio-mode
//  phase step from a precomputed base step
//-------------------------------------------------
static inline uint32_t sgu_compute_phase_step_from_base(uint32_t phase_step, uint32_t keycode, uint32_t multiplier, uint8_t detune3)
{
    // apply detune based on the keycode
    // DT1 -> small signed adjustment, scaled into step32 units
    // OPM applies detune BEFORE multiplier: result = (base + detune) * multiplier
    // This way detune effect scales with the multiplier, matching OPM behavior
    int32_t adj = detune_adjustment(detune3, keycode); // about -22..+22

    // Cortex-M33 SMULL is single-cycle; use full 64-bit multiply
    int32_t det_step = (int32_t)(((int64_t)phase_step * adj) >> 12);

    // Apply detune to phase_step BEFORE multiplier (like OPM does)
    int32_t adjusted_step = (int32_t)phase_step + det_step;
    if (adjusted_step < 0)
        adjusted_step = 0;

    // Apply frequency multiplier (which is an x.1 value)
    return ((uint32_t)adjusted_step * multiplier) >> 1;
}

// helper to apply KSR to the raw ADSR rate, ignoring ksr if the
// raw value is 0, and clamping to 63
static inline uint32_t effective_rate(uint32_t rawrate, uint32_t ksr)
{
    return (rawrate == 0) ? 0 : __builtin_arm_usat(rawrate + ksr, 6);
}

//-------------------------------------------------
//  attenuation_increment - given a 6-bit ADSR
//  rate value and a 3-bit stepping index,
//  return a 4-bit increment to the attenuation
//  for this step (or for the attack case, the
//  fractional scale factor to decrease by)
//-------------------------------------------------
static inline uint32_t attenuation_increment(uint32_t rate, uint32_t index)
{
    static uint32_t const s_increment_table[64] = {
        0x00000000, 0x00000000, 0x10101010, 0x10101010, // 0-3    (0x00-0x03)
        0x10101010, 0x10101010, 0x11101110, 0x11101110, // 4-7    (0x04-0x07)
        0x10101010, 0x10111010, 0x11101110, 0x11111110, // 8-11   (0x08-0x0B)
        0x10101010, 0x10111010, 0x11101110, 0x11111110, // 12-15  (0x0C-0x0F)
        0x10101010, 0x10111010, 0x11101110, 0x11111110, // 16-19  (0x10-0x13)
        0x10101010, 0x10111010, 0x11101110, 0x11111110, // 20-23  (0x14-0x17)
        0x10101010, 0x10111010, 0x11101110, 0x11111110, // 24-27  (0x18-0x1B)
        0x10101010, 0x10111010, 0x11101110, 0x11111110, // 28-31  (0x1C-0x1F)
        0x10101010, 0x10111010, 0x11101110, 0x11111110, // 32-35  (0x20-0x23)
        0x10101010, 0x10111010, 0x11101110, 0x11111110, // 36-39  (0x24-0x27)
        0x10101010, 0x10111010, 0x11101110, 0x11111110, // 40-43  (0x28-0x2B)
        0x10101010, 0x10111010, 0x11101110, 0x11111110, // 44-47  (0x2C-0x2F)
        0x11111111, 0x21112111, 0x21212121, 0x22212221, // 48-51  (0x30-0x33)
        0x22222222, 0x42224222, 0x42424242, 0x44424442, // 52-55  (0x34-0x37)
        0x44444444, 0x84448444, 0x84848484, 0x88848884, // 56-59  (0x38-0x3B)
        0x88888888, 0x88888888, 0x88888888, 0x88888888  // 60-63  (0x3C-0x3F)
    };
    return (s_increment_table[rate] >> (4 * index)) & 0xF;
}

// freq16_to_ksl_params removed -- merged into freq16_decode()

//-------------------------------------------------
//  compute_eg_rate - compute the envelope rate
//  for the given envelope state, including KSR
//-------------------------------------------------
static inline uint8_t compute_eg_rate(uint8_t op_reg[], uint32_t keycode, enum envelope_state state)
{
    // OPM-style 5-bit keycode (block + top 2 frac bits)
    uint32_t ksrval = keycode >> (SGU_OP0_KSR(op_reg[0]) ^ 3);
    uint32_t rawrate;
    switch (state)
    {
    case SGU_EG_ATTACK:
        rawrate = SGU_OP27_AR(op_reg[2], op_reg[7]) * 2;
        break;
    case SGU_EG_DECAY:
        rawrate = SGU_OP27_DR(op_reg[2], op_reg[7]) * 2;
        break;
    case SGU_EG_SUSTAIN:
        rawrate = SGU_OP4_SR(op_reg[4]) * 2;
        break;
    case SGU_EG_RELEASE:
    default:
    {
        uint8_t rr = SGU_OP3_RR(op_reg[3]);
        rawrate = rr ? rr * 4 + 2 : 0;
        break;
    }
    }
    uint32_t rate = effective_rate(rawrate, ksrval);
    // Note: SGU EG runs at 16kHz (48kHz/3), OPN/ESFM at ~17.7kHz
    // This results in ~10% slower envelope timing compared to ESFM
    return (uint8_t)rate;
}

//-------------------------------------------------
//  clock_lfo - clock the global LFO for AM and PM
//  Called once per sample (global state)
//-------------------------------------------------
static inline int32_t clock_lfo(uint16_t *lfo_am_counter, uint16_t *lfo_pm_counter, uint8_t *lfo_am)
{
    // OPL has two fixed-frequency LFOs, one for AM, one for PM

    // the AM LFO has 210*64 steps; at a nominal 50kHz output,
    // this equates to a period of 50000/(210*64) = 3.72Hz
    uint32_t am_counter = (*lfo_am_counter)++;
    if (am_counter >= 210 * 64 - 1)
        *lfo_am_counter = 0;

    // low 8 bits are fractional; compute at max depth and scale per-operator later
    int shift = 7;

    // AM value is the upper bits of the value, inverted across the midpoint
    // to produce a triangle
    *lfo_am = (uint8_t)(((am_counter < 105 * 64) ? am_counter : (210 * 64 + 63 - am_counter)) >> shift);

    // the PM LFO has 8192 steps, or a nominal period of 6.1Hz
    uint32_t pm_counter = (*lfo_pm_counter)++;

    // PM LFO is broken into 8 chunks, each lasting 1024 steps; the PM value
    // depends on the upper bits of FNUM, so this value is a fraction and
    // sign to apply to that value, as a 1.3 value
    static int8_t const pm_scale[8] = {8, 4, 0, -4, -8, -4, 0, 4};
    return pm_scale[(pm_counter >> 10) & 7];
}

//-------------------------------------------------
//  start_attack - start the attack phase; called
//  when a keyon happens or when an SSG-EG cycle
//  is complete and restarts
//-------------------------------------------------
static inline void start_attack(struct sgu_ch_state *self, uint8_t op, uint8_t op_reg[], uint32_t keycode)
{
    // don't change anything if already in attack state
    if (self->op[op].envelope_state == SGU_EG_ATTACK)
        return;
    self->op[op].envelope_state = SGU_EG_ATTACK;

    // if the attack rate >= 62 then immediately go to max attenuation
    if (compute_eg_rate(op_reg, keycode, SGU_EG_ATTACK) >= 62)
        self->op[op].envelope_attenuation = 0;
}

//-------------------------------------------------
//  start_release - start the release phase;
//  called when a keyoff happens
//-------------------------------------------------
static inline void start_release(struct sgu_ch_state *self, uint8_t op)
{
    // don't change anything if already in release state
    if (self->op[op].envelope_state >= SGU_EG_RELEASE)
        return;
    self->op[op].envelope_state = SGU_EG_RELEASE;
}

static inline void phase_reset(struct sgu_ch_state *self, uint8_t ch, uint8_t op)
{
    self->op[op].phase = 0;
    OP_FLAG_CLR(self->op_flags, OP_FLAGS_PHASE_WRAP, op);
    // initialize per-operator noise LFSR with unique seed per channel/operator
    self->op[op].lfsr_state = 0x1FFFFF ^ ((uint32_t)(ch * SGU_OP_PER_CH + op) << 8);
}

//-------------------------------------------------
//  reset - reset the channel state
//-------------------------------------------------
static void fm_channel_reset(struct sgu_ch_state *self, uint8_t ch)
{
    // reset our data
    self->op_flags = 0; // clear all packed booleans
    for (uint8_t op = 0; op < SGU_OP_PER_CH; op++)
    {
        phase_reset(self, ch, op);
        self->op[op].envelope_attenuation = 0x3ff;
        self->op[op].envelope_state = SGU_EG_RELEASE;
        self->op[op].eg_delay_counter = 0;
    }
}

//-------------------------------------------------
//  clock_envelope - clock the envelope state
//  according to the given count
//-------------------------------------------------
static inline void clock_envelope(struct sgu_ch_state *self, uint8_t op, uint8_t op_reg[], uint32_t keycode, uint32_t env_counter)
{
    struct sgu_op_state *os = &self->op[op];

    // handle attack->decay transitions
    if (os->envelope_state == SGU_EG_ATTACK && os->envelope_attenuation == 0)
        os->envelope_state = SGU_EG_DECAY;

    // handle decay->sustain transitions; it is important to do this immediately
    // after the attack->decay transition above in the event that the sustain level
    // is set to 0 (in which case we will skip right to sustain without doing any
    // decay); as an example where this can be heard, check the cymbals sound
    // in channel 0 of shinobi's test mode sound #5
    if (os->envelope_state == SGU_EG_DECAY && os->envelope_attenuation >= compute_eg_sustain(op_reg))
        os->envelope_state = SGU_EG_SUSTAIN;

    // compute the 6-bit rate value for the current envelope state
    uint32_t rate = compute_eg_rate(op_reg, keycode, os->envelope_state);

    // compute the rate shift value; this is the shift needed to
    // apply to the env_counter such that it becomes a 5.11 fixed
    // point number
    uint32_t rate_shift = rate >> 2;
    env_counter <<= rate_shift;

    // see if the fractional part is 0; if not, it's not time to clock
    if ((env_counter & 0x7FF) != 0)
        return;

    // determine the increment based on the non-fractional part of env_counter
    uint32_t relevant_bits = (env_counter >> ((rate_shift <= 11) ? 11 : rate_shift)) & 7;
    uint32_t increment = attenuation_increment(rate, relevant_bits);

    // attack is the only one that increases
    if (os->envelope_state == SGU_EG_ATTACK)
    {
        // glitch means that attack rates of 62/63 don't increment if
        // changed after the initial key on (where they are handled
        // specially); nukeykt confirms this happens on OPM, OPN, OPL/OPLL
        // at least so assuming it is true for everyone
        if (rate < 62)
            os->envelope_attenuation += (~os->envelope_attenuation * increment) >> 4;
    }
    // all other cases are similar
    else
    {
        os->envelope_attenuation += increment;

        // clamp the final attenuation
        if (os->envelope_attenuation >= 0x400)
            os->envelope_attenuation = 0x3ff;
    }
}

//-------------------------------------------------
//  clock_phase - clock the 10.10 phase value; the
//  OPN version of the logic has been verified
//  against the Nuked phase generator
//-------------------------------------------------
static inline void clock_phase(struct sgu_ch_state *self, uint8_t op, uint8_t op_reg[], uint32_t ch_keycode, uint32_t ratio_base_step)
{
    uint32_t phase_step;
    const uint8_t base = SGU_OP0_MUL(op_reg[0]);
    const uint8_t scale = SGU_OP4_DT(op_reg[4]);
    if (SGU_OP5_FIX(op_reg[5]))
    {
        // fixed frequency mode: 8..32640
        uint16_t freq16 = (uint16_t)(compute_fixed_base_freq16(base) << scale);
        phase_step = sgu_phase_step_freq16(freq16, 0);
    }
    else
    {
        // normal mode: compute phase step from channel frequency
        phase_step = sgu_compute_phase_step_from_base(ratio_base_step,
                                                      ch_keycode,
                                                      compute_multiplier(base),
                                                      scale);
    }

    // finally apply the step to the current phase value
    self->op[op].phase += phase_step;
}

//-------------------------------------------------
//  attenuation_to_volume - given a 5.8 fixed point
//  logarithmic attenuation value, return a 13-bit
//  linear volume
//-------------------------------------------------
static inline uint32_t attenuation_to_volume(uint32_t input)
{
    // the values here are 10-bit mantissas with an implied leading bit
    // this matches the internal format of the OPN chip, extracted from the die

    // as a nod to performance, the implicit 0x400 bit is pre-incorporated, and
    // the values are left-shifted by 2 so that a simple right shift is all that
    // is needed; also the order is reversed to save a NOT on the input
#define X(a) (((a) | 0x400) << 2)
    static uint16_t const s_power_table[256] = {
        X(0x3fa), X(0x3f5), X(0x3ef), X(0x3ea), X(0x3e4), X(0x3df), X(0x3da), X(0x3d4),
        X(0x3cf), X(0x3c9), X(0x3c4), X(0x3bf), X(0x3b9), X(0x3b4), X(0x3ae), X(0x3a9),
        X(0x3a4), X(0x39f), X(0x399), X(0x394), X(0x38f), X(0x38a), X(0x384), X(0x37f),
        X(0x37a), X(0x375), X(0x370), X(0x36a), X(0x365), X(0x360), X(0x35b), X(0x356),
        X(0x351), X(0x34c), X(0x347), X(0x342), X(0x33d), X(0x338), X(0x333), X(0x32e),
        X(0x329), X(0x324), X(0x31f), X(0x31a), X(0x315), X(0x310), X(0x30b), X(0x306),
        X(0x302), X(0x2fd), X(0x2f8), X(0x2f3), X(0x2ee), X(0x2e9), X(0x2e5), X(0x2e0),
        X(0x2db), X(0x2d6), X(0x2d2), X(0x2cd), X(0x2c8), X(0x2c4), X(0x2bf), X(0x2ba),
        X(0x2b5), X(0x2b1), X(0x2ac), X(0x2a8), X(0x2a3), X(0x29e), X(0x29a), X(0x295),
        X(0x291), X(0x28c), X(0x288), X(0x283), X(0x27f), X(0x27a), X(0x276), X(0x271),
        X(0x26d), X(0x268), X(0x264), X(0x25f), X(0x25b), X(0x257), X(0x252), X(0x24e),
        X(0x249), X(0x245), X(0x241), X(0x23c), X(0x238), X(0x234), X(0x230), X(0x22b),
        X(0x227), X(0x223), X(0x21e), X(0x21a), X(0x216), X(0x212), X(0x20e), X(0x209),
        X(0x205), X(0x201), X(0x1fd), X(0x1f9), X(0x1f5), X(0x1f0), X(0x1ec), X(0x1e8),
        X(0x1e4), X(0x1e0), X(0x1dc), X(0x1d8), X(0x1d4), X(0x1d0), X(0x1cc), X(0x1c8),
        X(0x1c4), X(0x1c0), X(0x1bc), X(0x1b8), X(0x1b4), X(0x1b0), X(0x1ac), X(0x1a8),
        X(0x1a4), X(0x1a0), X(0x19c), X(0x199), X(0x195), X(0x191), X(0x18d), X(0x189),
        X(0x185), X(0x181), X(0x17e), X(0x17a), X(0x176), X(0x172), X(0x16f), X(0x16b),
        X(0x167), X(0x163), X(0x160), X(0x15c), X(0x158), X(0x154), X(0x151), X(0x14d),
        X(0x149), X(0x146), X(0x142), X(0x13e), X(0x13b), X(0x137), X(0x134), X(0x130),
        X(0x12c), X(0x129), X(0x125), X(0x122), X(0x11e), X(0x11b), X(0x117), X(0x114),
        X(0x110), X(0x10c), X(0x109), X(0x106), X(0x102), X(0x0ff), X(0x0fb), X(0x0f8),
        X(0x0f4), X(0x0f1), X(0x0ed), X(0x0ea), X(0x0e7), X(0x0e3), X(0x0e0), X(0x0dc),
        X(0x0d9), X(0x0d6), X(0x0d2), X(0x0cf), X(0x0cc), X(0x0c8), X(0x0c5), X(0x0c2),
        X(0x0be), X(0x0bb), X(0x0b8), X(0x0b5), X(0x0b1), X(0x0ae), X(0x0ab), X(0x0a8),
        X(0x0a4), X(0x0a1), X(0x09e), X(0x09b), X(0x098), X(0x094), X(0x091), X(0x08e),
        X(0x08b), X(0x088), X(0x085), X(0x082), X(0x07e), X(0x07b), X(0x078), X(0x075),
        X(0x072), X(0x06f), X(0x06c), X(0x069), X(0x066), X(0x063), X(0x060), X(0x05d),
        X(0x05a), X(0x057), X(0x054), X(0x051), X(0x04e), X(0x04b), X(0x048), X(0x045),
        X(0x042), X(0x03f), X(0x03c), X(0x039), X(0x036), X(0x033), X(0x030), X(0x02d),
        X(0x02a), X(0x028), X(0x025), X(0x022), X(0x01f), X(0x01c), X(0x019), X(0x016),
        X(0x014), X(0x011), X(0x00e), X(0x00b), X(0x008), X(0x006), X(0x003), X(0x000)};
#undef X

    // look up the fractional part, then shift by the whole
    return s_power_table[input & 0xff] >> (input >> 8);
}

// -----------------------------------------------------------------------------
// Generate one stereo sample (l,r) for the whole CHNS-channel chip.
// Order per channel:
//   1) clock key state (attack/release)
//   2) clock envelope (ADSR)
//   3) clock phase for each operator (with detune, multiplier, LFO)
//   4) generate FM waveform (sine/tri/saw/pulse/noise) with phase modulation
//      - optional ring modulation per operator
//      - apply envelope attenuation, LFO AM, total level, key scaling
//      - sum operators to channel via OUT levels
//   5) optional channel-level ring modulation (multiply by next channel)
//   6) apply channel volume scaling
//   7) optional resonant SVF (LP/HP/BP selection via flags0 bits 5-7)
//   8) pan -> outL/outR (using pan gain LUTs)
//   9) apply sweeps (vol/freq/cutoff) for next samples
//  10) apply one-shot phase reset / timer sync
// Finally: sum all outL/outR into output with global HPF.
// -----------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// Phase 1: Global per-sample setup (LFO, envelope counters).
// Call once per sample before SGU_NextSample_Channels.
// ---------------------------------------------------------------------------
void __attribute__((optimize("Ofast"))) SGU_NextSample_Setup(struct SGU *restrict sgu)
{
    sgu->sample_counter += 1;
    // YMFM-style envelope counter with 2-bit subcounter
    if (EG_CLOCK_DIVIDER == 1)
        sgu->envelope_counter += 4;
    else if (((++sgu->envelope_counter) & 0b11) == EG_CLOCK_DIVIDER)
        sgu->envelope_counter += 4 - EG_CLOCK_DIVIDER;

    // clock the global LFO (once per sample)
    sgu->cached_lfo_raw_pm = clock_lfo(
        &sgu->lfo_am_counter,
        &sgu->lfo_pm_counter,
        &sgu->lfo_am);
    sgu->cached_env_tick = ((sgu->envelope_counter & 3u) == 0u);
    sgu->cached_env_counter_tick = sgu->envelope_counter >> 2;
}

// ---------------------------------------------------------------------------
// Phase 2: Process channels [ch_start, ch_end).
// Can be called from multiple cores with non-overlapping ranges.
// Accumulates partial stereo sums into *l, *r (caller initializes to 0).
// ---------------------------------------------------------------------------
void __attribute__((optimize("Ofast"))) SGU_NextSample_Channels(
    struct SGU *restrict sgu, unsigned ch_start, unsigned ch_end,
    int32_t *restrict l, int32_t *restrict r)
{
    int32_t L = 0;
    int32_t R = 0;

    const int32_t lfo_raw_pm = sgu->cached_lfo_raw_pm;
    const bool env_tick = sgu->cached_env_tick;
    const uint32_t env_counter_tick = sgu->cached_env_counter_tick;

    for (unsigned ch = ch_start; ch < ch_end; ch++)
    {
        struct SGU_CH *restrict ch_reg = &sgu->chan[ch];
        struct sgu_ch_state *restrict ch_state = &sgu->m_channel[ch];
        const uint16_t ch_freq = ch_reg->freq;
        const uint8_t ch_flags0 = ch_reg->flags0;
        const bool key_live = (ch_flags0 & SGU1_FLAGS0_CTL_GATE) != 0;

        int32_t ch_sample = 0;

        if (ch_flags0 & SGU1_FLAGS0_PCM_MASK) // PCM mode
        {
            // Signed 8-bit PCM sample scaled to match FM operator output (~14-bit range).
            ch_sample = (int16_t)sgu->pcm[ch_reg->pcmpos] << 6;

            // PCM phase accumulator. When it crosses 0x8000, advance sample position by 1.
            sgu->pcm_phase_accum[ch] += minval(ch_reg->freq, 0x8000);

            if (sgu->pcm_phase_accum[ch] >= 0x8000)
            {
                sgu->pcm_phase_accum[ch] -= 0x8000;

                // Advance sample pointer with boundary and optional looping.
                if (ch_reg->pcmpos < ch_reg->pcmbnd)
                {
                    ch_reg->pcmpos++;

                    // If we hit the boundary exactly, loop if enabled.
                    if (ch_reg->pcmpos == ch_reg->pcmbnd)
                    {
                        if (ch_reg->flags1 & SGU1_FLAGS1_PCM_LOOP)
                            ch_reg->pcmpos = ch_reg->pcmrst;
                    }

                    // Wrap to PCM RAM size (power-of-2 ring buffer).
                    ch_reg->pcmpos &= (SGU_PCM_RAM_SIZE - 1);
                }
                else if (ch_reg->flags1 & SGU1_FLAGS1_PCM_LOOP)
                {
                    // If already at/over boundary and looping, force restart.
                    ch_reg->pcmpos = ch_reg->pcmrst;
                }
            }
        }
        else
        {
            uint32_t ch_keycode, block, fnum_4msb;
            freq16_decode(ch_freq, &ch_keycode, &block, &fnum_4msb);
            uint32_t ch_ksl_atten = opl_key_scale_atten(block, fnum_4msb);
            const uint32_t phase_step_pm0 = sgu_phase_step_from_freq_clamped((int32_t)ch_freq);
            const int32_t pm_mul = (int32_t)ch_freq * lfo_raw_pm;
            const uint32_t phase_step_pm_half = sgu_phase_step_from_freq_clamped((int32_t)ch_freq + (pm_mul >> 11));
            const uint32_t phase_step_pm_full = sgu_phase_step_from_freq_clamped((int32_t)ch_freq + (pm_mul >> 10));

            // run channel operators
            for (uint8_t op = 0; op < SGU_OP_PER_CH; op++)
            {
                // Cache operator registers into stack-local array to break uint8_t* aliasing.
                // With -Ofast this compiles to a single LDRD (double-word load, 2 cycles)
                // instead of up to 8 separate LDRB. Stack-locals can't alias ch_state writes,
                // so the compiler won't re-read registers after every state store.
                uint8_t op_reg[8];
                memcpy(op_reg, &ch_reg->op[op], 8);

                struct sgu_op_state *os = &ch_state->op[op];

                // clock the key state (with optional per-operator delay)
                if (OP_FLAG_GET(ch_state->op_flags, OP_FLAGS_EG_DELAY, op) && os->eg_delay_counter <= INT16_MAX)
                    os->eg_delay_counter++;

                if (key_live && !OP_FLAG_GET(ch_state->op_flags, OP_FLAGS_KEYON_GATE, op))
                {
                    OP_FLAG_SET(ch_state->op_flags, OP_FLAGS_EG_DELAY, op);
                    os->eg_delay_counter = 0;
                }
                else if (!key_live)
                {
                    OP_FLAG_CLR(ch_state->op_flags, OP_FLAGS_EG_DELAY, op);
                    os->eg_delay_counter = 0;
                }

                const unsigned delay = SGU_OP5_DELAY(op_reg[5]);
                const unsigned delay_target = delay ? (256u << delay) : 0;
                bool keystate = key_live
                                && (!OP_FLAG_GET(ch_state->op_flags, OP_FLAGS_EG_DELAY, op)
                                    || os->eg_delay_counter >= delay_target);
                if (key_live)
                    OP_FLAG_SET(ch_state->op_flags, OP_FLAGS_KEYON_GATE, op);
                else
                    OP_FLAG_CLR(ch_state->op_flags, OP_FLAGS_KEYON_GATE, op);

                // has the key changed?
                if ((keystate ^ OP_FLAG_GET(ch_state->op_flags, OP_FLAGS_KEY_STATE, op)) != 0)
                {
                    if (keystate)
                        OP_FLAG_SET(ch_state->op_flags, OP_FLAGS_KEY_STATE, op);
                    else
                        OP_FLAG_CLR(ch_state->op_flags, OP_FLAGS_KEY_STATE, op);

                    // if the key has turned on, start the attack
                    if (keystate != 0)
                        start_attack(ch_state, op, op_reg, ch_keycode);
                    // otherwise, start the release
                    else
                        start_release(ch_state, op);
                }

                // save previous phase for noise boundary detection
                const uint32_t phase_before = os->phase;

                // clock the envelope (only on envelope ticks)
                if (env_tick)
                    clock_envelope(ch_state, op, op_reg, ch_keycode, env_counter_tick);

                // handle phase reset due to SYNC (previous op wrap from last sample)
                const unsigned prev_op = op ? (op - 1u) : (SGU_OP_PER_CH - 1u);
                const bool sync_reset = (SGU_OP6_SYNC(op_reg[6]) && OP_FLAG_GET(ch_state->op_flags, OP_FLAGS_PHASE_WRAP, prev_op));
                if (sync_reset)
                {
                    phase_reset(ch_state, ch, op);
                }
                else
                {
                    // clock the phase (apply per-operator PM depth)
                    uint32_t ratio_base_step = phase_step_pm0;
                    if (SGU_OP0_VIB(op_reg[0]))
                        ratio_base_step = SGU_OP6_VIBD(op_reg[6]) ? phase_step_pm_full : phase_step_pm_half;
                    clock_phase(ch_state, op, op_reg, ch_keycode, ratio_base_step);
                }

                // record wrap for next operator's SYNC
                if (os->phase < phase_before)
                    OP_FLAG_SET(ch_state->op_flags, OP_FLAGS_PHASE_WRAP, op);
                else
                    OP_FLAG_CLR(ch_state->op_flags, OP_FLAGS_PHASE_WRAP, op);

                // compute LFSR state 6x per operator cycle (only for noise waveforms)
                unsigned wave = SGU_OP7_WAVE(op_reg[7]);
                if (wave == SGU_WAVE_NOISE || wave == SGU_WAVE_PERIODIC_NOISE)
                {
                    // NOTE: This is a 6-bit LFSR, thus it takes 6 shift cycles to complete a full period.
                    // So if we take 6 times per cycle, the LFSR repeating output frequency matches the
                    // operator frequency.
                    if (((os->phase >> 8) * 6 >> 24) != ((phase_before >> 8) * 6 >> 24))
                    {
                        uint32_t *lfsr = &os->lfsr_state;
                        if (wave == SGU_WAVE_NOISE)
                        {
                            *lfsr = (*lfsr >> 1 | (((*lfsr) ^ (*lfsr >> 2) ^ (*lfsr >> 3) ^ (*lfsr >> 5)) & 1) << 31);
                        }
                        else
                        {
                            switch (SGU_OP5_WPAR(op_reg[5]) & 3)
                            {
                            case SGU_LFSR_TAP34:
                                *lfsr = (*lfsr >> 1 | (((*lfsr >> 3) ^ (*lfsr >> 4)) & 1) << 5);
                                break;
                            case SGU_LFSR_TAP23:
                                *lfsr = (*lfsr >> 1 | (((*lfsr >> 2) ^ (*lfsr >> 3)) & 1) << 5);
                                break;
                            case SGU_LFSR_TAP023:
                                *lfsr = (*lfsr >> 1 | (((*lfsr) ^ (*lfsr >> 2) ^ (*lfsr >> 3)) & 1) << 5);
                                break;
                            case SGU_LFSR_TAP0235:
                                *lfsr = (*lfsr >> 1 | (((*lfsr) ^ (*lfsr >> 2) ^ (*lfsr >> 3) ^ (*lfsr >> 5)) & 1) << 5);
                                break;
                            }
                            if ((*lfsr & 0x3F) == 0 || (~*lfsr & 0x3F) == 0)
                            {
                                *lfsr = 0x2A;
                            }
                        }
                    }
                }

                // generate the FM sample for this channel

                //-------------------------------------------------
                // compute the 14-bit signed amplitude of this operator,
                // given a phase modulation and an AM LFO offset
                // the low 10 bits of phase represents a full 2*PI period over
                // the full sin wave
                //-------------------------------------------------
                int32_t val = 0;

                // skip work if the envelope is effectively off
                if (os->envelope_attenuation < EG_QUIET)
                {
                    // get the phase modulation input
                    const unsigned mod = SGU_OP6_MOD(op_reg[6]);
                    // Feedback: >> 1 to average two samples, >> 1 to prevent runaway (ESFM design).
                    const int16_t in_val = op ? ch_state->op[op - 1].value
                                              : (ch_state->op0_fb + ch_state->op[0].value) >> (1 + 1);
                    // ESFM uses 13-bit samples and >> (7 - mod); SGU uses 14-bit, thus >> (8 - mod).
                    const int16_t p_mod = (mod == 0)
                                              ? 0
                                              : (in_val >> (8 - mod));

                    unsigned wpar = SGU_OP5_WPAR(op_reg[5]);

                    // scale down to 10-bit phase for waveform lookup
                    // Round instead of truncate to reduce phase drift artifacts
                    int phase = (((os->phase + (1 << 21)) >> 22) + p_mod) & 0x3FF;

                    int sample = 0;
                    bool need_blep = false;

                    switch (wave)
                    {
                    case SGU_WAVE_SINE:
                    case SGU_WAVE_TRIANGLE:
                    case SGU_WAVE_SAWTOOTH:
                    {
                        if (wpar & SGU_WPAR_QUANT)
                        {
                            // WPAR[2:0] quantizes phase by zeroing LSBs
                            phase &= ~((1 << ((wpar & 0x07) + 1)) - 1);
                        }

                        switch (wave)
                        {
                        case SGU_WAVE_SINE:
                            sample = sine_lut[phase];
                            break;
                        case SGU_WAVE_TRIANGLE:
                            sample = triangle_lut[phase];
                            break;
                        case SGU_WAVE_SAWTOOTH:
                            sample = sawtooth_lut[phase];
                            break;
                        }

                        // Apply OPL-style wave modifiers (SGU_WPAR_HALF, SGU_WPAR_ABS)
                        if (wpar < SGU_WPAR_QUANT)
                        {
                            const bool high = (phase >> 3) >= ch_reg->duty;
                            switch (wpar)
                            {
                            case SGU_WPAR_HALF_L:
                                sample = high ? sample : 0;
                                break;
                            case SGU_WPAR_HALF_H:
                                sample = high ? 0 : sample;
                                break;
                            case SGU_WPAR_ABS_L:
                                sample = high ? sample : (int16_t)-sample;
                                break;
                            case SGU_WPAR_ABS_H:
                                sample = high ? (int16_t)-sample : sample;
                                break;
                            }
                        }

                        // BLEP for hard edge: detect dramatic sample change (|delta| > half range)
                        int32_t delta = (int32_t)sample - (int32_t)os->blep_prev_sample;
                        need_blep = (delta > INT16_MAX || delta < INT16_MIN);
                    }
                    break;
                    case SGU_WAVE_PULSE:
                    {
                        // compare phase-derived 7-bit ramp against duty (0..127).
                        // WPAR: 0 => channel duty, 1..15 => fixed pulse width (x/16th of period)
                        const uint8_t duty = wpar ? (uint8_t)((uint8_t)wpar << 3) : ch_reg->duty;
                        sample = ((phase >> 3) >= duty) ? INT16_MAX : INT16_MIN;

                        // Detect edge by comparing with previous RAW sample (not enveloped value)
                        need_blep = sample != os->blep_prev_sample;
                    }
                    break;
                    case SGU_WAVE_NOISE:
                    case SGU_WAVE_PERIODIC_NOISE:
                        // Bipolar output spanning full INT16 range
                        sample = (os->lfsr_state & 1) ? INT16_MAX : INT16_MIN;
                        break;
                    case SGU_WAVE_RESERVED6:
                        // Reserved - outputs silence
                        sample = 0;
                        break;
                    case SGU_WAVE_SAMPLE:
                    {
                        // Sample-as-waveform mode: read 8-bit PCM sample from memory
                        // Uses channel's pcmrst register as the base address for a 1024-sample waveform
                        // Phase (0-1023) indexes into the sample region, looping naturally via phase wraparound
                        uint16_t sample_addr = (ch_reg->pcmrst + phase) & (SGU_PCM_RAM_SIZE - 1);
                        // Scale 8-bit signed sample to 16-bit to match other waveforms
                        // (attenuation to 14-bit happens later at the envelope processing stage)
                        sample = (int16_t)((int16_t)sgu->pcm[sample_addr] << 8);
                    }
                    break;
                    }

                    if (need_blep)
                    {
                        os->blep = 1; // Only 1 sample needs interpolation
                        // Capture fractional phase (22 bits) as 16-bit for sub-sample position
                        // High frac = crossed early in sample = more "new" value
                        // Low frac = crossed late in sample = more "old" value
                        os->blep_frac = (uint16_t)(os->phase >> 6);
                    }
                    // store current sample for next cycle's BLEP detection
                    os->blep_prev_sample = (int16_t)sample;

                    // Apply ring modulation if RING bit set (R6[4])
                    // 1-bit ring mod: flip sign based on previous operator's output
                    // For op 0, uses last operator (op 3) for ring mod source
                    // Creates sum/difference sidebands for metallic/bell-like timbres
                    if (SGU_OP6_RING(op_reg[6]))
                    {
                        const int16_t ring_src = (op > 0) ? ch_state->op[op - 1].value
                                                          : ch_state->op[SGU_OP_PER_CH - 1].value;
                        if (ring_src < 0)
                            sample = -sample;
                    }

                    // compute the effective attenuation of the envelope
                    uint32_t env_att = os->envelope_attenuation;

                    // add in LFO AM modulation (apply per-operator AM depth)
                    if (SGU_OP0_TRM(op_reg[0]))
                    {
                        uint32_t am_offset = sgu->lfo_am;
                        if (!SGU_OP6_TRMD(op_reg[6]))
                            am_offset >>= 2;
                        env_att += am_offset;
                    }

                    // add in total level, scaled by 8
                    env_att += SGU_OP16_TL(op_reg[1], op_reg[6]) << 3;

                    // add key scale level
                    const uint32_t ksl = SGU_OP1_KSL(op_reg[1]);
                    if (ksl)
                        env_att += ch_ksl_atten << ksl;

                    // clamp to max (USAT: single-cycle unsigned saturate)
                    env_att = (uint32_t)__builtin_arm_usat((int32_t)env_att, 10);

                    // the attenuation from the envelope generator as a 4.6 value, shifted up to 4.8
                    // env_att <<= 2; shifting handled by the lookup table generator.
                    const int32_t amp = sample * (int32_t)env_gain_lut[env_att];
                    // scale back to int16 range: divide by 2^13 (Q13 gain)
                    // and scale down 2 more to Q14 output range
                    // add 0.5 LSB for rounding before shifting (optional)
                    val = (amp + (1 << 14)) >> 15;
                }

                if (op == 0)
                {
                    // cache operator 0 output for delayed feedback on next sample
                    ch_state->op0_fb = ch_state->op[0].value;
                }

                const int32_t out_delta = val - os->value;
                // Store raw value for modulation (NO anti-aliasing - modulators need precise edges)
                os->value = (int16_t)val;

                // BLEP anti-aliasing correction for hard edges
                if (os->blep > 0)
                {
                    val -= ((int32_t)out_delta * (65536 - os->blep_frac)) >> 16;
                    os->blep--;
                }

                const unsigned out = SGU_OP7_OUT(op_reg[7]);
                if (out)
                    ch_sample += ((int16_t)val) >> (7 - out);
            }
        }

        // ------------------------------------------------------------
        // Store raw FM output for ring modulation (used by next channel)
        // ------------------------------------------------------------
        int16_t raw_sample = (int16_t)__builtin_arm_ssat(ch_sample, 16);

        // ------------------------------------------------------------
        // Channel-level ring modulation (amplitude modulation style)
        // Reads src[ch+1] directly; in dual-core mode cross-boundary reads
        // may see previous or current frame's value (1-sample jitter is OK).
        // ------------------------------------------------------------
        if (ch_flags0 & SGU1_FLAGS0_CTL_RING_MOD)
        {
            const int16_t ring_src = sgu->src[(ch + 1 < SGU_CHNS) ? ch + 1 : 0];
            ch_sample = ((int32_t)ch_sample * ring_src) >> 15;
        }

        // Store this channel's raw sample for ring modulation
        sgu->src[ch] = raw_sample;

        // ------------------------------------------------------------
        // Apply channel volume scaling
        // ------------------------------------------------------------
        int32_t voice_sample = (ch_sample * ch_reg->vol) >> 7;

        // ------------------------------------------------------------
        // Resonant filter (state-variable filter)
        // flags0 bits 5..7 select which outputs to mix: LP/HP/BP.
        // ------------------------------------------------------------
        if (ch_flags0 & (SGU1_FLAGS0_CTL_NSLOW | SGU1_FLAGS0_CTL_NSHIGH | SGU1_FLAGS0_CTL_NSBAND))
        {
            const int32_t ff = (int32_t)ch_reg->cutoff * 3;

            const int32_t drive = 256 + (ch_reg->reson >> 1);
            voice_sample = svf_saturate((voice_sample * drive) >> 8);

            sgu->svf_low[ch] = svf_saturate(sgu->svf_low[ch] + ((ff * sgu->svf_band[ch]) >> 16));
            sgu->svf_high[ch] = voice_sample - sgu->svf_low[ch] - (((256 - ch_reg->reson) * sgu->svf_band[ch]) >> 8);
            sgu->svf_band[ch] = svf_saturate(((ff * sgu->svf_high[ch]) >> 16) + sgu->svf_band[ch]);

            voice_sample = ((ch_flags0 & SGU1_FLAGS0_CTL_NSLOW) ? sgu->svf_low[ch] : 0)
                           + ((ch_flags0 & SGU1_FLAGS0_CTL_NSHIGH) ? sgu->svf_high[ch] : 0)
                           + ((ch_flags0 & SGU1_FLAGS0_CTL_NSBAND) ? sgu->svf_band[ch] : 0);
        }

        // Store post-processed sample for debug/meters
        sgu->post[ch] = voice_sample;

        // Panning
        int32_t out_l = (voice_sample * pan_gain_lut_l[(uint8_t)ch_reg->pan]) >> 7;
        int32_t out_r = (voice_sample * pan_gain_lut_r[(uint8_t)ch_reg->pan]) >> 7;

        // Sweeps (affect parameters for future samples)
        // (flags1 bit5).
        // swvol.amt encoding:
        //   bit5 (0x20): direction (1=up, 0=down)
        //   bits0..4: step size
        //   bit6 (0x40): "wrap/loop" behavior
        //   bit7 (0x80): "bounce/alternate" behavior
        if ((ch_reg->flags1 & SGU1_FLAGS1_VOL_SWEEP) && ch_reg->swvol.speed)
        {
            if (--sgu->vol_sweep_countdown[ch] <= 0)
            {
                sgu->vol_sweep_countdown[ch] += ch_reg->swvol.speed;

                if (ch_reg->swvol.amt & 32) // up
                {
                    int v = ch_reg->vol + (ch_reg->swvol.amt & 31);
                    ch_reg->vol = (v > 127) ? 127 : (v < -128) ? -128
                                                               : (int8_t)v;

                    // If not wrapping, clamp at upper bound.
                    if (ch_reg->vol > (int8_t)ch_reg->swvol.bound && !(ch_reg->swvol.amt & 64))
                        ch_reg->vol = (int8_t)ch_reg->swvol.bound;

                    // Handle wrap/bounce on overflow sign bit.
                    if (ch_reg->vol & 0x80)
                    {
                        if (ch_reg->swvol.amt & 64) // wrap enabled
                        {
                            if (ch_reg->swvol.amt & 128) // bounce enabled
                            {
                                ch_reg->swvol.amt ^= 32;                    // flip direction
                                ch_reg->vol = (int8_t)(0xFF - ch_reg->vol); // reflect
                            }
                            else
                            {
                                ch_reg->vol &= ~0x80; // wrap into positive
                            }
                        }
                        else
                        {
                            ch_reg->vol = 0x7F; // clamp at upper bound
                        }
                    }
                }
                else // down
                {
                    int v = ch_reg->vol - (ch_reg->swvol.amt & 31);
                    ch_reg->vol = (v > 127) ? 127 : (v < -128) ? -128
                                                               : (int8_t)v;

                    if (ch_reg->vol & 0x80)
                    {
                        if (ch_reg->swvol.amt & 64) // wrap enabled
                        {
                            if (ch_reg->swvol.amt & 128) // bounce enabled
                            {
                                ch_reg->swvol.amt ^= 32; // flip direction
                                ch_reg->vol = (int8_t)(-ch_reg->vol);
                            }
                            else
                            {
                                ch_reg->vol &= ~0x80;
                            }
                        }
                        else
                        {
                            ch_reg->vol = 0x00; // clamp at 0
                        }
                    }

                    // If not wrapping, clamp at lower bound.
                    if (ch_reg->vol < (int8_t)ch_reg->swvol.bound && !(ch_reg->swvol.amt & 64))
                        ch_reg->vol = (int8_t)ch_reg->swvol.bound;
                }
            }
        }

        if ((ch_reg->flags1 & SGU1_FLAGS1_FREQ_SWEEP) && ch_reg->swfreq.speed)
        {
            if (--sgu->freq_sweep_countdown[ch] <= 0)
            {
                sgu->freq_sweep_countdown[ch] += ch_reg->swfreq.speed;

                if (ch_reg->swfreq.amt & 128) // up
                {
                    if (ch_reg->freq > (0xFFFF - (ch_reg->swfreq.amt & 127)))
                        ch_reg->freq = 0xFFFF;
                    else
                    {
                        // Multiply by (1.0 + amt/128).
                        ch_reg->freq = (uint16_t)((ch_reg->freq * (0x80 + (ch_reg->swfreq.amt & 127))) >> 7);

                        if ((ch_reg->freq >> 8) > ch_reg->swfreq.bound)
                            ch_reg->freq = (uint16_t)(ch_reg->swfreq.bound << 8);
                    }
                }
                else // down
                {
                    if (ch_reg->freq < (ch_reg->swfreq.amt & 127))
                        ch_reg->freq = 0;
                    else
                    {
                        // Multiply by (1.0 - amt/256).
                        ch_reg->freq = (uint16_t)((ch_reg->freq * (0xFF - (ch_reg->swfreq.amt & 127))) >> 8);

                        if ((ch_reg->freq >> 8) < ch_reg->swfreq.bound)
                            ch_reg->freq = (uint16_t)(ch_reg->swfreq.bound << 8);
                    }
                }
            }
        }

        if ((ch_reg->flags1 & SGU1_FLAGS1_CUT_SWEEP) && ch_reg->swcut.speed)
        {
            if (--sgu->cut_sweep_countdown[ch] <= 0)
            {
                sgu->cut_sweep_countdown[ch] += ch_reg->swcut.speed;

                if (ch_reg->swcut.amt & 128) // up
                {
                    if (ch_reg->cutoff > (0xFFFF - (ch_reg->swcut.amt & 127)))
                        ch_reg->cutoff = 0xFFFF;
                    else
                    {
                        ch_reg->cutoff += (ch_reg->swcut.amt & 127);

                        if ((ch_reg->cutoff >> 8) > ch_reg->swcut.bound)
                            ch_reg->cutoff = (uint16_t)(ch_reg->swcut.bound << 8);
                    }
                }
                else // down
                {
                    if (ch_reg->cutoff < (ch_reg->swcut.amt & 127))
                        ch_reg->cutoff = 0;
                    else
                    {
                        // Multiply by (1.0 - amt/2048).
                        ch_reg->cutoff = (uint16_t)(((2048 - (unsigned int)(ch_reg->swcut.amt & 127)) * (unsigned int)ch_reg->cutoff) >> 11);

                        if ((ch_reg->cutoff >> 8) < ch_reg->swcut.bound)
                            ch_reg->cutoff = (uint16_t)(ch_reg->swcut.bound << 8);
                    }
                }
            }
        }

        // Phase reset requests
        if (ch_reg->flags1 & SGU1_FLAGS1_PHASE_RESET)
        {
            for (uint8_t op = 0; op < SGU_OP_PER_CH; op++)
            {
                phase_reset(ch_state, ch, op);
            }
            sgu->phase_reset_countdown[ch] = ch_reg->restimer;
            ch_reg->flags1 &= ~SGU1_FLAGS1_PHASE_RESET;
        }

        if (ch_reg->flags1 & SGU1_FLAGS1_FILTER_PHASE_RESET)
        {
            sgu->svf_low[ch] = sgu->svf_high[ch] = sgu->svf_band[ch] = 0;
            ch_reg->flags1 &= ~SGU1_FLAGS1_FILTER_PHASE_RESET;
        }

        // Timer sync: periodic phase reset (flags1 bit3)
        if ((ch_reg->flags1 & SGU1_FLAGS1_TIMER_SYNC) && ch_reg->restimer)
        {
            if (--sgu->phase_reset_countdown[ch] <= 0)
            {
                sgu->phase_reset_countdown[ch] += ch_reg->restimer;
                for (uint8_t op = 0; op < SGU_OP_PER_CH; op++)
                {
                    phase_reset(ch_state, ch, op);
                }
            }
        }

        // Software mute: also clears filter state to avoid stale ringing when unmuted.
        if (sgu->muted[ch])
        {
            sgu->svf_low[ch] = sgu->svf_high[ch] = sgu->svf_band[ch] = 0;
            out_l = out_r = 0;
        }

        sgu->outL[ch] = out_l;
        sgu->outR[ch] = out_r;

        L += out_l;
        R += out_r;
    }

    *l = L;
    *r = R;
}

// ---------------------------------------------------------------------------
// Phase 3: DC-removal HPF and final output clamping.
// Call once per sample after merging all channel partial sums.
// ---------------------------------------------------------------------------
void __attribute__((optimize("Ofast"))) SGU_NextSample_Finalize(
    struct SGU *restrict sgu, int64_t L, int64_t R,
    int32_t *restrict l, int32_t *restrict r)
{
    // Leaky Integrator HPF to remove DC offset and create pulse droop
    int64_t diff_L = L - sgu->L_in;
    int64_t diff_R = R - sgu->R_in;

    // High Pass Filter: y[n] = alpha * (y[n-1] + x[n] - x[n-1])
    // We shift the diff to Q16 to match the state's fixed-point scale
    int64_t L_q16 = (SGU_ALPHA_RC_DECAY_Q16 * (sgu->L_q16 + (diff_L << 16))) >> 16;
    int64_t R_q16 = (SGU_ALPHA_RC_DECAY_Q16 * (sgu->R_q16 + (diff_R << 16))) >> 16;

    sgu->L_in = L;
    sgu->R_in = R;
    sgu->L_q16 = L_q16;
    sgu->R_q16 = R_q16;

    // 4. Convert back from Q16 and clamp to INT32
    // Use the values as-is since they are already centered by the filter
    int32_t final_L = (int32_t)(L_q16 >> 16);
    int32_t final_R = (int32_t)(R_q16 >> 16);
    *l = sgu->L = (int32_t)minval(INT32_MAX, maxval(INT32_MIN, final_L));
    *r = sgu->R = (int32_t)minval(INT32_MAX, maxval(INT32_MIN, final_R));
}

// ---------------------------------------------------------------------------
// Single-core wrapper: calls all 3 phases sequentially.
// ---------------------------------------------------------------------------
void __attribute__((optimize("Ofast"))) SGU_NextSample(struct SGU *restrict sgu, int32_t *restrict l, int32_t *restrict r)
{
    SGU_NextSample_Setup(sgu);
    int32_t L = 0, R = 0;
    SGU_NextSample_Channels(sgu, 0, SGU_CHNS, &L, &R);
    SGU_NextSample_Finalize(sgu, L, R, l, r);
}

void __attribute__((optimize("Ofast"))) SGU_Init(struct SGU *sgu, size_t sampleMemSize)
{
    (void)sampleMemSize;
    memset(sgu, 0, sizeof(struct SGU));

    /**
     * Compute lookup tables.
     * NOTE: these are shared among all SGU instances,
     * but with exactly same values, so we compute them per-instance for simplicity.
     * TODO: move to pre-computed static const tables.
     */
    for (int32_t i = 0; i < SGU_WAVEFORM_LENGTH; i++)
    {
        uint16_t t = (uint16_t)((i * UINT16_MAX) / (SGU_WAVEFORM_LENGTH - 1));
        sawtooth_lut[i] = (int16_t)(INT16_MIN + t);
    }

    for (int32_t i = 0; i < SGU_WAVEFORM_LENGTH >> 1; i++)
    {
        // Build positive half and mirror to negative half
        int16_t s = (int16_t)(sin(M_PI * (float)i / (float)((SGU_WAVEFORM_LENGTH >> 1) - 1)) * INT16_MAX);
        sine_lut[i] = s;
        sine_lut[i + (SGU_WAVEFORM_LENGTH >> 1)] = (int16_t)-s;
    }

    for (int32_t i = 0; i < SGU_WAVEFORM_LENGTH >> 2; i++)
    {
        int16_t t = (int16_t)((i * INT16_MAX) / ((SGU_WAVEFORM_LENGTH >> 2) - 1));
        triangle_lut[i] = t;
        triangle_lut[i + (SGU_WAVEFORM_LENGTH >> 2)] = INT16_MAX - t;
        triangle_lut[i + 2 * (SGU_WAVEFORM_LENGTH >> 2)] = (int16_t)-t;
        triangle_lut[i + 3 * (SGU_WAVEFORM_LENGTH >> 2)] = INT16_MIN + t;
    }

    for (int32_t i = 0; i < 0x400; i++)
    {
        // Precompute 0..1023 attenuation steps as linear Q13 gain.
        env_gain_lut[i] = (uint16_t)attenuation_to_volume((uint32_t)i << 2);
    }

    // Build pan gain lookup tables (same as su.c)
    // Start with "center pan": both gains 127
    for (size_t i = 0; i < 256; i++)
    {
        pan_gain_lut_l[i] = 127;
        pan_gain_lut_r[i] = 127;
    }
    // Pan shaping:
    // - For i=0..127: left gain decreases from 127->0 (panned right)
    // - For i=128..255: right gain increases from 0->126 (panned left)
    for (size_t i = 0; i < 128; i++)
    {
        pan_gain_lut_l[i] = (uint8_t)(127 - i);
        pan_gain_lut_r[128 + i] = (uint8_t)(i - 1);
    }
    pan_gain_lut_r[128] = 0;

#ifdef SGU_ON_MCU
    // there can be only one…
    sgu->pcm = pcm_mem;
#else
    sgu->pcm = malloc(SGU_PCM_RAM_SIZE);
#endif

    SGU_Reset(sgu);
}
void SGU_Reset(struct SGU *sgu)
{
    memset(sgu->chan, 0, sizeof(struct SGU_CH) * SGU_CHNS);

    sgu->sample_counter = 0;
    sgu->envelope_counter = 0;
    sgu->lfo_am_counter = 0;
    sgu->lfo_pm_counter = 0;
    sgu->lfo_am = 0;

    for (uint8_t ch = 0; ch < SGU_CHNS; ch++)
    {
        fm_channel_reset(&sgu->m_channel[ch], ch);

        // Reset SID-like channel processing state
        sgu->svf_low[ch] = 0;
        sgu->svf_high[ch] = 0;
        sgu->svf_band[ch] = 0;

        // Initialize sweep timers so first decrement lands at 0
        sgu->vol_sweep_countdown[ch] = 1;
        sgu->freq_sweep_countdown[ch] = 1;
        sgu->cut_sweep_countdown[ch] = 1;

        sgu->phase_reset_countdown[ch] = 0;
        sgu->pcm_phase_accum[ch] = 0;

        sgu->src[ch] = 0;
        sgu->post[ch] = 0;
        sgu->outL[ch] = 0;
        sgu->outR[ch] = 0;
    }
}

void SGU_Write(struct SGU *sgu, uint16_t addr13, uint8_t data)
{
    ((uint8_t *)sgu->chan)[addr13] = data;
}

static_assert(sizeof(struct SGU_CH) == (SGU_OP_PER_CH * SGU_OP_REGS + SGU_CH_REGS), "SGU channel size mismatch");
static_assert(SGU_REGS_PER_CH == (SGU_OP_PER_CH * SGU_OP_REGS + SGU_CH_REGS), "SGU regs size mismatch");

int32_t SGU_GetSample(struct SGU *sgu, uint8_t ch)
{
    // Return the post-processed mono sample (after volume/filter, before pan)
    return sgu->post[ch];
}
