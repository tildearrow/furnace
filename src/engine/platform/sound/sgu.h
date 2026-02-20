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

#pragma once

/**
 * SGU-1 - Sound Generator Unit 1
 *
 * SGU-1 is a hybrid sound synthesis unit combining features
 * from additive operator synthesis and subtractive filtering.
 *
 * - Per-voice sound is produced by a 4-operator ESFM-style chain;
 *   each operator can act as modulator and/or carrier via per-operator MOD
 *   (phase modulation from previous operator, with feedback on operator 0)
 *   and per-operator OUT (direct contribution to the voice mix).
 * - One base pitch per channel. Operator pitch is derived from channel pitch
 *   using OPL-style MUL plus per-operator DT detune.
 * - Optional per-operator fixed-frequency mode (OPZ-like) enabled by a flag;
 *   fixed frequency is selected by reinterpreting existing MUL+DT fields.
 * - Per-operator level and timbre controls: TL attenuation with
 *   key-level scaling (KSL), KSR/rate scaling, waveform selection,
 *   tremolo/vibrato enable and depth, plus optional hard sync
 *   and ring modulation to the previous operator.
 * - Per-operator envelope uses AR/DR/SR/SL/RR for improved percussive control.
 * - Register layout is compact and Yamaha-like:
 *   8 bytes per operator, closely aligned with OPL/OPL3-style ergonomics
 *   and ESFM routing flexibility, while enabling reasonable reproduction
 *   of OPN/M-family chiptunes and broadly similar FM/subtractive workflows.
 * - Operator output is mixed into a per-voice pipeline that applies
 *   SID-like shaping (multimode filter with cutoff/resonance),
 *   plus per-channel volume/frequency/filter slides with optional wrap/bounce behaviors.
 * - Channel supports waveform or PCM sample playback; one-shot and looping,
 *   per-channel volume control and stereo panning.
 */

#define SGU_CHIP_CLOCK     (48000) // 48kHz
#define SGU_AUDIO_CHANNELS (2)

// Alpha = 0.99869 scaled to 16-bit (0.99869 * 65536)
#define SGU_ALPHA_RC_DECAY_Q16 (65450)

#define SGU_CHNS        (9)
#define SGU_CH_REGS     (32)
#define SGU_OP_PER_CH   (4)
#define SGU_OP_REGS     (8)
#define SGU_REGS_PER_CH (SGU_OP_PER_CH * SGU_OP_REGS + SGU_CH_REGS)

#define SGU_PCM_RAM_SIZE    (0x10000) // 64KB PCM RAM
#define SGU_WAVEFORM_LENGTH (0x400)   // 1024 samples per waveform

