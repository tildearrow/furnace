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

#define minval(a, b) (((a) < (b)) ? (a) : (b))
#define maxval(a, b) (((a) > (b)) ? (a) : (b))

#define F16_QMASK(qb) (uint16_t)(0x3FFu & ~((1u << (qb)) - 1u))

//-------------------------------------------------
//  clamp - clamp between the minimum and maximum
//  values provided
//-------------------------------------------------

static inline int32_t clamp(int32_t value, int32_t minval, int32_t maxval)
{
    if (value < minval)
        return minval;
    if (value > maxval)
        return maxval;
    return value;
}

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
//  bitfield - extract a bitfield from the given
//  value, starting at bit 'start' for a length of
//  'length' bits
//-------------------------------------------------
static inline uint32_t bitfield(uint32_t value, int start, int length /*= 1*/)
{
    return (value >> start) & ((1 << length) - 1);
}

// return a bitfield extracted from a byte
static inline uint32_t byte(uint8_t data[], uint32_t offset, uint32_t start, uint32_t count, uint32_t extra_offset /*= 0*/)
{
    return bitfield(data[offset + extra_offset], start, count);
}

// return a bitfield extracted from a pair of bytes, MSBs listed first
static inline uint32_t word(uint8_t data[], uint32_t offset1, uint32_t start1, uint32_t count1, uint32_t offset2, uint32_t start2, uint32_t count2, uint32_t extra_offset /*= 0*/)
{
    return (byte(data, offset1, start1, count1, extra_offset) << count2)
           | byte(data, offset2, start2, count2, extra_offset);
}

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
    return maxval(0, result);
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
    return bitfield(detune, 2, 1) ? -result : result;
}

// Cheap 0..31 keycode from freq16 for detune table
static inline uint32_t keycode_from_freq16_32(uint16_t freq16)
{
    if (freq16 < 0x0100)
        return 0;
    uint32_t msb = 31u - sgu_clz32((uint32_t)freq16); // 8..15
    uint32_t block = msb - 8u;                            // 0..7
    uint32_t mant2 = (freq16 >> (msb - 2)) & 3u;          // 0..3
    return (block << 2) | mant2;                          // 0..31
}

// SGU uses SID semantics with Fclk = 1,000,000
// with sample rate 48,000.
static inline uint32_t sgu_phase_step_freq16(uint16_t freq16, int32_t lfo_raw_pm)
{
    // shift=10 => depth=1 gives ~±13.5 cents peak on the max PM step
    const int32_t delta = ((int32_t)freq16 * lfo_raw_pm) >> 10;

    int32_t f = (int32_t)freq16 + delta;
    if (f < 0)
        f = 0;
    if (f > 65535)
        f = 65535;

    // Fclk=1_000_000, Fs=48_000 => factor = 16000/3
    uint32_t phase_step = f * 16000u;    // <= ~1.05e9 fits in 32-bit unsigned
    phase_step = (phase_step + 1u) / 3u; // rounded-ish; or use (x + 1) / 3

    // If you want true round-to-nearest: (x + 1) / 3 for this scale is fine;
    // stricter: (x + 3/2) / 3 -> (x + 1) / 3 since 3/2 truncated.

    return phase_step;
}

//-------------------------------------------------
//  compute_eg_sustain - compute the sustain level
//  shifted up to envelope values
//-------------------------------------------------
static inline uint32_t compute_eg_sustain(uint8_t op_data[])
{
    // 4-bit sustain level, but 15 means 31 so effectively 5 bits
    uint32_t eg_sustain = SGU_OP3_SL(op_data[3]);
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
    // multiplier value, as an x.1 value (0 means 0.5)
    // replace the low bit with a table lookup to give 0,1,2,3,4,5,6,7,8,9,10,10,12,12,15,15
    uint32_t multiplier = ((mul & 0xe)
                           | bitfield(0xc2aa, mul, 1))
                          * 2;
    if (multiplier == 0)
        multiplier = 1;
    return multiplier;
}

//-------------------------------------------------
//  compute_phase_step - compute the phase step
//-------------------------------------------------
static inline uint32_t sgu_compute_phase_step(uint32_t freq16, uint32_t multiplier, int32_t lfo_raw_pm, uint8_t detune3)
{
    uint32_t phase_step = sgu_phase_step_freq16((uint16_t)freq16, lfo_raw_pm);

    // apply detune based on the keycode
    // DT1 -> small signed adjustment, scaled into step32 units
    // OPM applies detune BEFORE multiplier: result = (base + detune) * multiplier
    // This way detune effect scales with the multiplier, matching OPM behavior
    uint32_t keycode = keycode_from_freq16_32((uint16_t)freq16); // use *unmodulated* pitch
    int32_t adj = detune_adjustment(detune3, keycode);           // about -22..+22

    // Scale detune adjustment to match SGU's phase_step range vs OPM's
    // OPM phase_step range: ~41000-83000 (table values)
    // SGU phase_step for middle-C: freq16*16000/3 => roughly similar magnitudes
    // Use multiplicative scaling to convert adj to SGU units
    int64_t det_step = ((int64_t)phase_step * (int64_t)adj) >> 12;

    // Apply detune to phase_step BEFORE multiplier (like OPM does)
    int64_t adjusted_step = (int64_t)phase_step + det_step;
    if (adjusted_step < 0)
        adjusted_step = 0;

    // Apply frequency multiplier (which is an x.1 value)
    return ((uint32_t)adjusted_step * multiplier) >> 1;
}

// helper to apply KSR to the raw ADSR rate, ignoring ksr if the
// raw value is 0, and clamping to 63
static inline uint32_t effective_rate(uint32_t rawrate, uint32_t ksr)
{
    return (rawrate == 0) ? 0 : minval(rawrate + ksr, 63);
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
    return bitfield(s_increment_table[rate], 4 * index, 4);
}

// Extract block (octave 0-7) and top 4 fractional bits from freq16 for KSL calculation
static inline void freq16_to_ksl_params(uint16_t freq16, uint32_t *block, uint32_t *fnum_4msb)
{
    if (freq16 < 0x0100)
    {
        *block = 0;
        *fnum_4msb = 0;
        return;
    }
    uint32_t msb = 31u - sgu_clz32((uint32_t)freq16); // 8..15
    *block = msb - 8u;                                    // 0..7
    *fnum_4msb = (freq16 >> (msb - 4)) & 0x0F;            // top 4 bits after implicit leading 1
}

