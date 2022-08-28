/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2021 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2004 Dag Lem <resid@nimrod.no>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#define WAVEFORMGENERATOR_CPP

#include "WaveformGenerator.h"

/*
 * This fixes tests
 *  SID/wb_testsuite/noise_writeback_check_8_to_C_old
 *  SID/wb_testsuite/noise_writeback_check_9_to_C_old
 *  SID/wb_testsuite/noise_writeback_check_A_to_C_old
 *  SID/wb_testsuite/noise_writeback_check_C_to_C_old
 *
 * but breaks SID/wf12nsr/wf12nsr
 *
 * needs more digging...
 */
//#define NO_WB_NOI_PUL

namespace reSIDfp
{

/**
 * Number of cycles after which the waveform output fades to 0 when setting
 * the waveform register to 0.
 * Values measured on warm chips (6581R3/R4 and 8580R5)
 * checking OSC3.
 * Times vary wildly with temperature and may differ
 * from chip to chip so the numbers here represent
 * only the big difference between the old and new models.
 *
 * See [VICE Bug #290](http://sourceforge.net/p/vice-emu/bugs/290/)
 * and [VICE Bug #1128](http://sourceforge.net/p/vice-emu/bugs/1128/)
 */
// ~95ms
const unsigned int FLOATING_OUTPUT_TTL_6581R3  =   54000;
const unsigned int FLOATING_OUTPUT_FADE_6581R3 =    1400;
// ~1s
//const unsigned int FLOATING_OUTPUT_TTL_6581R4  = 1000000;
// ~1s
const unsigned int FLOATING_OUTPUT_TTL_8580R5  =  800000;
const unsigned int FLOATING_OUTPUT_FADE_8580R5 =   50000;

/**
 * Number of cycles after which the shift register is reset
 * when the test bit is set.
 * Values measured on warm chips (6581R3/R4 and 8580R5)
 * checking OSC3.
 * Times vary wildly with temperature and may differ
 * from chip to chip so the numbers here represent
 * only the big difference between the old and new models.
 */
// ~210ms
const unsigned int SHIFT_REGISTER_RESET_6581R3 =   50000;
const unsigned int SHIFT_REGISTER_FADE_6581R3  =   15000;
// ~2.15s
//const unsigned int SHIFT_REGISTER_RESET_6581R4 = 2150000;
// ~2.8s
const unsigned int SHIFT_REGISTER_RESET_8580R5 =  986000;
const unsigned int SHIFT_REGISTER_FADE_8580R5  =  314300;

/*
 * This is what happens when the lfsr is clocked:
 *
 * cycle 0: bit 19 of the accumulator goes from low to high, the noise register acts normally,
 *          the output may overwrite a bit;
 *
 * cycle 1: first phase of the shift, the bits are interconnected and the output of each bit
 *          is latched into the following. The output may overwrite the latched value.
 *
 * cycle 2: second phase of the shift, the latched value becomes active in the first
 *          half of the clock and from the second half the register returns to normal operation.
 *
 * When the test or reset lines are active the first phase is executed at every cyle
 * until the signal is released triggering the second phase.
 */
void WaveformGenerator::clock_shift_register(unsigned int bit0)
{
    shift_register = (shift_register >> 1) | bit0;

    // New noise waveform output.
    set_noise_output();
}

unsigned int WaveformGenerator::get_noise_writeback()
{
  return
    ~(
        (1 <<  2) |  // Bit 20
        (1 <<  4) |  // Bit 18
        (1 <<  8) |  // Bit 14
        (1 << 11) |  // Bit 11
        (1 << 13) |  // Bit  9
        (1 << 17) |  // Bit  5
        (1 << 20) |  // Bit  2
        (1 << 22)    // Bit  0
    ) |
    ((waveform_output & (1 << 11)) >>  9) |  // Bit 11 -> bit 20
    ((waveform_output & (1 << 10)) >>  6) |  // Bit 10 -> bit 18
    ((waveform_output & (1 <<  9)) >>  1) |  // Bit  9 -> bit 14
    ((waveform_output & (1 <<  8)) <<  3) |  // Bit  8 -> bit 11
    ((waveform_output & (1 <<  7)) <<  6) |  // Bit  7 -> bit  9
    ((waveform_output & (1 <<  6)) << 11) |  // Bit  6 -> bit  5
    ((waveform_output & (1 <<  5)) << 15) |  // Bit  5 -> bit  2
    ((waveform_output & (1 <<  4)) << 18);   // Bit  4 -> bit  0
}

void WaveformGenerator::write_shift_register()
{
    if (unlikely(waveform > 0x8) && likely(!test) && likely(shift_pipeline != 1))
    {
        // Write changes to the shift register output caused by combined waveforms
        // back into the shift register. This happens only when the register is clocked
        // (see $D1+$81_wave_test [1]) or when the test bit is falling.
        // A bit once set to zero cannot be changed, hence the and'ing.
        //
        // [1] ftp://ftp.untergrund.net/users/nata/sid_test/$D1+$81_wave_test.7z
        //
        // FIXME: Write test program to check the effect of 1 bits and whether
        // neighboring bits are affected.

#ifdef NO_WB_NOI_PUL
        if (waveform == 0xc)
            return;
#endif
        shift_register &= get_noise_writeback();

        noise_output &= waveform_output;
        set_no_noise_or_noise_output();
    }
}

void WaveformGenerator::set_noise_output()
{
    noise_output =
        ((shift_register & (1 <<  2)) <<  9) |  // Bit 20 -> bit 11
        ((shift_register & (1 <<  4)) <<  6) |  // Bit 18 -> bit 10
        ((shift_register & (1 <<  8)) <<  1) |  // Bit 14 -> bit  9
        ((shift_register & (1 << 11)) >>  3) |  // Bit 11 -> bit  8
        ((shift_register & (1 << 13)) >>  6) |  // Bit  9 -> bit  7
        ((shift_register & (1 << 17)) >> 11) |  // Bit  5 -> bit  6
        ((shift_register & (1 << 20)) >> 15) |  // Bit  2 -> bit  5
        ((shift_register & (1 << 22)) >> 18);   // Bit  0 -> bit  4

    set_no_noise_or_noise_output();
}

void WaveformGenerator::setWaveformModels(matrix_t* models)
{
    model_wave = models;
}

void WaveformGenerator::synchronize(WaveformGenerator* syncDest, const WaveformGenerator* syncSource) const
{
    // A special case occurs when a sync source is synced itself on the same
    // cycle as when its MSB is set high. In this case the destination will
    // not be synced. This has been verified by sampling OSC3.
    if (unlikely(msb_rising) && syncDest->sync && !(sync && syncSource->msb_rising))
    {
        syncDest->accumulator = 0;
    }
}

bool do_pre_writeback(unsigned int waveform_prev, unsigned int waveform, bool is6581)
{
    // no writeback without combined waveforms
    if (likely(waveform_prev <= 0x8))
        return false;
    // no writeback when changing to noise
    if (waveform == 8)
        return false;
    // What's happening here?
    if (is6581 &&
            ((((waveform_prev & 0x3) == 0x1) && ((waveform & 0x3) == 0x2))
            || (((waveform_prev & 0x3) == 0x2) && ((waveform & 0x3) == 0x1))))
        return false;
    if (waveform_prev == 0xc)
    {
        if (is6581)
            return false;
        else if ((waveform != 0x9) && (waveform != 0xe))
            return false;
    }
#ifdef NO_WB_NOI_PUL
    if (waveform == 0xc)
        return false;
#endif
    // ok do the writeback
    return true;
}

/*
 * When noise and pulse are combined all the bits are
 * connected and the four lower ones are grounded.
 * This causes the adjacent bits to be pulled down,
 * with different strength depending on model.
 *
 * This is just a rough attempt at modelling the effect.
 */
 
static unsigned int noise_pulse6581(unsigned int noise)
{
    return (noise < 0xf00) ? 0x000 : noise & (noise << 1) & (noise << 2);
}

static unsigned int noise_pulse8580(unsigned int noise)
{
    return (noise < 0xfc0) ? noise & (noise << 1) : 0xfc0;
}

void WaveformGenerator::set_no_noise_or_noise_output()
{
    no_noise_or_noise_output = no_noise | noise_output;

    // pulse+noise
    if (unlikely((waveform & 0xc) == 0xc))
        no_noise_or_noise_output = is6581
            ? noise_pulse6581(no_noise_or_noise_output)
            : noise_pulse8580(no_noise_or_noise_output);

}

void WaveformGenerator::writeCONTROL_REG(unsigned char control)
{
    const unsigned int waveform_prev = waveform;
    const bool test_prev = test;

    waveform = (control >> 4) & 0x0f;
    test = (control & 0x08) != 0;
    sync = (control & 0x02) != 0;

    // Substitution of accumulator MSB when sawtooth = 0, ring_mod = 1.
    ring_msb_mask = ((~control >> 5) & (control >> 2) & 0x1) << 23;

    if (waveform != waveform_prev)
    {
        // Set up waveform table.
        wave = (*model_wave)[waveform & 0x7];

        // no_noise and no_pulse are used in set_waveform_output() as bitmasks to
        // only let the noise or pulse influence the output when the noise or pulse
        // waveforms are selected.
        no_noise = (waveform & 0x8) != 0 ? 0x000 : 0xfff;
        set_no_noise_or_noise_output();
        no_pulse = (waveform & 0x4) != 0 ? 0x000 : 0xfff;

        if (waveform == 0)
        {
            // Change to floating DAC input.
            // Reset fading time for floating DAC input.
            floating_output_ttl = is6581 ? FLOATING_OUTPUT_TTL_6581R3 : FLOATING_OUTPUT_TTL_8580R5;
        }
    }

    if (test != test_prev)
    {
        if (test)
        {
            // Reset accumulator.
            accumulator = 0;

            // Flush shift pipeline.
            shift_pipeline = 0;

            // Set reset time for shift register.
            shift_register_reset = is6581 ? SHIFT_REGISTER_RESET_6581R3 : SHIFT_REGISTER_RESET_8580R5;
        }
        else
        {
            // When the test bit is falling, the second phase of the shift is
            // completed by enabling SRAM write.

            // During first phase of the shift the bits are interconnected
            // and the output of each bit is latched into the following.
            // The output may overwrite the latched value.
            if (do_pre_writeback(waveform_prev, waveform, is6581))
            {
                shift_register &= get_noise_writeback();
            }

            // bit0 = (bit22 | test) ^ bit17 = 1 ^ bit17 = ~bit17
            clock_shift_register((~shift_register << 17) & (1 << 22));
        }
    }
}

void WaveformGenerator::waveBitfade()
{
    waveform_output &= waveform_output >> 1;
    osc3 = waveform_output;
    if (waveform_output != 0)
        floating_output_ttl = is6581 ? FLOATING_OUTPUT_FADE_6581R3 : FLOATING_OUTPUT_FADE_8580R5;
}

void WaveformGenerator::shiftregBitfade()
{
    shift_register |= shift_register >> 1;
    shift_register |= 0x400000;
    if (shift_register != 0x7fffff)
        shift_register_reset = is6581 ? SHIFT_REGISTER_FADE_6581R3 : SHIFT_REGISTER_FADE_8580R5;
}

void WaveformGenerator::reset()
{
    // accumulator is not changed on reset
    freq = 0;
    pw = 0;

    msb_rising = false;

    waveform = 0;
    osc3 = 0;

    test = false;
    sync = false;

    wave = model_wave ? (*model_wave)[0] : nullptr;

    ring_msb_mask = 0;
    no_noise = 0xfff;
    no_pulse = 0xfff;
    pulse_output = 0xfff;

    shift_register_reset = 0;
    shift_register = 0x7fffff;
    // when reset is released the shift register is clocked once
    // so the lower bit is zeroed out
    // bit0 = (bit22 | test) ^ bit17 = 1 ^ 1 = 0
    clock_shift_register(0);

    shift_pipeline = 0;

    waveform_output = 0;
    floating_output_ttl = 0;
}

} // namespace reSIDfp