/*
### Key differences vs OPL/OPM/ESFM

- Envelopes: AR/DR/SR/SL/RR structure (like OPN/OPM), vs OPL's AR/DR/SL+RR.
  For better percussive envelope shaping.
- TL is 7-bit (0..127) vs OPL's 6-bit (0..63), for finer attenuation steps.
- KSR is 2-bit (0..3) vs OPL's 1-bit (0/1), for better rate scaling.
- MUL is OPL-style (0=0.5x, 1..15=1x..15x) vs OPN/OPM's (1..16=1x..16x).
- DT is 3-bit (0..7) vs OPL's 2-bit (0..3), for finer detune steps.
- SR (D2R) is explicit 5-bit (0..31) vs OPL's implicit (0 or 15).
- WAVE selection is per-operator (3-bit) vs OPL's global per-channel (2-bit).
- MOD/OUT routing like ESFM (per-operator) vs OPL's fixed 2-op algorithms.

### Tremolo/Vibrato

- Tremolo (AM) and Vibrato (PM) enable bits are per-operator (R0[7], R0[6])
  vs OPL's per-channel (R0[7:6]).
- Tremolo depth is global per-channel (like OPL).
- Vibrato depth is global per-channel (like OPL).
- Hard sync and ring modulation are per-operator (R6[5], R6[4]).

### Fixed frequency mode (OPZ-like)

- Enabled by FIX bit (R5[4]) per-operator.
- In fixed mode, operator ignores channel base frequency.
- Fixed frequency is derived from MUL and DT reinterpreted:
  `freq16 = (8 + (MUL * 247 + 7) / 15) << DT`  (range ≈ 8..32640).
- In fixed mode, DT does not act as detune.

---

## Operator frequency behavior (single channel pitch)

Let channel base frequency be `f_ch` (from channel FREQ).

### Ratio mode (FIX=0)

* `f_op = f_ch * mul(MUL) + detune(DT)`
* `MUL` uses OPL semantics (0 => 0.5×, 1..15 => 1×..15×).
* `DT` is per-operator detune (3 bits) based on keycode.

### Fixed mode (FIX=1, OPZ-like)

* Operator ignores `f_ch`.
* Fixed frequency uses MUL as a base and DT as a scale:
  `freq16 = (8 + (MUL * 247 + 7) / 15) << DT`  (range ≈ 8..32640).
* In fixed mode, DT does not act as detune.

---

### Wave parameter WPAR

Additional WAVE form related parameter (per-operator, 4 bits)

- PULSE
  Selects channel pulse width or a fixed per-operator pulse width:
  0 => use channel pulse width
  1..15 => x/16 pulse width (x low units, 16-x high units)
- SINE, TRIANGLE, SAWTOOTH
  bit 3 = 0:
    bits 0..2 selects OPL-style variant of the base waveform:
    1 (HALF_L): "half-sine" - part of the wave before DUTY is silenced
    2 (HALF_H): "half-sine" - part of the wave after DUTY is silenced
    3 (ABS_L) : "absolute-sine" - negate part of the wave before DUTY
    4 (ABS_H) : "absolute-sine" - negate part of the wave after DUTY
  bit 3 = 1: quantize wave table look-up by zeroing out some low bits, creating stepped waveforms:
    bits 0..2: selects how many bits to zero-out (bits0..2 + 1)
- PERIODIC_NOISE
  bits 0..1: selects 6-bit LFSR tap configuration:
    0: taps 3,4 (~31 states)
    1: taps 2,3 (~31 states)
    2: taps 0,2,3 (~63 states)
    3: taps 0,2,3,5 (~63 states)
  This is per-operator, allowing different noise timbres in each operator


## MOD / OUT routing

### MOD (R6[3:1])

* Sets how much this operator is phase-modulated by the **previous operator** (6 dB steps).
* For operator 0, MOD becomes **feedback level** (ESFM behavior).

### OUT (R7[7:5])

* Sets this operator’s direct contribution to the channel mix (6 dB steps).
* By choosing MOD/OUT per operator you can realize OPL-style 2-op algorithms, OPN/OPM-ish 4-op structures, and hybrids (operator can be both modulator and carrier).

*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// -----------------------------------------------------------------------------
// Operator register offsets (0-7 within each operator's 8-byte block)
// -----------------------------------------------------------------------------
#define SGU_OP_REG_MUL      0 // R0: [7]TRM [6]VIB [5:4]KSR [3:0]MUL
#define SGU_OP_REG_TL       1 // R1: [7:6]KSL [5:0]TL_lo6
#define SGU_OP_REG_AR_DR    2 // R2: [7:4]AR_lo4 [3:0]DR_lo4
#define SGU_OP_REG_SL_RR    3 // R3: [7:4]SL [3:0]RR
#define SGU_OP_REG_DT_SR    4 // R4: [7:5]DT [4:0]SR
#define SGU_OP_REG_DELAY    5 // R5: [7:5]DELAY [4]FIX [3:0]WPAR
#define SGU_OP_REG_MOD      6 // R6: [7]TRMD [6]VIBD [5]SYNC [4]RING [3:1]MOD [0]TL_msb
#define SGU_OP_REG_OUT_WAVE 7 // R7: [7:5]OUT [4]AR_msb [3]DR_msb [2:0]WAVE

// -----------------------------------------------------------------------------
// Operator bitfields
// -----------------------------------------------------------------------------

// R0: [7]TRM [6]VIB [5:4]KSR [3:0]MUL
#define SGU_OP0_MUL_MASK  0x0F
#define SGU_OP0_KSR_MASK  0x30
#define SGU_OP0_KSR_SHIFT 4
#define SGU_OP0_VIB_BIT   0x40
#define SGU_OP0_TRM_BIT   0x80

#define SGU_OP0_TRM(reg) ((reg) & SGU_OP0_TRM_BIT)
#define SGU_OP0_VIB(reg) ((reg) & SGU_OP0_VIB_BIT)
#define SGU_OP0_KSR(reg) (((reg) & SGU_OP0_KSR_MASK) >> SGU_OP0_KSR_SHIFT)
#define SGU_OP0_MUL(reg) ((reg) & SGU_OP0_MUL_MASK)

// R1: [7:6]KSL [5:0]TL_lo6
#define SGU_OP1_TL_MASK   0x3F
#define SGU_OP1_TL_OFFSET 6
#define SGU_OP1_KSL_MASK  0xC0
#define SGU_OP1_KSL_SHIFT 6

#define SGU_OP1_KSL(reg)    (((reg) & SGU_OP1_KSL_MASK) >> SGU_OP1_KSL_SHIFT)
#define SGU_OP1_TL_LO6(reg) ((reg) & SGU_OP1_TL_MASK)

// R2: [7:4]AR_lo4 [3:0]DR_lo4
#define SGU_OP2_DR_MASK   0x0F
#define SGU_OP2_DR_OFFSET 4
#define SGU_OP2_AR_MASK   0xF0
#define SGU_OP2_AR_SHIFT  4
#define SGU_OP2_AR_OFFSET 4

#define SGU_OP2_AR_LO4(reg) (((reg) & SGU_OP2_AR_MASK) >> SGU_OP2_AR_SHIFT)
#define SGU_OP2_DR_LO4(reg) ((reg) & SGU_OP2_DR_MASK)

// R3: [7:4]SL [3:0]RR
#define SGU_OP3_RR_MASK  0x0F
#define SGU_OP3_SL_MASK  0xF0
#define SGU_OP3_SL_SHIFT 4

#define SGU_OP3_SL(reg) (((reg) & SGU_OP3_SL_MASK) >> SGU_OP3_SL_SHIFT)
#define SGU_OP3_RR(reg) ((reg) & SGU_OP3_RR_MASK)

// R4: [7:5]DT [4:0]SR
#define SGU_OP4_SR_MASK  0x1F
#define SGU_OP4_DT_MASK  0xE0
#define SGU_OP4_DT_SHIFT 5

#define SGU_OP4_DT(reg) (((reg) & SGU_OP4_DT_MASK) >> SGU_OP4_DT_SHIFT)
#define SGU_OP4_SR(reg) ((reg) & SGU_OP4_SR_MASK)

// R5: [7:5]DELAY [4]FIX [3:0]WPAR
#define SGU_OP5_WPAR_MASK   0x0F
#define SGU_OP5_FIX_BIT     0x10
#define SGU_OP5_DELAY_MASK  0xE0
#define SGU_OP5_DELAY_SHIFT 5

#define SGU_OP5_DELAY(reg) (((reg) & SGU_OP5_DELAY_MASK) >> SGU_OP5_DELAY_SHIFT)
#define SGU_OP5_FIX(reg)   ((reg) & SGU_OP5_FIX_BIT)
#define SGU_OP5_WPAR(reg)  ((reg) & SGU_OP5_WPAR_MASK)

// WPAR OPL-style modifiers
#define SGU_WPAR_HALF_L (1) // "half-sine" - part of the wave before DUTY is silenced
#define SGU_WPAR_HALF_H (2) // "half-sine" - part of the wave AFTER DUTY is silenced
#define SGU_WPAR_ABS_L  (3) // "absolute-sine" - negate part of the wave before DUTY
#define SGU_WPAR_ABS_H  (4) // "absolute-sine" - negate part of the wave after DUTY
// WPAR wave quantization
#define SGU_WPAR_QUANT  (1 << 3) // bit 3: quantize wave table look-up by zeroing out some low bits

// R6: [7]TRMD [6]VIBD [5]SYNC [4]RING [3:1]MOD [0]TL_msb
#define SGU_OP6_TL_MSB_BIT 0x01
#define SGU_OP6_MOD_MASK   0x0E
#define SGU_OP6_MOD_SHIFT  1
#define SGU_OP6_RING_BIT   0x10
#define SGU_OP6_SYNC_BIT   0x20
#define SGU_OP6_VIBD_BIT   0x40
#define SGU_OP6_TRMD_BIT   0x80

#define SGU_OP6_TRMD(reg)   ((reg) & SGU_OP6_TRMD_BIT)
#define SGU_OP6_VIBD(reg)   ((reg) & SGU_OP6_VIBD_BIT)
#define SGU_OP6_SYNC(reg)   ((reg) & SGU_OP6_SYNC_BIT)
#define SGU_OP6_RING(reg)   ((reg) & SGU_OP6_RING_BIT)
#define SGU_OP6_MOD(reg)    (((reg) & SGU_OP6_MOD_MASK) >> SGU_OP6_MOD_SHIFT)
#define SGU_OP6_TL_MSB(reg) ((reg) & SGU_OP6_TL_MSB_BIT)
#define SGU_OP16_TL(reg1, reg6) \
    (SGU_OP1_TL_LO6(reg1) | (SGU_OP6_TL_MSB(reg6) << SGU_OP1_TL_OFFSET))

// R7: [7:5]OUT [4]AR_msb [3]DR_msb [2:0]WAVE
#define SGU_OP7_WAVE_MASK    0x07
#define SGU_OP7_DR_MSB_BIT   0x08
#define SGU_OP7_DR_MSB_SHIFT 3
#define SGU_OP7_AR_MSB_BIT   0x10
#define SGU_OP7_AR_MSB_SHIFT 4
#define SGU_OP7_OUT_MASK     0xE0
#define SGU_OP7_OUT_SHIFT    5

#define SGU_OP7_OUT(reg)    (((reg) & SGU_OP7_OUT_MASK) >> SGU_OP7_OUT_SHIFT)
#define SGU_OP7_AR_MSB(reg) (((reg) & SGU_OP7_AR_MSB_BIT) >> SGU_OP7_AR_MSB_SHIFT)
#define SGU_OP7_DR_MSB(reg) (((reg) & SGU_OP7_DR_MSB_BIT) >> SGU_OP7_DR_MSB_SHIFT)
#define SGU_OP7_WAVE(reg)   ((reg) & SGU_OP7_WAVE_MASK)
#define SGU_OP27_AR(reg2, reg7) \
    (SGU_OP2_AR_LO4(reg2) | (SGU_OP7_AR_MSB(reg7) << SGU_OP2_AR_OFFSET))
#define SGU_OP27_DR(reg2, reg7) \
    (SGU_OP2_DR_LO4(reg2) | (SGU_OP7_DR_MSB(reg7) << SGU_OP2_DR_OFFSET))

// -----------------------------------------------------------------------------
// Channel register set (per channel)
// -----------------------------------------------------------------------------
#define SGU1_CHN_FREQ_L       (0x00)
#define SGU1_CHN_FREQ_H       (0x01)
#define SGU1_CHN_VOL          (0x02)
#define SGU1_CHN_PAN          (0x03)
#define SGU1_CHN_FLAGS0       (0x04)
#define SGU1_CHN_FLAGS1       (0x05)
#define SGU1_CHN_CUTOFF_L     (0x06)
#define SGU1_CHN_CUTOFF_H     (0x07)
#define SGU1_CHN_DUTY         (0x08)
#define SGU1_CHN_RESON        (0x09)
#define SGU1_CHN_PCM_POS_L    (0x0A)
#define SGU1_CHN_PCM_POS_H    (0x0B)
#define SGU1_CHN_PCM_END_L    (0x0C)
#define SGU1_CHN_PCM_END_H    (0x0D)
#define SGU1_CHN_PCM_RST_L    (0x0E)
#define SGU1_CHN_PCM_RST_H    (0x0F)
#define SGU1_CHN_SWFREQ_SPD_L (0x10)
#define SGU1_CHN_SWFREQ_SPD_H (0x11)
#define SGU1_CHN_SWFREQ_AMT   (0x12)
#define SGU1_CHN_SWFREQ_BND   (0x13)
#define SGU1_CHN_SWVOL_SPD_L  (0x14)
#define SGU1_CHN_SWVOL_SPD_H  (0x15)
#define SGU1_CHN_SWVOL_AMT    (0x16)
#define SGU1_CHN_SWVOL_BND    (0x17)
#define SGU1_CHN_SWCUT_SPD_L  (0x18)
#define SGU1_CHN_SWCUT_SPD_H  (0x19)
#define SGU1_CHN_SWCUT_AMT    (0x1A)
#define SGU1_CHN_SWCUT_BND    (0x1B)
#define SGU1_CHN_RESTIMER_L   (0x1C)
#define SGU1_CHN_RESTIMER_H   (0x1D)
#define SGU1_CHN_SPECIAL1     (0x1E)
#define SGU1_CHN_SPECIAL2     (0x1F)

// channel control bits
#define SGU1_FLAGS0_CTL_GATE      (1 << 0)
#define SGU1_FLAGS0_PCM_SHIFT     (3)
#define SGU1_FLAGS0_PCM_MASK      (0x1 << SGU1_FLAGS0_PCM_SHIFT)
#define SGU1_FLAGS0_CONTROL_SHIFT (4)
#define SGU1_FLAGS0_CONTROL_MASK  (0xF << SGU1_FLAGS0_CONTROL_SHIFT)
#define SGU1_FLAGS0_CTL_RING_MOD  (1 << 4)
#define SGU1_FLAGS0_CTL_NSLOW     (1 << 5)
#define SGU1_FLAGS0_CTL_NSHIGH    (1 << 6)
#define SGU1_FLAGS0_CTL_NSBAND    (1 << 7)

#define SGU1_FLAGS1_PHASE_RESET        (1 << 0)
#define SGU1_FLAGS1_FILTER_PHASE_RESET (1 << 1)
#define SGU1_FLAGS1_PCM_LOOP           (1 << 2)
#define SGU1_FLAGS1_TIMER_SYNC         (1 << 3)
#define SGU1_FLAGS1_FREQ_SWEEP         (1 << 4)
#define SGU1_FLAGS1_VOL_SWEEP          (1 << 5)
#define SGU1_FLAGS1_CUT_SWEEP          (1 << 6)

// -----------------------------------------------------------------------------
// Notes on behavior (implementation-level, not register-level)
// - Operator envelope is AR -> DR toward SL, then SR while key held, then RR on key-off.
// - Envelope timing: SGU EG runs at 16kHz (48kHz/3), vs OPN/ESFM at ~17.7kHz.
//   This results in ~10% slower envelope timing compared to ESFM reference.
// - MOD is phase modulation gain from previous op; op0 uses MOD as feedback gain.
// - SYNC resets this op phase on previous op wrap; RING multiplies by previous op sign/value.
// - FIX mode ignores channel pitch and derives a fixed frequency from MUL+DT.
// -----------------------------------------------------------------------------

// Waveform types for operator R7[2:0] (all 8 waveforms implemented)
// - WAVE_SINE, WAVE_TRIANGLE: OPL-style wave modifiers via WPAR
// - WAVE_NOISE: 32-bit LFSR white noise, SID-compatible clocking (freq16 * 0.9537 Hz)
// - WAVE_PERIODIC_NOISE: Configurable 6-bit LFSR metallic/tonal noise
//     Frequency: channel freq16 × operator multiplier (R0[3:0])
//     Timbre: WPAR[1:0] selects tap configuration (per-operator)
// - WAVE_SAMPLE: PCM sample playback as FM operator waveform
//     Uses channel pcmrst register as base address for 1024-sample waveform
//     Phase (0-1023) indexes into sample region, looping naturally
typedef enum
{
    SGU_WAVE_SINE = 0,
    SGU_WAVE_TRIANGLE = 1,
    SGU_WAVE_SAWTOOTH = 2,
    SGU_WAVE_PULSE = 3,
    SGU_WAVE_NOISE = 4,
    SGU_WAVE_PERIODIC_NOISE = 5,
    SGU_WAVE_RESERVED6 = 6, // reserved for future use
    SGU_WAVE_SAMPLE = 7,    // sample from PCM memory as waveform
} sgu_waveform_t;

// WPAR[1:0] selects 6-bit LFSR tap configuration for PERIODIC_NOISE
typedef enum
{
    SGU_LFSR_TAP34 = 0,   // taps 3,4 (XOR) - simple periodic
    SGU_LFSR_TAP23 = 1,   // taps 2,3 (XOR) - simple periodic
    SGU_LFSR_TAP023 = 2,  // taps 0,2,3 (XOR) - intermediate
    SGU_LFSR_TAP0235 = 3, // taps 0,2,3,5 (XOR) - most complex/noisy
} sgu_lfsr_t;

// Envelope states
enum envelope_state : uint8_t
{
    SGU_EG_ATTACK = 0,
    SGU_EG_DECAY = 1,
    SGU_EG_SUSTAIN = 2,
    SGU_EG_RELEASE = 3,
    SGU_EG_STATES = 4
};

struct SGU_OP
{
    // R0: [7]TRM [6]VIB [5:4]KSR [3:0]MUL
    // - TRM/VIB enable LFO AM/PM (depth set in R6).
    // - KSR selects rate-scaling strength (2-bit).
    // - MUL is OPL-style multiplier (0 => 0.5×, 1..15 => 1×..15×).
    uint8_t reg0;

    // R1: [7:6]KSL [5:0]TL
    // - TL is output attenuation in 0.75 dB steps.
    // - KSL scales attenuation by pitch (OPL-style).
    uint8_t reg1;

    // R2: [7:4]AR_lo4 [3:0]DR_lo4
    // R3: [7:4]SL [3:0]RR
    // R4: [7:5]DT [4:0]SR
    // ADSR envelope:
    // Attack, Decay, Sustain Level and Rate, Release Rate
    // - AR/DR/SR are 5-bit values using R7 msbs; SL/RR are 4-bit.
    // - DT selects detune table entry (signed adjustment).
    uint8_t reg2;
    uint8_t reg3;
    uint8_t reg4;

    // R5: [7:5]DELAY [4]FIX [3:0]WPAR
    // - DELAY adds 2**(DELAY + 8) samples to key-on (phase + envelope).
    // - FIX selects fixed-frequency mode.
    // - WPAR is a per-waveform shape parameter.
    uint8_t reg5;

    // R6: [7]TRMD [6]VIBD [5]SYNC [4]RING [3:1]MOD [0]TL_msb
    // - TRMD/VIBD set LFO depth.
    // - SYNC hard-syncs to previous operator wrap.
    // - RING multiplies by previous operator output.
    // - MOD sets phase modulation depth (6 dB steps); op0 uses feedback.
    // - TL_msb extends total level to 7 bits.
    uint8_t reg6;

    // R7: [7:5]OUT [4]AR_msb [3]DR_msb [2:0]WAVE
    // - AR/DR msb extend envelope rates to 5 bits.
    // - OUT sets direct mix level (6 dB steps).
    // - WAVE selects waveform (0..5 implemented).
    uint8_t reg7;
};

struct SGU_CH
{
    struct SGU_OP op[SGU_OP_PER_CH];

    uint16_t freq; // 16-bit phase increment / playback rate
    int8_t vol;    // signed 8-bit; sign allows inversion.
    int8_t pan;    // positive Right, negative Left

    // flags0:
    //  - bit 0: (GATE) ADSR envelope is running when set; key-on/key-off,
    //    rising edge is starting the envelope generator and resetting the signal phase.
    //  - bit 3: PCM enable (when set, src = pcm[pcmpos])
    //  - bit 4: ring mod enable (multiply by next channel's raw sample)
    //  - bits 5..7: filter mode selects (LP/HP/BP) (implemented as bitmask picks)
    uint8_t flags0;

    // flags1:
    //  - bit 0: one-shot phase reset request (handled at end of channel processing)
    //  - bit 2: PCM loop enable
    //  - bit 3: timer sync enable (enables restimer-based periodic phase reset)
    //  - bit 4: freq sweep enable
    //  - bit 5: vol sweep enable
    //  - bit 6: cutoff sweep enable
    uint8_t flags1;

    // cutoff: filter cutoff control (scaled to ff inside filter section).
    uint16_t cutoff;

    // duty[6:0]: pulse width for WAVE_PULSE (0..127), where low = duty steps, high = 128 - duty
    uint8_t duty;

    // reson: resonance amount (0..255). Used as (256 - reson) feedback term.
    uint8_t reson;

    // PCM playback pointers.
    uint16_t pcmpos; // current sample position
    uint16_t pcmbnd; // boundary/end position
    uint16_t pcmrst; // loop restart position

    // Sweep parameter blocks:
    // speed: period in "ticks" (same domain as Pm) between sweep steps
    // amt:   step amount + direction/mode bits (interpretation differs by sweep type)
    // bound: limit value (coarse, often compared against high byte of freq/cutoff)
    struct
    {
        uint16_t speed;
        uint8_t amt;
        uint8_t bound;
    } swfreq;
    struct
    {
        uint16_t speed;
        uint8_t amt;
        uint8_t bound;
    } swvol;
    struct
    {
        uint16_t speed;
        uint8_t amt;
        uint8_t bound;
    } swcut;

    // restimer: period for periodic phase reset when SGU_FLAGS1_TIMER_SYNC is set.
    uint16_t restimer;

    // ### Used for implementation specific purposes.
    // Default function in X65 deployment special2 changes the channel mapped into
    // CPU memory space, which consists of 64 registers only
    // and would not fit all channels at once.
    // Channel FFh is special, as it maps service registers into memory space.
    // Chip identifier, UniqueID and mixer/DSP controls.
    uint8_t special1;
    uint8_t special2;
};

// Per-operator state, packed for cache locality (20 bytes per operator)
struct sgu_op_state
{
    uint32_t phase;                     // current phase value (10.22 format)
    int16_t value;                      // current operator value
    uint16_t envelope_attenuation;      // computed envelope attenuation (4.6 format)
    uint16_t eg_delay_counter;          // delay counter (samples)
    uint16_t blep_frac;                 // fractional phase at edge (for sub-sample interpolation)
    int16_t blep_prev_sample;           // previous raw sample for edge detection
    enum envelope_state envelope_state; // current envelope state
    uint8_t blep;                       // BLEP damping after dramatic phase changes
    uint32_t lfsr_state;                // per-operator noise LFSR state
};

// op_flags packed boolean bit groups (4 bits each, one per operator)
#define OP_FLAGS_PHASE_WRAP 0  // bits 0-3:   phase wrap flag (for SYNC)
#define OP_FLAGS_KEY_STATE  4  // bits 4-7:   current key state
#define OP_FLAGS_KEYON_GATE 8  // bits 8-11:  last raw key state (edge detect)
#define OP_FLAGS_EG_DELAY   12 // bits 12-15: envelope delay active

#define OP_FLAG_GET(flags, group, op) (((flags) >> ((group) + (op))) & 1u)
#define OP_FLAG_SET(flags, group, op) ((flags) |= (1u << ((group) + (op))))
#define OP_FLAG_CLR(flags, group, op) ((flags) &= ~(1u << ((group) + (op))))

struct SGU
{

    // ========================================================================
    // HOT RUNTIME STATE (offset 0..~2KB from struct base)
    // Placed first for efficient ARM Thumb-2 addressing (12-bit immediate offsets).
    // ========================================================================

    // ------ private generator state ------
    uint32_t sample_counter;   // sample clock ticks
    uint32_t envelope_counter; // envelope counter; low 2 bits are sub-counter

    // internal state - global LFO
    uint16_t lfo_am_counter; // LFO AM counter
    uint16_t lfo_pm_counter; // LFO PM counter
    uint8_t lfo_am;          // current LFO AM value

    // channels internal state
    struct sgu_ch_state
    {
        int16_t op0_fb;                        // feedback memory for first operator
        uint16_t op_flags;                     // packed boolean flags (see OP_FLAGS_*)
        struct sgu_op_state op[SGU_OP_PER_CH]; // per-operator state (AoS layout)
    } m_channel[SGU_CHNS];

    // Cached per-sample globals (written by Setup, read by both cores)
    int32_t cached_lfo_raw_pm;
    bool cached_env_tick;
    uint32_t cached_env_counter_tick;

    // src[i] = raw oscillator sample for channel i (16-bit, used for ring mod).
    // post[i] = processed sample after volume/filter (higher precision int32).
    int16_t src[SGU_CHNS];
    int32_t post[SGU_CHNS];

    // Per-channel stereo contributions after pan (still int32).
    int32_t outL[SGU_CHNS];
    int32_t outR[SGU_CHNS];

    // Last mixed output sample (left/right), also returned by NextSample().
    int32_t L, R;
    int64_t L_in, R_in, L_q16, R_q16; // used for high-pass filtering

    // ------ SID-like channel processing state (post-FM) ------

    // State-variable filter state per channel
    int32_t svf_low[SGU_CHNS];
    int32_t svf_high[SGU_CHNS];
    int32_t svf_band[SGU_CHNS];

    // Sweep countdown timers (decrement each sample, trigger when <= 0)
    int32_t vol_sweep_countdown[SGU_CHNS];
    int32_t freq_sweep_countdown[SGU_CHNS];
    int32_t cut_sweep_countdown[SGU_CHNS];

    // Phase reset countdown for timer sync
    int32_t phase_reset_countdown[SGU_CHNS];

    // PCM phase accumulator for fractional PCM playback
    int32_t pcm_phase_accum[SGU_CHNS];

    // PCM sample memory (signed 8-bit)
    int8_t *pcm;

    // Per-channel mute (software-side, not part of chip spec).
    bool muted[SGU_CHNS];

    // ========================================================================
    // REGISTER MAP (must be contiguous for SGU_Write)
    // SGU_Write does: ((uint8_t *)sgu->chan)[addr13] = data;
    // ========================================================================
    struct SGU_CH __attribute__((aligned(4))) chan[SGU_CHNS];
};

// -----------------------------------------------------------------------------

void SGU_Init(struct SGU *sgu, size_t sampleMemSize);
void SGU_Reset(struct SGU *sgu);

void SGU_Write(struct SGU *sgu, uint16_t addr13, uint8_t data);

void SGU_NextSample(struct SGU *sgu, int32_t *l, int32_t *r);

// Split API for dual-core rendering:
// Setup: update global state (LFO, envelope). Call once per sample.
void SGU_NextSample_Setup(struct SGU *sgu);
// Channels: process channels [ch_start, ch_end). Accumulates into *l, *r.
void SGU_NextSample_Channels(struct SGU *sgu, unsigned ch_start, unsigned ch_end,
                             int32_t *l, int32_t *r);
// Finalize: DC-removal HPF and output clamping.
void SGU_NextSample_Finalize(struct SGU *sgu, int64_t L, int64_t R,
                             int32_t *l, int32_t *r);

// Convenience getter: returns mono downmix of current per-channel post-pan samples (averaged).
// This is not used in NextSample, but useful for taps/meters/debug.
int32_t SGU_GetSample(struct SGU *sgu, uint8_t ch);