//-------------------------------------------------
//  compute_eg_rate - compute the envelope rate
//  for the given envelope state, including KSR
//-------------------------------------------------
static inline uint8_t compute_eg_rate(uint8_t op_data[], uint16_t ch_freq, enum envelope_state state)
{
    // OPM-style 5-bit keycode (block + top 2 frac bits)
    uint32_t keycode = keycode_from_freq16_32(ch_freq);
    uint32_t ksrval = keycode >> (SGU_OP0_KSR(op_data[0]) ^ 3);
    uint32_t rawrate;
    switch (state)
    {
    case SGU_EG_ATTACK:
        rawrate = SGU_OP27_AR(op_data[2], op_data[7]) * 2;
        break;
    case SGU_EG_DECAY:
        rawrate = SGU_OP27_DR(op_data[2], op_data[7]) * 2;
        break;
    case SGU_EG_SUSTAIN:
        rawrate = SGU_OP4_SR(op_data[4]) * 2;
        break;
    case SGU_EG_RELEASE:
    default:
    {
        uint8_t rr = SGU_OP3_RR(op_data[3]);
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
    return pm_scale[bitfield(pm_counter, 10, 3)];
}

//-------------------------------------------------
//  start_attack - start the attack phase; called
//  when a keyon happens or when an SSG-EG cycle
//  is complete and restarts
//-------------------------------------------------
static inline void start_attack(struct sgu_ch_state *self, uint8_t op, uint8_t op_data[], uint16_t ch_freq)
{
    // don't change anything if already in attack state
    if (self->envelope_state[op] == SGU_EG_ATTACK)
        return;
    self->envelope_state[op] = SGU_EG_ATTACK;

    // if the attack rate >= 62 then immediately go to max attenuation
    if (compute_eg_rate(op_data, ch_freq, SGU_EG_ATTACK) >= 62)
        self->envelope_attenuation[op] = 0;
}

//-------------------------------------------------
//  start_release - start the release phase;
//  called when a keyoff happens
//-------------------------------------------------
static inline void start_release(struct sgu_ch_state *self, uint8_t op)
{
    // don't change anything if already in release state
    if (self->envelope_state[op] >= SGU_EG_RELEASE)
        return;
    self->envelope_state[op] = SGU_EG_RELEASE;
}

static inline void phase_reset(struct sgu_ch_state *self, uint8_t ch, uint8_t op)
{
    self->phase[op] = 0;
    self->prev_phase[op] = 0;
    self->phase_wrap[op] = false;
    // initialize per-operator noise LFSR with unique seed per channel/operator
    self->lfsr_state[op] = 0x1FFFFF ^ ((uint32_t)(ch * SGU_OP_PER_CH + op) << 8);
}

//-------------------------------------------------
//  reset - reset the channel state
//-------------------------------------------------
static void fm_channel_reset(struct sgu_ch_state *self, uint8_t ch)
{
    // reset our data
    for (uint8_t op = 0; op < SGU_OP_PER_CH; op++)
    {
        phase_reset(self, ch, op);
        self->envelope_attenuation[op] = 0x3ff;
        self->envelope_state[op] = SGU_EG_RELEASE;
        self->key_state[op] = false;
        self->keyon_live[op] = false;
        self->keyon_gate[op] = false;
        self->eg_delay_run[op] = false;
        self->eg_delay_counter[op] = 0;
    }
}

//-------------------------------------------------
//  keyonoff - signal key on/off to our operators
//-------------------------------------------------
static inline void fm_channel_keyonoff(struct sgu_ch_state *self, bool on)
{
    for (uint8_t opnum = 0; opnum < SGU_OP_PER_CH; opnum++)
    {
        self->keyon_live[opnum] = on;
    }
}

//-------------------------------------------------
//  clock_envelope - clock the envelope state
//  according to the given count
//-------------------------------------------------
static inline void clock_envelope(struct sgu_ch_state *self, uint8_t op, uint8_t op_data[], uint16_t ch_freq, uint32_t env_counter)
{
    // handle attack->decay transitions
    if (self->envelope_state[op] == SGU_EG_ATTACK && self->envelope_attenuation[op] == 0)
        self->envelope_state[op] = SGU_EG_DECAY;

    // handle decay->sustain transitions; it is important to do this immediately
    // after the attack->decay transition above in the event that the sustain level
    // is set to 0 (in which case we will skip right to sustain without doing any
    // decay); as an example where this can be heard, check the cymbals sound
    // in channel 0 of shinobi's test mode sound #5
    if (self->envelope_state[op] == SGU_EG_DECAY && self->envelope_attenuation[op] >= compute_eg_sustain(op_data))
        self->envelope_state[op] = SGU_EG_SUSTAIN;

    // compute the 6-bit rate value for the current envelope state
    uint32_t rate = compute_eg_rate(op_data, ch_freq, self->envelope_state[op]);

    // compute the rate shift value; this is the shift needed to
    // apply to the env_counter such that it becomes a 5.11 fixed
    // point number
    uint32_t rate_shift = rate >> 2;
    env_counter <<= rate_shift;

    // see if the fractional part is 0; if not, it's not time to clock
    if (bitfield(env_counter, 0, 11) != 0)
        return;

    // determine the increment based on the non-fractional part of env_counter
    uint32_t relevant_bits = bitfield(env_counter, (rate_shift <= 11) ? 11 : rate_shift, 3);
    uint32_t increment = attenuation_increment(rate, relevant_bits);

    // attack is the only one that increases
    if (self->envelope_state[op] == SGU_EG_ATTACK)
    {
        // glitch means that attack rates of 62/63 don't increment if
        // changed after the initial key on (where they are handled
        // specially); nukeykt confirms this happens on OPM, OPN, OPL/OPLL
        // at least so assuming it is true for everyone
        if (rate < 62)
            self->envelope_attenuation[op] += (~self->envelope_attenuation[op] * increment) >> 4;
    }
    // all other cases are similar
    else
    {
        self->envelope_attenuation[op] += increment;

        // clamp the final attenuation
        if (self->envelope_attenuation[op] >= 0x400)
            self->envelope_attenuation[op] = 0x3ff;
    }
}

//-------------------------------------------------
//  clock_phase - clock the 10.10 phase value; the
//  OPN version of the logic has been verified
//  against the Nuked phase generator
//-------------------------------------------------
static inline void clock_phase(struct sgu_ch_state *self, uint8_t op, uint8_t op_data[], uint16_t ch_freq, int32_t lfo_raw_pm)
{
    uint32_t phase_step;
    const uint8_t base = SGU_OP0_MUL(op_data[0]);
    const uint8_t scale = SGU_OP4_DT(op_data[4]);
    if (SGU_OP5_FIX(op_data[5]))
    {
        // fixed frequency mode: 8..32640
        uint16_t freq16 = (uint16_t)((8 + (base * 247 + 7) / 15) << scale);
        phase_step = sgu_phase_step_freq16(freq16, 0);
    }
    else
    {
        // normal mode: compute phase step from channel frequency
        phase_step = sgu_compute_phase_step(ch_freq,
                                            compute_multiplier(base),
                                            SGU_OP0_VIB(op_data[0]) ? lfo_raw_pm : 0,
                                            scale);
    }

    // finally apply the step to the current phase value
    self->phase[op] += phase_step;
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
void SGU_NextSample(struct SGU *sgu, int32_t *l, int32_t *r)
{
    // uses int64 to avoid overflow during summation.
    int64_t L = 0;
    int64_t R = 0;

    // Cache channel 0's raw sample from the previous iteration.
    // Ring mod uses "next channel" sample; channel 8 uses channel 0, so caching makes it
    // consistent (uses previous sample period's value, not the freshly computed one).
    int16_t src0_cached = sgu->src[0];

    sgu->sample_counter += 1;
    // YMFM-style envelope counter with 2-bit subcounter
    if (EG_CLOCK_DIVIDER == 1)
        sgu->envelope_counter += 4;
    else if (bitfield(++sgu->envelope_counter, 0, 2) == EG_CLOCK_DIVIDER)
        sgu->envelope_counter += 4 - EG_CLOCK_DIVIDER;

    // clock the global LFO (once per sample)
    int32_t lfo_raw_pm = clock_lfo(
        &sgu->lfo_am_counter,
        &sgu->lfo_pm_counter,
        &sgu->lfo_am);

    // now update the state of all the channels and operators
    for (uint8_t ch = 0; ch < SGU_CHNS; ch++)
    {
        struct sgu_ch_state *ch_state = &sgu->m_channel[ch];
        const uint16_t ch_freq = sgu->chan[ch].freq;

        int32_t ch_sample = 0;

        if (sgu->chan[ch].flags0 & SGU1_FLAGS0_PCM_MASK) // PCM mode
        {
            // Signed 8-bit PCM sample scaled to match FM operator output (~14-bit range).
            ch_sample = (int16_t)sgu->pcm[sgu->chan[ch].pcmpos] << 6;

            // PCM phase accumulator. When it crosses 0x8000, advance sample position by 1.
            sgu->pcm_phase_accum[ch] += minval(sgu->chan[ch].freq, 0x8000);

            if (sgu->pcm_phase_accum[ch] >= 0x8000)
            {
                sgu->pcm_phase_accum[ch] -= 0x8000;

                // Advance sample pointer with boundary and optional looping.
                if (sgu->chan[ch].pcmpos < sgu->chan[ch].pcmbnd)
                {
                    sgu->chan[ch].pcmpos++;

                    // If we hit the boundary exactly, loop if enabled.
                    if (sgu->chan[ch].pcmpos == sgu->chan[ch].pcmbnd)
                    {
                        if (sgu->chan[ch].flags1 & SGU1_FLAGS1_PCM_LOOP)
                            sgu->chan[ch].pcmpos = sgu->chan[ch].pcmrst;
                    }

                    // Wrap to PCM RAM size (power-of-2 ring buffer).
                    sgu->chan[ch].pcmpos &= (SGU_PCM_RAM_SIZE - 1);
                }
                else if (sgu->chan[ch].flags1 & SGU1_FLAGS1_PCM_LOOP)
                {
                    // If already at/over boundary and looping, force restart.
                    sgu->chan[ch].pcmpos = sgu->chan[ch].pcmrst;
                }
            }
        }
        else
        {
            // run channel operators
            for (uint8_t op = 0; op < SGU_OP_PER_CH; op++)
            {
                uint8_t *op_data = (uint8_t *)&sgu->chan[ch].op[op];

                // clock the key state (with optional per-operator delay)
                if (ch_state->eg_delay_run[op] && ch_state->eg_delay_counter[op] < 32768)
                    ch_state->eg_delay_counter[op]++;

                const bool key_live = (ch_state->keyon_live[op] != 0);
                if (key_live && !ch_state->keyon_gate[op])
                {
                    ch_state->eg_delay_run[op] = true;
                    ch_state->eg_delay_counter[op] = 0;
                }
                else if (!key_live)
                {
                    ch_state->eg_delay_run[op] = false;
                    ch_state->eg_delay_counter[op] = 0;
                }

                const uint8_t delay = SGU_OP5_DELAY(op_data[5]);
                const uint16_t delay_target = delay ? (uint16_t)(256u << delay) : 0;
                bool keystate = key_live
                                && (!ch_state->eg_delay_run[op]
                                    || ch_state->eg_delay_counter[op] >= delay_target);
                ch_state->keyon_gate[op] = key_live;

                // has the key changed?
                if ((keystate ^ ch_state->key_state[op]) != 0)
                {
                    ch_state->key_state[op] = (uint8_t)keystate;

                    // if the key has turned on, start the attack
                    if (keystate != 0)
                        start_attack(ch_state, op, (uint8_t *)op_data, ch_freq);
                    // otherwise, start the release
                    else
                        start_release(ch_state, op);
                }

                // save previous phase for noise boundary detection
                const uint32_t phase_before = ch_state->phase[op];

                // clock the envelope (only on envelope ticks)
                if (bitfield(sgu->envelope_counter, 0, 2) == 0)
                    clock_envelope(ch_state, op, op_data, ch_freq, sgu->envelope_counter >> 2);

                // handle phase reset due to SYNC (previous op wrap from last sample)
                const uint8_t prev_op = op ? (uint8_t)(op - 1) : (uint8_t)(SGU_OP_PER_CH - 1);
                const bool sync_reset = (SGU_OP6_SYNC(op_data[6]) && ch_state->phase_wrap[prev_op]);
                if (sync_reset)
                {
                    phase_reset(ch_state, ch, op);
                }
                else
                {
                    // clock the phase (apply per-operator PM depth)
                    int32_t op_lfo_pm = lfo_raw_pm;
                    if (!SGU_OP6_VIBD(op_data[6]))
                        op_lfo_pm >>= 1;

                    clock_phase(ch_state, op, op_data, ch_freq,
                                SGU_OP0_VIB(op_data[0]) ? op_lfo_pm : 0);
                }

                // record wrap for next operator's SYNC
                ch_state->phase_wrap[op] = (ch_state->phase[op] < phase_before);

                // compute LFSR state 6x per operator cycle (only for noise waveforms)
                uint8_t wave = SGU_OP7_WAVE(op_data[7]);
                if (wave == SGU_WAVE_NOISE || wave == SGU_WAVE_PERIODIC_NOISE)
                {
                    // NOTE: This is a 6-bit LFSR, thus it takes 6 shift cycles to complete a full period.
                    // So if we take 6 times per cycle, the LFSR repeating output frequency matches the
                    // operator frequency.
                    if (((ch_state->phase[op] >> 8) * 6 >> 24) != ((phase_before >> 8) * 6 >> 24))
                    {
                        uint32_t *lfsr = &ch_state->lfsr_state[op];
                        if (wave == SGU_WAVE_NOISE)
                        {
                            *lfsr = (*lfsr >> 1 | (((*lfsr) ^ (*lfsr >> 2) ^ (*lfsr >> 3) ^ (*lfsr >> 5)) & 1) << 31);
                        }
                        else
                        {
                            switch (SGU_OP5_WPAR(op_data[5]) & 3)
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

                // get the phase modulation input
                const uint8_t mod = SGU_OP6_MOD(op_data[6]);
                // Feedback: >> 1 to average two samples, >> 1 to prevent runaway (ESFM design).
                const int16_t in_val = op ? ch_state->value[op - 1]
                                          : (ch_state->op0_fb + ch_state->value[0]) >> (1 + 1);
                // ESFM uses 13-bit samples and >> (7 - mod); SGU uses 14-bit, thus >> (8 - mod).
                const int16_t p_mod = (mod == 0)
                                          ? 0
                                          : (in_val >> (8 - mod));

                // compute the peak position for triangle/sine skew
                uint8_t wpar = SGU_OP5_WPAR(op_data[5]);
                uint16_t duty_peak = (uint16_t)(256 + (sgu->chan[ch].duty << 1)) & 0x1FF;
                uint16_t peak = (wpar & SGU_WPAR_SKEW) ? duty_peak : 256;

                //-------------------------------------------------
                // compute the 14-bit signed amplitude of this operator,
                // given a phase modulation and an AM LFO offset
                // the low 10 bits of phase represents a full 2*PI period over
                // the full sin wave
                //-------------------------------------------------
                int32_t val = 0;

                // skip work if the envelope is effectively off
                if (ch_state->envelope_attenuation[op] < EG_QUIET)
                {
                    // scale down to 10-bit phase for waveform lookup
                    // Round instead of truncate to reduce phase drift artifacts
                    int16_t phase = ((ch_state->phase[op] + (1 << 21)) >> 22) + p_mod;

                    int16_t sample = 0;

                    switch (wave)
                    {
                    case SGU_WAVE_SINE:
                    {
                        // Skewed sine: Peak at 'peak', Trough at '1024 - peak'
                        // peak=0:   Falling Sine-Saw (Starts at max, falls to min)
                        // peak=256: Symmetric Sine
                        // peak=512: Rising Sine-Saw (Rises 0->max, jumps to min, rises min->0)

                        uint32_t p = phase & 0x3FF;
                        uint32_t trough = 1024 - peak;
                        int idx;

                        if (p < peak)
                        {
                            // Segment 1: 0 -> 255
                            // (Only executes if peak > 0)
                            idx = ((int)p * 255) / (int)peak;
                        }
                        else if (p < trough)
                        {
                            // Segment 2: 256 -> 767
                            // Width is (1024 - 2*peak). If peak=0, width=1024.
                            uint32_t width = trough - peak;
                            idx = 256 + ((int)(p - peak) * 512) / (int)width;
                        }
                        else
                        {
                            // Segment 3: Rise 768 -> 1023
                            // Width is 'peak'.
                            idx = 768 + ((int)(p - trough) * 255) / (int)peak;
                        }

                        sample = sgu->waveform_lut[(uint16_t)idx & 0x1FF];
                        sample = idx < 512 ? sample : (int16_t)-sample;

                        // Apply OPL-style wave modifiers (SGU_WPAR_HALF, SGU_WPAR_ABS)
                        if (wpar & SGU_WPAR_HALF)
                            sample = (sample < 0) ? 0 : sample;
                        if (wpar & SGU_WPAR_ABS)
                            sample = (sample < 0) ? -sample : sample;
                    }
                    break;
                    case SGU_WAVE_TRIANGLE:
                    {
                        // Skewed triangle: Peak at 'peak', Trough at '1024 - peak'
                        // peak=0:   Falling Saw (Starts at max, falls to min)
                        // peak=256: Symmetric Triangle
                        // peak=512: Rising Saw (Rises 0->max, jumps to min, rises min->0)

                        uint32_t p = phase & 0x3FF;
                        uint32_t trough = 1024 - peak;

                        if (p < peak)
                        {
                            // Segment 1: Rise 0 -> 32767
                            // (Only executes if peak > 0)
                            sample = (int16_t)(((int32_t)p * 32767) / (int32_t)peak);
                        }
                        else if (p < trough)
                        {
                            // Segment 2: Fall 32767 -> -32768
                            // Width is (1024 - 2*peak). If peak=0, width=1024.
                            uint32_t width = trough - peak;
                            sample = (int16_t)(32767 - ((int32_t)(p - peak) * 65535) / (int32_t)width);
                        }
                        else
                        {
                            // Segment 3: Rise -32768 -> 0
                            // Width is 'peak'.
                            sample = (int16_t)(-32768 + ((int32_t)(p - trough) * 32767) / (int32_t)peak);
                        }

                        // Apply OPL-style wave modifiers (SGU_WPAR_HALF, SGU_WPAR_ABS)
                        if (wpar & SGU_WPAR_HALF)
                            sample = (sample < 0) ? 0 : sample;
                        if (wpar & SGU_WPAR_ABS)
                            sample = (sample < 0) ? -sample : sample;
                    }
                    break;
                    case SGU_WAVE_SAWTOOTH:
                    {
                        // WPAR bit 0 selects rising/falling (invert when set)
                        bool inverted = (wpar & 0x01);
                        int16_t phase_adj = inverted ? (int16_t)(1023 - phase) : phase;
                        uint32_t p10 = phase_adj & 0x3FF;
                        // WPAR[2:1] quantizes phase by zeroing {0,4,6,8} LSBs
                        static const uint16_t qmask[4] = {
                            F16_QMASK(0),
                            F16_QMASK(4),
                            F16_QMASK(6),
                            F16_QMASK(8),
                        };
                        p10 &= qmask[(wpar >> 1) & 0x03];
                        // Sawtooth: linear ramp 0 → max → (wrap) min → 0, no mirroring needed
                        int32_t centered = (int32_t)(p10 ^ 0x200) - 0x200; // XOR swaps halves, subtract centers at 0
                        sample = (int16_t)(centered << 6);                 // scale to 16-bit range

                        // BLEP for hard edge: detect dramatic sample change (|delta| > half range)
                        int32_t delta = (int32_t)sample - (int32_t)ch_state->blep_prev_sample[op];
                        if (delta > 32767 || delta < -32767)
                        {
                            ch_state->blep[op] = 1;
                            ch_state->blep_frac[op] = (uint16_t)(ch_state->phase[op] >> 6);
                        }
                        ch_state->blep_prev_sample[op] = sample;
                    }
                    break;
                    case SGU_WAVE_PULSE:
                    {
                        // compare phase-derived 7-bit ramp against duty (0..127).
                        // WPAR: 0 => channel duty, 1..7 => fixed pulse width (x/8)
                        uint8_t duty = sgu->chan[ch].duty;
                        if (wpar)
                            duty = (uint8_t)((uint8_t)wpar << 4);
                        uint32_t p = phase & 0x3FF;
                        sample = ((p >> 3) >= duty) ? 32767 : -32768;

                        // Detect edge by comparing with previous RAW sample (not enveloped value)
                        if (sample != ch_state->blep_prev_sample[op])
                        {
                            ch_state->blep[op] = 1; // Only 1 sample needs interpolation
                            // Capture fractional phase (22 bits) as 16-bit for sub-sample position
                            // High frac = crossed early in sample = more "new" value
                            // Low frac = crossed late in sample = more "old" value
                            ch_state->blep_frac[op] = (uint16_t)(ch_state->phase[op] >> 6);
                            ch_state->blep_prev_sample[op] = sample;
                        }
                    }
                    break;
                    case SGU_WAVE_NOISE:
                    case SGU_WAVE_PERIODIC_NOISE:
                        // Bipolar output spanning full INT16 range
                        sample = (ch_state->lfsr_state[op] & 1) ? 32767 : -32768;
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
                        uint32_t p = phase & 0x3FF; // 10-bit phase index (0-1023)
                        uint16_t sample_addr = (sgu->chan[ch].pcmrst + p) & (SGU_PCM_RAM_SIZE - 1);
                        // Scale 8-bit signed sample to 16-bit to match other waveforms
                        // (attenuation to 14-bit happens later at the envelope processing stage)
                        sample = (int16_t)((int16_t)sgu->pcm[sample_addr] << 8);
                    }
                    break;
                    }

                    // Apply ring modulation if RING bit set (R6[4])
                    // 1-bit ring mod: flip sign based on previous operator's output
                    // For op 0, uses last operator (op 3) for ring mod source
                    // Creates sum/difference sidebands for metallic/bell-like timbres
                    if (SGU_OP6_RING(op_data[6]))
                    {
                        int16_t ring_src = (op > 0) ? ch_state->value[op - 1]
                                                    : ch_state->value[SGU_OP_PER_CH - 1];
                        if (ring_src < 0)
                            sample = (int16_t)(-sample);
                    }

                    // compute the effective attenuation of the envelope
                    uint32_t env_att = ch_state->envelope_attenuation[op];

                    // add in LFO AM modulation (apply per-operator AM depth)
                    if (SGU_OP0_TRM(op_data[0]))
                    {
                        uint32_t am_offset = sgu->lfo_am;
                        if (!SGU_OP6_TRMD(op_data[6]))
                            am_offset >>= 2;
                        env_att += am_offset;
                    }

                    // add in total level, scaled by 8
                    env_att += SGU_OP16_TL(op_data[1], op_data[6]) << 3;

                    // add key scale level
                    uint32_t ksl = SGU_OP1_KSL(op_data[1]);
                    if (ksl != 0)
                    {
                        uint32_t block, fnum_4msb;
                        freq16_to_ksl_params(ch_freq, &block, &fnum_4msb);
                        env_att += opl_key_scale_atten(block, fnum_4msb) << ksl;
                    }

                    // clamp to max
                    env_att = minval(env_att, 0x3ff);

                    // the attenuation from the envelope generator as a 4.6 value, shifted up to 4.8
                    env_att <<= 2;

                    int32_t amp = sample * attenuation_to_volume(env_att);
                    // scale back to int16 range: divide by 2^13 (Q13 gain)
                    // and scale down 2 more to Q14 output range
                    // add 0.5 LSB for rounding before shifting (optional)
                    val = (amp + (1 << 14)) >> 15;
                }

                if (op == 0)
                {
                    // cache operator 0 output for delayed feedback on next sample
                    ch_state->op0_fb = ch_state->value[0];
                }

                // Store raw value for modulation (NO anti-aliasing - modulators need precise edges)
                ch_state->value[op] = (int16_t)val;

                const uint8_t out = SGU_OP7_OUT(op_data[7]);

                if (out)
                {
                    int32_t out_delta = val - ch_state->out[op];
                    ch_state->out[op] = val;
                    if (ch_state->blep[op] > 0)
                    {
                        // Sub-sample interpolation using fractional phase
                        // frac indicates how far past the edge we are:
                        //   high frac = crossed early = keep more of new value (less correction)
                        //   low frac = crossed late = keep more of old value (more correction)
                        uint16_t frac = ch_state->blep_frac[op];
                        ch_state->out[op] -= ((int32_t)out_delta * (65536 - frac)) >> 16;
                        ch_state->blep[op]--;
                    }
                    ch_sample += ch_state->out[op] >> (7 - out);
                }
                else
                {
                    ch_state->out[op] = 0;
                }
            }
        }

        // ------------------------------------------------------------
        // Store raw FM output for ring modulation (used by previous channel next sample)
        // ------------------------------------------------------------
        int16_t raw_sample = (int16_t)clamp(ch_sample, -32768, 32767);

        // ------------------------------------------------------------
        // 4a) Channel-level ring modulation (amplitude modulation style)
        // Uses previous sample period's output from next channel (or ch0 for last channel)
        // ------------------------------------------------------------
        if (sgu->chan[ch].flags0 & SGU1_FLAGS0_CTL_RING_MOD)
        {
            // Multiply by next channel's cached sample (ch+1, or cached ch0 for last channel)
            // >>15 rescales back to 16-bit range (two 16-bit values multiplied)
            const int16_t ring_src = (ch == SGU_CHNS - 1) ? src0_cached : sgu->src[ch + 1];
            ch_sample = ((int32_t)ch_sample * ring_src) >> 15;
        }

        // Store this channel's raw sample for next iteration's ring modulation
        sgu->src[ch] = raw_sample;

        // ------------------------------------------------------------
        // 4b) Apply channel volume scaling
        // ------------------------------------------------------------
        // ch_sample is already the FM operator sum; now apply channel volume
        // vol is signed 8-bit (-128..127), allowing phase inversion
        int32_t voice_sample = (ch_sample * sgu->chan[ch].vol) >> 7;

        // ------------------------------------------------------------
        // 5) Optional resonant filter (state-variable filter)
        // flags0 bits 5..7 select which outputs to mix: LP/HP/BP.
        // ------------------------------------------------------------
        if (sgu->chan[ch].flags0 & (SGU1_FLAGS0_CTL_NSLOW | SGU1_FLAGS0_CTL_NSHIGH | SGU1_FLAGS0_CTL_NSBAND))
        {
            // Scale cutoff for 48kHz operation.
            // SGU runs at 48kHz vs SU's ~297kHz (~6× slower).
            // 3× scaling gives 0.35Hz-23kHz range, stable for all filter modes.
            // Formula: fc ≈ (ff / 65536) × (sample_rate / 2π)
            // With 3× scaling at 48kHz: fc ≈ cutoff × 0.35 Hz (0.35Hz to ~23kHz)
            // Max effective ff = 196,605, well within int32_t multiply safety.
            int32_t ff = (int32_t)sgu->chan[ch].cutoff * 3;

            // Apply resonance-driven distortion before filter (SID characteristic).
            // Higher resonance drives harder saturation, adding harmonic distortion.
            // Gain scales from 1.0× (reson=0) to 1.5× (reson=255).
            int32_t drive = 256 + (sgu->chan[ch].reson >> 1); // 256-383
            voice_sample = svf_saturate((voice_sample * drive) >> 8);

            // SVF core with soft saturation for analog warmth.
            // Saturation on low/band states emulates analog op-amp behavior,
            // prevents harsh digital clipping at high resonance.
            sgu->svf_low[ch] = svf_saturate(sgu->svf_low[ch] + ((ff * sgu->svf_band[ch]) >> 16));
            sgu->svf_high[ch] = voice_sample - sgu->svf_low[ch] - (((256 - sgu->chan[ch].reson) * sgu->svf_band[ch]) >> 8);
            sgu->svf_band[ch] = svf_saturate(((ff * sgu->svf_high[ch]) >> 16) + sgu->svf_band[ch]);

            // Select which components to output (LP/HP/BP can be combined).
            voice_sample = ((sgu->chan[ch].flags0 & SGU1_FLAGS0_CTL_NSLOW) ? sgu->svf_low[ch] : 0)
                           + ((sgu->chan[ch].flags0 & SGU1_FLAGS0_CTL_NSHIGH) ? sgu->svf_high[ch] : 0)
                           + ((sgu->chan[ch].flags0 & SGU1_FLAGS0_CTL_NSBAND) ? sgu->svf_band[ch] : 0);
        }

        // Store post-processed sample for debug/meters
        sgu->post[ch] = voice_sample;

        // ------------------------------------------------------------
        // 7) Panning (apply per-channel stereo gains)
        // ------------------------------------------------------------
        sgu->outL[ch] = (voice_sample * sgu->pan_gain_lut_l[(uint8_t)sgu->chan[ch].pan]) >> 7;
        sgu->outR[ch] = (voice_sample * sgu->pan_gain_lut_r[(uint8_t)sgu->chan[ch].pan]) >> 7;

        // ------------------------------------------------------------
        // 8) Sweeps (affect parameters for future samples)
        // ------------------------------------------------------------

        // 8a) Volume sweep (flags1 bit5).
        // swvol.amt encoding:
        //   bit5 (0x20): direction (1=up, 0=down)
        //   bits0..4: step size
        //   bit6 (0x40): "wrap/loop" behavior
        //   bit7 (0x80): "bounce/alternate" behavior
        if ((sgu->chan[ch].flags1 & SGU1_FLAGS1_VOL_SWEEP) && sgu->chan[ch].swvol.speed)
        {
            if (--sgu->vol_sweep_countdown[ch] <= 0)
            {
                sgu->vol_sweep_countdown[ch] += sgu->chan[ch].swvol.speed;

                if (sgu->chan[ch].swvol.amt & 32) // up
                {
                    int v = sgu->chan[ch].vol + (sgu->chan[ch].swvol.amt & 31);
                    sgu->chan[ch].vol = (v > 127) ? 127 : (v < -128) ? -128
                                                                     : (int8_t)v;

                    // If not wrapping, clamp at upper bound.
                    if (sgu->chan[ch].vol > (int8_t)sgu->chan[ch].swvol.bound && !(sgu->chan[ch].swvol.amt & 64))
                        sgu->chan[ch].vol = (int8_t)sgu->chan[ch].swvol.bound;

                    // Handle wrap/bounce on overflow sign bit.
                    if (sgu->chan[ch].vol & 0x80)
                    {
                        if (sgu->chan[ch].swvol.amt & 64) // wrap enabled
                        {
                            if (sgu->chan[ch].swvol.amt & 128) // bounce enabled
                            {
                                sgu->chan[ch].swvol.amt ^= 32;                          // flip direction
                                sgu->chan[ch].vol = (int8_t)(0xFF - sgu->chan[ch].vol); // reflect
                            }
                            else
                            {
                                sgu->chan[ch].vol &= ~0x80; // wrap into positive
                            }
                        }
                        else
                        {
                            sgu->chan[ch].vol = 0x7F; // clamp
                        }
                    }
                }
                else // down
                {
                    int v = sgu->chan[ch].vol - (sgu->chan[ch].swvol.amt & 31);
                    sgu->chan[ch].vol = (v > 127) ? 127 : (v < -128) ? -128
                                                                     : (int8_t)v;

                    if (sgu->chan[ch].vol & 0x80)
                    {
                        if (sgu->chan[ch].swvol.amt & 64) // wrap enabled
                        {
                            if (sgu->chan[ch].swvol.amt & 128) // bounce enabled
                            {
                                sgu->chan[ch].swvol.amt ^= 32; // flip direction
                                sgu->chan[ch].vol = (int8_t)(-sgu->chan[ch].vol);
                            }
                            else
                            {
                                sgu->chan[ch].vol &= ~0x80;
                            }
                        }
                        else
                        {
                            sgu->chan[ch].vol = 0x00; // clamp at 0
                        }
                    }

                    // If not wrapping, clamp at lower bound.
                    if (sgu->chan[ch].vol < (int8_t)sgu->chan[ch].swvol.bound && !(sgu->chan[ch].swvol.amt & 64))
                        sgu->chan[ch].vol = (int8_t)sgu->chan[ch].swvol.bound;
                }
            }
        }

        // 8b) Frequency sweep (flags1 bit4).
        // swfreq.amt bit7: direction (1=up, 0=down), bits0..6 magnitude.
        if ((sgu->chan[ch].flags1 & SGU1_FLAGS1_FREQ_SWEEP) && sgu->chan[ch].swfreq.speed)
        {
            if (--sgu->freq_sweep_countdown[ch] <= 0)
            {
                sgu->freq_sweep_countdown[ch] += sgu->chan[ch].swfreq.speed;

                if (sgu->chan[ch].swfreq.amt & 128) // up
                {
                    if (sgu->chan[ch].freq > (0xFFFF - (sgu->chan[ch].swfreq.amt & 127)))
                        sgu->chan[ch].freq = 0xFFFF;
                    else
                    {
                        // Multiply by (1.0 + amt/128).
                        sgu->chan[ch].freq = (uint16_t)((sgu->chan[ch].freq * (0x80 + (sgu->chan[ch].swfreq.amt & 127))) >> 7);

                        // Clamp to bound (stored as coarse high-byte limit).
                        if ((sgu->chan[ch].freq >> 8) > sgu->chan[ch].swfreq.bound)
                            sgu->chan[ch].freq = (uint16_t)(sgu->chan[ch].swfreq.bound << 8);
                    }
                }
                else // down
                {
                    if (sgu->chan[ch].freq < (sgu->chan[ch].swfreq.amt & 127))
                        sgu->chan[ch].freq = 0;
                    else
                    {
                        // Multiply by (1.0 - amt/256).
                        sgu->chan[ch].freq = (uint16_t)((sgu->chan[ch].freq * (0xFF - (sgu->chan[ch].swfreq.amt & 127))) >> 8);

                        if ((sgu->chan[ch].freq >> 8) < sgu->chan[ch].swfreq.bound)
                            sgu->chan[ch].freq = (uint16_t)(sgu->chan[ch].swfreq.bound << 8);
                    }
                }
            }
        }

        // 8c) Cutoff sweep (flags1 bit6).
        // swcut.amt bit7: direction (1=up, 0=down), bits0..6 magnitude.
        if ((sgu->chan[ch].flags1 & SGU1_FLAGS1_CUT_SWEEP) && sgu->chan[ch].swcut.speed)
        {
            if (--sgu->cut_sweep_countdown[ch] <= 0)
            {
                sgu->cut_sweep_countdown[ch] += sgu->chan[ch].swcut.speed;

                if (sgu->chan[ch].swcut.amt & 128) // up
                {
                    if (sgu->chan[ch].cutoff > (0xFFFF - (sgu->chan[ch].swcut.amt & 127)))
                        sgu->chan[ch].cutoff = 0xFFFF;
                    else
                    {
                        sgu->chan[ch].cutoff += (sgu->chan[ch].swcut.amt & 127);

                        if ((sgu->chan[ch].cutoff >> 8) > sgu->chan[ch].swcut.bound)
                            sgu->chan[ch].cutoff = (uint16_t)(sgu->chan[ch].swcut.bound << 8);
                    }
                }
                else // down
                {
                    if (sgu->chan[ch].cutoff < (sgu->chan[ch].swcut.amt & 127))
                        sgu->chan[ch].cutoff = 0;
                    else
                    {
                        // Multiply by (1.0 - amt/2048).
                        sgu->chan[ch].cutoff = (uint16_t)(((2048 - (unsigned int)(sgu->chan[ch].swcut.amt & 127)) * (unsigned int)sgu->chan[ch].cutoff) >> 11);

                        if ((sgu->chan[ch].cutoff >> 8) < sgu->chan[ch].swcut.bound)
                            sgu->chan[ch].cutoff = (uint16_t)(sgu->chan[ch].swcut.bound << 8);
                    }
                }
            }
        }

        // ------------------------------------------------------------
        // 9) One-shot phase reset request (flags1 bit0)
        // ------------------------------------------------------------
        if (sgu->chan[ch].flags1 & SGU1_FLAGS1_PHASE_RESET)
        {
            // Reset all operator phases for this channel
            for (uint8_t op = 0; op < SGU_OP_PER_CH; op++)
            {
                phase_reset(ch_state, ch, op);
            }
            sgu->phase_reset_countdown[ch] = sgu->chan[ch].restimer; // preload timer sync counter
            sgu->chan[ch].flags1 &= ~SGU1_FLAGS1_PHASE_RESET;        // clear request
        }

        // Filter phase reset (flags1 bit1)
        if (sgu->chan[ch].flags1 & SGU1_FLAGS1_FILTER_PHASE_RESET)
        {
            sgu->svf_low[ch] = sgu->svf_high[ch] = sgu->svf_band[ch] = 0;
            sgu->chan[ch].flags1 &= ~SGU1_FLAGS1_FILTER_PHASE_RESET; // clear request
        }

        // Timer sync: periodic phase reset (flags1 bit3)
        if ((sgu->chan[ch].flags1 & SGU1_FLAGS1_TIMER_SYNC) && sgu->chan[ch].restimer)
        {
            if (--sgu->phase_reset_countdown[ch] <= 0)
            {
                sgu->phase_reset_countdown[ch] += sgu->chan[ch].restimer;
                // Reset all operator phases
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
            sgu->outL[ch] = sgu->outR[ch] = 0;
        }

        // ------------------------------------------------------------
        // Mixdown: sum all channel stereo contributions into output.
        // ------------------------------------------------------------
        L += sgu->outL[ch];
        R += sgu->outR[ch];
    }

    // --------------- Final output processing ----------------
    // Leaky Integrator HPF to remove DC offset and create pulse droop

    // 1. Calculate the difference (delta) from the last input
    int64_t diff_L = L - sgu->L_in;
    int64_t diff_R = R - sgu->R_in;

    // 2. High Pass Filter: y[n] = alpha * (y[n-1] + x[n] - x[n-1])
    // We shift the diff to Q16 to match the state's fixed-point scale
    int64_t L_q16 = (SGU_ALPHA_RC_DECAY_Q16 * (sgu->L_q16 + ((int64_t)diff_L << 16))) >> 16;
    int64_t R_q16 = (SGU_ALPHA_RC_DECAY_Q16 * (sgu->R_q16 + ((int64_t)diff_R << 16))) >> 16;

    // 3. Store states for next iteration
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

void SGU_Init(struct SGU *sgu, size_t sampleMemSize)
{
    (void)sampleMemSize;
    memset(sgu, 0, sizeof(struct SGU));

    // create the sine LUT
    for (size_t i = 0; i < SGU_WAVEFORM_LENGTH / 2; i++)
    {
        // sine
        const float sin_val = sin(((float)i / ((float)SGU_WAVEFORM_LENGTH / 2)) * M_PI) * INT16_MAX;
        sgu->waveform_lut[i] = (int16_t)sin_val;
    }

    // Build pan gain lookup tables (same as su.c)
    // Start with "center pan": both gains 127
    for (size_t i = 0; i < 256; i++)
    {
        sgu->pan_gain_lut_l[i] = 127;
        sgu->pan_gain_lut_r[i] = 127;
    }
    // Pan shaping:
    // - For i=0..127: left gain decreases from 127->0 (panned right)
    // - For i=128..255: right gain increases from 0->126 (panned left)
    for (size_t i = 0; i < 128; i++)
    {
        sgu->pan_gain_lut_l[i] = (uint8_t)(127 - i);
        sgu->pan_gain_lut_r[128 + i] = (uint8_t)(i - 1);
    }
    sgu->pan_gain_lut_r[128] = 0;

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

#if IN_EMU
    uint8_t AR5 = 4;
    uint8_t DR5 = 31;
    uint8_t SL4 = 12;
    uint8_t SR5 = 0;
    uint8_t RR4 = 15;

    // channel FREQ and gate
    SGU_Write(sgu, 0x20, 0xD6);
    SGU_Write(sgu, 0x21, 0x1C);
#if 0
    SGU_Write(sgu, 0x24, 0x01); // KEY
    SGU_Write(sgu, 0x28, 0x44); // DUTY
    // operator 1
    SGU_Write(sgu, 0x00, 0x01);
    SGU_Write(sgu, 0x02, 0xF0);
    SGU_Write(sgu, 0x07, 0xE0);
#else
    SGU_Write(sgu, 0x00, 0x01); // MUL=1
    // Envelope
    SGU_Write(sgu, 0x02, (uint8_t)((AR5 & 0xF) << 4) | (DR5 & 0xF));
    SGU_Write(sgu, 0x03, (uint8_t)((SL4 & 0xF) << 4) | (RR4 & 0xF));
    SGU_Write(sgu, 0x04, (uint8_t)((SR5 & 0x1F)));
    SGU_Write(sgu, 0x07, (uint8_t)((AR5 & 0x10) | ((DR5 & 0x10) >> 1) | SGU_WAVE_SINE | 0xE0)); // Full OUT, sine
#endif
    // // operator 2
    // SGU_Write(sgu, 0x08, 0x02);
    // SGU_Write(sgu, 0x0a, 0xF0);
    // SGU_Write(sgu, 0x0e, 0x0E);
    // SGU_Write(sgu, 0x0f, 0xF0);
    // // operator 3
    // SGU_Write(sgu, 0x10, 0x03);
    // SGU_Write(sgu, 0x12, 0xF0);
    // SGU_Write(sgu, 0x17, 0x80);
    // // operator 4
    // SGU_Write(sgu, 0x18, 0x05);
    // SGU_Write(sgu, 0x1a, 0xF0);
    // SGU_Write(sgu, 0x1f, 0xF0);

    // SGU_Write(sgu, 0x40 + 0x00, 0x01);
    // SGU_Write(sgu, 0x40 + 0x02, 0xF0);
    // SGU_Write(sgu, 0x40 + 0x07, 0xF0);
    // SGU_Write(sgu, 0x80 + 0x00, 0x01);
    // SGU_Write(sgu, 0x80 + 0x02, 0xF0);
    // SGU_Write(sgu, 0x80 + 0x07, 0xF0);
#endif
}

void SGU_Write(struct SGU *sgu, uint16_t addr13, uint8_t data)
{
    ((uint8_t *)sgu->chan)[addr13] = data;
    const uint8_t channel = (addr13 / SGU_REGS_PER_CH) % SGU_CHNS;
    // handle writes to the keyon register(s)
    fm_channel_keyonoff(&sgu->m_channel[channel], sgu->chan[channel].flags0 & SGU1_FLAGS0_CTL_GATE);
    // printf("SGU_Write: addr=0x%02X data=0x%02X -> (chan=%u)\n", addr13, data, channel);
}

static_assert(sizeof(struct SGU_CH) == (SGU_OP_PER_CH * SGU_OP_REGS + SGU_CH_REGS), "SGU channel size mismatch");
static_assert(SGU_REGS_PER_CH == (SGU_OP_PER_CH * SGU_OP_REGS + SGU_CH_REGS), "SGU regs size mismatch");

int32_t SGU_GetSample(struct SGU *sgu, uint8_t ch)
{
    // Return the post-processed mono sample (after volume/filter, before pan)
    int32_t ret = sgu->post[ch];
    if (ret < -32768)
        ret = -32768;
    if (ret > 32767)
        ret = 32767;
    return ret;
}
