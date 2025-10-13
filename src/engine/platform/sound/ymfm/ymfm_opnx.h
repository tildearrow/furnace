// BSD 3-Clause License
//
// Copyright (c) 2021, Aaron Giles
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef YMFM_OPNX_H
#define YMFM_OPNX_H

#pragma once

#include "ymfm.h"
#include "ymfm_adpcmx.h"
#include "ymfm_fm.h"
#include "ymfm_ssgx.h"

namespace ymfm
{

//*********************************************************
//  REGISTER CLASSES
//*********************************************************

// ======================> opnx_registers

//
// OPNX register map:
//
// Emulation mode:
//      System-wide registers:
//           21 xxxxxxxx Test register
//           22 ----x--- LFO enable
//              -----xxx LFO rate
//           24 xxxxxxxx Timer A value (upper 8 bits)
//           25 ------xx Timer A value (lower 2 bits)
//           26 xxxxxxxx Timer B value
//           27 xx------ CSM/Multi-frequency mode for channel #2
//              --x----- Reset timer B
//              ---x---- Reset timer A
//              ----x--- Enable timer B
//              -----x-- Enable timer A
//              ------x- Load timer B
//              -------x Load timer A
//           28 x------- Key on/off operator 4
//              -x------ Key on/off operator 3
//              --x----- Key on/off operator 2
//              ---x---- Key on/off operator 1
//              -----xxx Channel select
//           2f ------x- YM2610B mode
//              -------x Emulation mode(0) or Native mode(1)
//
//     Per-channel registers (channel in address bits 0-1)
//     Note that all these apply to address+100 as well on OPNA+
//        A0-A3 xxxxxxxx Frequency number lower 8 bits
//        A4-A7 --xxx--- Block (0-7)
//              -----xxx Frequency number upper 3 bits
//        B0-B3 --xxx--- Feedback level for operator 1 (0-7)
//              -----xxx Operator connection algorithm (0-7)
//        B4-B7 x------- Pan left
//              -x------ Pan right
//              --xx---- LFO AM shift (0-3)
//              -----xxx LFO PM depth (0-7)
//
//     Per-operator registers (channel in address bits 0-1, operator in bits 2-3)
//     Note that all these apply to address+100 as well on OPNA+
//        30-3F -xxx---- Detune value (0-7)
//              ----xxxx Multiple value (0-15)
//        40-4F -xxxxxxx Total level (0-127)
//        50-5F xx------ Key scale rate (0-3)
//              ---xxxxx Attack rate (0-31)
//        60-6F x------- LFO AM enable
//              ---xxxxx Decay rate (0-31)
//        70-7F ---xxxxx Sustain rate (0-31)
//        80-8F xxxx---- Sustain level (0-15)
//              ----xxxx Release rate (0-15)
//        90-9F ----x--- SSG-EG enable
//              -----xxx SSG-EG envelope (0-7)
//
//     Special multi-frequency registers (channel implicitly #2; operator in address bits 0-1)
//        A8-AB xxxxxxxx Frequency number lower 8 bits
//        AC-AF --xxx--- Block (0-7)
//              -----xxx Frequency number upper 3 bits
//
//     Internal (fake) registers:
//        B8-BB --xxxxxx Latched frequency number upper bits (from A4-A7)
//        BC-BF --xxxxxx Latched frequency number upper bits (from AC-AF)
//
// Native mode:
//      System-wide registers:
//           21 xxxxxxxx Test register
//           22 ----x--- Global LFO enable
//              -----xxx Global LFO rate
//           24 xxxxxxxx Timer A value (upper 8 bits)
//           25 ------xx Timer A value (lower 2 bits)
//           26 xxxxxxxx Timer B value
//           27 xx------ CSM/Multi-frequency mode for channel #2
//              --x----- Reset timer B
//              ---x---- Reset timer A
//              ----x--- Enable timer B
//              -----x-- Enable timer A
//              ------x- Load timer B
//              -------x Load timer A
//           28 x------- Key on/off operator 4
//              -x------ Key on/off operator 3
//              --x----- Key on/off operator 2
//              ---x---- Key on/off operator 1
//              -----xxx Channel select
//           2f ------x- YM2610B mode
//              -------x Emulation mode(0) or Native mode(1)
//
//     Per-channel registers (channel in address bits 0-1)
//     Note that all these apply to address+100 as well on OPNA+
//        A0-A3 xxxxxxxx Frequency number lower 8 bits
//        A4-A7 --xxx--- Block (0-7)
//              -----xxx Frequency number upper 3 bits
//        B0-B3 --xxx--- Feedback level for operator 1 (0-7)
//              -----xxx Operator connection algorithm (0-7)
//        B4-B7 x------- Pan left
//              -x------ Pan right
//              --xx---- Global LFO AM shift (0-3)
//              -----xxx Global LFO PM depth (0-7)
//        C0-C3 xxxxxxxx Per-channel LFO frequency
//        C4-C7 xxxxxxxx Per-channel PM LFO depth
//        C8-CB xxxxxxxx Per-channel AM LFO depth
//        CC-CF x------- Per-channel LFO sync
//              -xxx---- Per-channel LFO PM sensitivity
//              ----xx-- Per-channel LFO waveform
//              ------xx Per-channel LFO AM shift
//        D0-D3 xxxxxxxx Per-channel LFO noise frequency
//        E0-E3 xxxxxxxx Per-channel LFO #2 frequency
//        E4-E7 xxxxxxxx Per-channel PM LFO #2 depth
//        E8-EB xxxxxxxx Per-channel AM LFO #2 depth
//        EC-EF x------- Per-channel LFO #2 sync
//              -xxx---- Per-channel LFO #2 PM sensitivity
//              ----xx-- Per-channel LFO #2 waveform
//              ------xx Per-channel LFO #2 AM shift
//        F0-F3 xxxxxxxx Per-channel LFO #2 noise frequency
//
//     Per-operator registers (channel in address bits 0-1, operator in bits 2-3)
//     Note that all these apply to address+100 as well on OPNA+
//        30-3F -xxx---- Detune value (0-7)
//              ----xxxx Multiple value (0-15)
//        40-4F -xxxxxxx Total level (0-127)
//        50-5F xx------ Key scale rate (0-3)
//              ---xxxxx Attack rate (0-31)
//        60-6F x------- LFO AM enable
//              ---xxxxx Decay rate (0-31)
//        70-7F ---xxxxx Sustain rate (0-31)
//        80-8F xxxx---- Sustain level (0-15)
//              ----xxxx Release rate (0-15)
//        90-9F xxxx---- Oscillator waveform (0-15)
//              ----x--- SSG-EG enable
//              -----xxx SSG-EG envelope (0-7)
//
//     Special multi-frequency registers (channel implicitly #2; operator in address bits 0-1)
//        A8-AB xxxxxxxx Frequency number lower 8 bits
//        AC-AF --xxx--- Block (0-7)
//              -----xxx Frequency number upper 3 bits
//
//     Internal (fake) registers:
//        B8-BB --xxxxxx Latched frequency number upper bits (from A4-A7)
//        BC-BF --xxxxxx Latched frequency number upper bits (from AC-AF)
//

class opnx_registers : public fm_registers_base
{
	// LFO waveforms are 256 entries long
	static constexpr uint32_t LFO_WAVEFORM_LENGTH = 256;

public:
	// constants
	static constexpr uint32_t OUTPUTS = 2;
	static constexpr uint32_t CHANNELS = 8;
	static constexpr uint32_t ALL_CHANNELS = (1 << CHANNELS) - 1;
	static constexpr uint32_t OPERATORS = CHANNELS * 4;
	static constexpr uint32_t WAVEFORMS = 16;
	static constexpr uint32_t REGISTERS = 0x200;
	static constexpr uint32_t REG_MODE = 0x27;
	static constexpr uint32_t DEFAULT_PRESCALE = 18;
	static constexpr uint32_t EG_CLOCK_DIVIDER = 3;
	static constexpr bool EG_HAS_SSG = true;
	static constexpr bool MODULATOR_DELAY = false;
	static constexpr uint32_t CSM_TRIGGER_MASK = 1 << 2;
	static constexpr uint8_t STATUS_TIMERA = 0x01;
	static constexpr uint8_t STATUS_TIMERB = 0x02;
	static constexpr uint8_t STATUS_BUSY = 0x80;
	static constexpr uint8_t STATUS_IRQ = 0;

	// constructor
	opnx_registers();

	// reset to initial state
	void reset();

	// save/restore
	void save_restore(ymfm_saved_state &state);

	// map channel number to register offset
	static constexpr uint32_t channel_offset(uint32_t chnum)
	{
		assert(chnum < CHANNELS);
		return (chnum % 4) + 0x100 * (chnum / 4);
	}

	// map operator number to register offset
	static constexpr uint32_t operator_offset(uint32_t opnum)
	{
		assert(opnum < OPERATORS);
		return (opnum % 16) + 0x100 * (opnum / 16);
	}

	// return an array of operator indices for each channel
	struct operator_mapping { uint32_t chan[CHANNELS]; };
	void operator_map(operator_mapping &dest) const;

	// read a register value
	uint8_t read(uint16_t index) const { return m_regdata[index]; }

	// handle writes to the register array
	bool write(uint16_t index, uint8_t data, uint32_t &chan, uint32_t &opmask);

	// clock the noise and LFO, if present, returning LFO PM value
	int32_t clock_noise_and_lfo();

	// reset the LFO
	void reset_lfo() { m_lfo_counter = 0; }

	// return the AM offset from LFO for the given channel
	uint32_t lfo_am_offset(uint32_t choffs) const;

	// return LFO/noise states
	uint32_t noise_state() const { return 0; }

	// caching helpers
	void cache_operator_data(uint32_t choffs, uint32_t opoffs, opdata_cache &cache);

	// compute the phase step, given a PM value
	uint32_t compute_phase_step(uint32_t choffs, uint32_t opoffs, opdata_cache const &cache, int32_t lfo_raw_pm);

	// log a key-on event
	std::string log_keyon(uint32_t choffs, uint32_t opoffs);

	// system-wide registers
	uint32_t test() const                       { return byte(0x21, 0, 8); }
	uint32_t lfo_enable() const                 { return byte(0x22, 3, 1); }
	uint32_t lfo_rate() const                   { return byte(0x22, 0, 3); }
	uint32_t timer_a_value() const              { return word(0x24, 0, 8, 0x25, 0, 2); }
	uint32_t timer_b_value() const              { return byte(0x26, 0, 8); }
	uint32_t csm() const                        { return (byte(0x27, 6, 2) == 2); }
	uint32_t multi_freq() const                 { return (byte(0x27, 6, 2) != 0); }
	uint32_t reset_timer_b() const              { return byte(0x27, 5, 1); }
	uint32_t reset_timer_a() const              { return byte(0x27, 4, 1); }
	uint32_t enable_timer_b() const             { return byte(0x27, 3, 1); }
	uint32_t enable_timer_a() const             { return byte(0x27, 2, 1); }
	uint32_t load_timer_b() const               { return byte(0x27, 1, 1); }
	uint32_t load_timer_a() const               { return byte(0x27, 0, 1); }
	uint32_t is_ym2610b() const                 { return byte(0x2f, 1, 1); }
	uint32_t is_native_mode() const             { return byte(0x2f, 0, 1); }
	uint32_t multi_block_freq(uint32_t num) const    { return word(0xac, 0, 6, 0xa8, 0, 8, num); }

	// per-channel registers
	uint32_t ch_block_freq(uint32_t choffs) const    { return word(0xa4, 0, 6, 0xa0, 0, 8, choffs); }
	uint32_t ch_feedback(uint32_t choffs) const      { return byte(0xb0, 3, 3, choffs); }
	uint32_t ch_algorithm(uint32_t choffs) const     { return byte(0xb0, 0, 3, choffs); }
	uint32_t ch_output_any(uint32_t choffs) const    { return byte(0xb4, 6, 2, choffs); }
	uint32_t ch_output_0(uint32_t choffs) const      { return byte(0xb4, 7, 1, choffs); }
	uint32_t ch_output_1(uint32_t choffs) const      { return byte(0xb4, 6, 1, choffs); }
	uint32_t ch_output_2(uint32_t choffs) const      { return 0; }
	uint32_t ch_output_3(uint32_t choffs) const      { return 0; }
	uint32_t ch_lfo_am_sens(uint32_t choffs) const   { return byte(0xb4, 4, 2, choffs); }
	uint32_t ch_lfo_pm_sens(uint32_t choffs) const   { return byte(0xb4, 0, 3, choffs); }
	uint32_t ch_pclfo_rate(uint32_t choffs, uint32_t l) const     { return is_native_mode() ? byte(0xc0 + (l << 5), 0, 8, choffs) : 0; }
	uint32_t ch_pclfo_pm_depth(uint32_t choffs, uint32_t l) const { return is_native_mode() ? byte(0xc4 + (l << 5), 0, 8, choffs) : 0; }
	uint32_t ch_pclfo_am_depth(uint32_t choffs, uint32_t l) const { return is_native_mode() ? byte(0xc8 + (l << 5), 0, 8, choffs) : 0; }
	uint32_t ch_pclfo_sync(uint32_t choffs, uint32_t l) const     { return is_native_mode() ? byte(0xcc + (l << 5), 7, 1, choffs) : 0; }
	uint32_t ch_pclfo_pm_sens(uint32_t choffs, uint32_t l) const  { return is_native_mode() ? byte(0xcc + (l << 5), 4, 3, choffs) : 0; }
	uint32_t ch_pclfo_waveform(uint32_t choffs, uint32_t l) const { return is_native_mode() ? byte(0xcc + (l << 5), 2, 2, choffs) : 0; }
	uint32_t ch_pclfo_am_sens(uint32_t choffs, uint32_t l) const  { return is_native_mode() ? byte(0xcc + (l << 5), 0, 2, choffs) : 0; }
	uint32_t ch_pclfo_noise(uint32_t choffs, uint32_t l) const    { return is_native_mode() ? byte(0xd0 + (l << 5), 0, 8, choffs) : 0; }

	// per-operator registers
	uint32_t op_detune(uint32_t opoffs) const        { return byte(0x30, 4, 3, opoffs); }
	uint32_t op_multiple(uint32_t opoffs) const      { return byte(0x30, 0, 4, opoffs); }
	uint32_t op_total_level(uint32_t opoffs) const   { return byte(0x40, 0, 7, opoffs); }
	uint32_t op_ksr(uint32_t opoffs) const           { return byte(0x50, 6, 2, opoffs); }
	uint32_t op_attack_rate(uint32_t opoffs) const   { return byte(0x50, 0, 5, opoffs); }
	uint32_t op_decay_rate(uint32_t opoffs) const    { return byte(0x60, 0, 5, opoffs); }
	uint32_t op_lfo_am_enable(uint32_t opoffs) const { return byte(0x60, 7, 1, opoffs); }
	uint32_t op_sustain_rate(uint32_t opoffs) const  { return byte(0x70, 0, 5, opoffs); }
	uint32_t op_sustain_level(uint32_t opoffs) const { return byte(0x80, 4, 4, opoffs); }
	uint32_t op_release_rate(uint32_t opoffs) const  { return byte(0x80, 0, 4, opoffs); }
	uint32_t op_waveform(uint32_t opoffs) const      { return is_native_mode() ? byte(0x90, 4, 4, opoffs) : 0; }
	uint32_t op_ssg_eg_enable(uint32_t opoffs) const { return byte(0x90, 3, 1, opoffs); }
	uint32_t op_ssg_eg_mode(uint32_t opoffs) const   { return byte(0x90, 0, 3, opoffs); }

protected:
	// return a bitfield extracted from a byte
	uint32_t byte(uint32_t offset, uint32_t start, uint32_t count, uint32_t extra_offset = 0) const
	{
		return bitfield(m_regdata[offset + extra_offset], start, count);
	}

	// return a bitfield extracted from a pair of bytes, MSBs listed first
	uint32_t word(uint32_t offset1, uint32_t start1, uint32_t count1, uint32_t offset2, uint32_t start2, uint32_t count2, uint32_t extra_offset = 0) const
	{
		return (byte(offset1, start1, count1, extra_offset) << count2) | byte(offset2, start2, count2, extra_offset);
	}

	// internal state
	struct pclfo_t
	{
		struct pclfo_params_t
		{
			pclfo_params_t()
				: m_lfo_counter(0)
				, m_noise_lfsr(1)
				, m_noise_counter(0)
				, m_noise_state(0)
				, m_noise_lfo(0)
				, m_lfo_pm(0)
				, m_lfo_am(0)
				, m_noise_waveform{0}
			{
			}

			void reset()
			{
				m_lfo_counter = 0;
				m_noise_lfsr = 1;
				m_noise_counter = 0;
				m_noise_state = 0;
				m_noise_lfo = 0;
				m_lfo_pm = 0;
				m_lfo_am = 0;
			}

			uint32_t m_lfo_counter;            // LFO counter
			uint32_t m_noise_lfsr;                // noise LFSR state
			uint8_t m_noise_counter;              // noise counter
			uint8_t m_noise_state;                // latched noise state
			uint8_t m_noise_lfo;                  // latched LFO noise value
			uint8_t m_lfo_pm;                  // current LFO PM value
			uint8_t m_lfo_am;                  // current LFO AM value
			int16_t m_noise_waveform[LFO_WAVEFORM_LENGTH]; // noise waveform
		};

		pclfo_t()
			: m_params{pclfo_params_t(), pclfo_params_t()}
		{
		}

		void reset()
		{
			for (pclfo_params_t &param : m_params)
			{
				param.reset();
			}
		}

		pclfo_params_t m_params[2];
	};

	pclfo_t m_pclfo[CHANNELS];
	uint32_t m_lfo_counter;               // LFO counter
	uint8_t m_lfo_am;                     // current LFO AM value
	uint8_t m_regdata[REGISTERS];         // register data
	int16_t m_lfo_waveform[4][LFO_WAVEFORM_LENGTH]; // LFO waveforms; AM in low 8, PM in upper 8
	uint16_t m_waveform[WAVEFORMS][WAVEFORM_LENGTH]; // waveforms
};



//*********************************************************
//  OPN IMPLEMENTATION CLASSES
//*********************************************************

// A note about prescaling and sample rates.
//
// YM2203, YM2608, and YM2610 contain an onboard SSG (basically, a YM2149).
// In order to properly generate sound at fully fidelity, the output sample
// rate of the YM2149 must be input_clock / 8. This is much higher than the
// FM needs, but in the interest of keeping things simple, the OPN generate
// functions will output at the higher rate and just replicate the last FM
// sample as many times as needed.
//
// To make things even more complicated, the YM2203 and YM2608 allow for
// software-controlled prescaling, which affects the FM and SSG clocks in
// different ways. There are three settings: divide by 6/4 (FM/SSG); divide
// by 3/2; and divide by 2/1.
//
// Thus, the minimum output sample rate needed by each part of the chip
// varies with the prescale as follows:
//
//             ---- YM2203 -----    ---- YM2608 -----    ---- YM2610 -----
// Prescale    FM rate  SSG rate    FM rate  SSG rate    FM rate  SSG rate
//     6         /72      /16         /144     /32          /144    /32
//     3         /36      /8          /72      /16
//     2         /24      /4          /48      /8
//
// If we standardized on the fastest SSG rate, we'd end up with the following
// (ratios are output_samples:source_samples):
//
//             ---- YM2203 -----    ---- YM2608 -----    ---- YM2610 -----
//              rate = clock/4       rate = clock/8       rate = clock/16
// Prescale    FM rate  SSG rate    FM rate  SSG rate    FM rate  SSG rate
//     6         18:1     4:1         18:1     4:1          9:1    2:1
//     3          9:1     2:1          9:1     2:1
//     2          6:1     1:1          6:1     1:1
//
// However, that's a pretty big performance hit for minimal gain. Going to
// the other extreme, we could standardize on the fastest FM rate, but then
// at least one prescale case (3) requires the FM to be smeared across two
// output samples:
//
//             ---- YM2203 -----    ---- YM2608 -----    ---- YM2610 -----
//              rate = clock/24      rate = clock/48      rate = clock/144
// Prescale    FM rate  SSG rate    FM rate  SSG rate    FM rate  SSG rate
//     6          3:1     2:3          3:1     2:3          1:1    2:9
//     3        1.5:1     1:3        1.5:1     1:3
//     2          1:1     1:6          1:1     1:6
//
// Stepping back one factor of 2 addresses that issue:
//
//             ---- YM2203 -----    ---- YM2608 -----    ---- YM2610 -----
//              rate = clock/12      rate = clock/24      rate = clock/144
// Prescale    FM rate  SSG rate    FM rate  SSG rate    FM rate  SSG rate
//     6          6:1     4:3          6:1     4:3          1:1    2:9
//     3          3:1     2:3          3:1     2:3
//     2          2:1     1:3          2:1     1:3
//
// This gives us three levels of output fidelity:
//    OPN_FIDELITY_MAX -- highest sample rate, using fastest SSG rate
//    OPN_FIDELITY_MIN -- lowest sample rate, using fastest FM rate
//    OPN_FIDELITY_MED -- medium sample rate such that FM is never smeared
//
// At the maximum clocks for YM2203/YM2608 (4Mhz/8MHz), these rates will
// end up as:
//    OPN_FIDELITY_MAX = 1000kHz
//    OPN_FIDELITY_MIN =  166kHz
//    OPN_FIEDLITY_MED =  333kHz


// ======================> opnx_fidelity

enum opnx_fidelity : uint8_t
{
	OPNX_FIDELITY_MAX,
	OPNX_FIDELITY_MIN,
	OPNX_FIDELITY_MED,

	OPNX_FIDELITY_DEFAULT = OPNX_FIDELITY_MAX
};


// ======================> ssgx_resampler

template<typename OutputType, int FirstOutput, bool MixTo1>
class ssgx_resampler
{
private:
	// helper to add the last computed value to the sums, applying the given scale
	void add_last(int32_t &sum0, int32_t &sum1, int32_t &sum2, int32_t &sum3, int32_t &sum4, int32_t &sum5, int32_t scale = 1);

	// helper to clock a new value and then add it to the sums, applying the given scale
	void clock_and_add(int32_t &sum0, int32_t &sum1, int32_t &sum2, int32_t &sum3, int32_t &sum4, int32_t &sum5, int32_t scale = 1, uint32_t tick = 1);

	// helper to write the sums to the appropriate outputs, applying the given
	// divisor to the final result
	void write_to_output(OutputType *output, int32_t sum0, int32_t sum1, int32_t sum2, int32_t sum3, int32_t sum4, int32_t sum5, int32_t divisor = 1);

public:
	// constructor
	ssgx_resampler(ssgx_engine &ssg);

	// save/restore
	void save_restore(ymfm_saved_state &state);

	// get the current sample index
	uint32_t sampindex() const { return m_sampindex; }

	// configure the ratio
	void configure(uint32_t outsamples, uint32_t srcsamples);

	// resample
	void resample(OutputType *output, uint32_t numsamples)
	{
		(this->*m_resampler)(output, numsamples);
	}

private:
	// resample SSG output to the target at a rate of 1 SSG sample
	// to every n output samples
	template<int Multiplier>
	void resample_n_1(OutputType *output, uint32_t numsamples);

	// resample SSG output to the target at a rate of n SSG samples
	// to every 1 output sample
	template<int Divisor>
	void resample_1_n(OutputType *output, uint32_t numsamples);

	// resample SSG output to the target at a rate of 9 SSG samples
	// to every 2 output samples
	void resample_2_9(OutputType *output, uint32_t numsamples);

	// resample SSG output to the target at a rate of 3 SSG samples
	// to every 1 output sample
	void resample_1_3(OutputType *output, uint32_t numsamples);

	// resample SSG output to the target at a rate of 3 SSG samples
	// to every 2 output samples
	void resample_2_3(OutputType *output, uint32_t numsamples);

	// resample SSG output to the target at a rate of 3 SSG samples
	// to every 4 output samples
	void resample_4_3(OutputType *output, uint32_t numsamples);

	// no-op resampler
	void resample_nop(OutputType *output, uint32_t numsamples);

	// define a pointer type
	using resample_func = void (ssgx_resampler::*)(OutputType *output, uint32_t numsamples);

	// internal state
	ssgx_engine &m_ssgx;
	uint32_t m_sampindex;
	resample_func m_resampler;
	ssgx_engine::output_data m_last;
};


// ======================> ym2610x/ym2610b

class ym2610x
{
	static constexpr uint8_t EOS_FLAGS_MASK = 0xbf;

public:
	using fm_engine = fm_engine_base<opnx_registers>;
	static constexpr uint32_t FM_OUTPUTS = fm_engine::OUTPUTS;
	static constexpr uint32_t SSG_OUTPUTS = 2;
	static constexpr uint32_t OUTPUTS = FM_OUTPUTS + SSG_OUTPUTS;
	using output_data = ymfm_output<OUTPUTS>;

	// constructor
	ym2610x(ymfm_interface &intf);

	// configuration
	void ssgx_override(ssgx_override &intf) { m_ssgx.override(intf); }
	void set_fidelity(opnx_fidelity fidelity) { m_fidelity = fidelity; update_prescale(); }

	// reset
	void reset();

	// save/restore
	void save_restore(ymfm_saved_state &state);

	// pass-through helpers
	uint32_t sample_rate(uint32_t input_clock) const
	{
		switch (m_fidelity)
		{
			case OPNX_FIDELITY_MIN:	return input_clock / 576;
			case OPNX_FIDELITY_MED:	return input_clock / 576;
			default:
			case OPNX_FIDELITY_MAX:	return input_clock / 2;
		}
	}
	uint32_t ssgx_effective_clock(uint32_t input_clock) const { return input_clock / 2; }
	void invalidate_caches() { m_fm.invalidate_caches(); }

	// read access
	uint8_t read_status();
	uint8_t read_data();
	uint8_t read_status_hi();
	uint8_t read_data_hi();
	uint8_t read(uint32_t offset);

	// write access
	void write_address(uint8_t data);
	void write_data(uint8_t data);
	void write_address_hi(uint8_t data);
	void write_data_hi(uint8_t data);
	void write(uint32_t offset, uint8_t data);

	// generate one sample of sound
	void generate(output_data *output, uint32_t numsamples = 1);

	// get the engine
	fm_engine* debug_fm_engine() { return &m_fm; }
	ssgx_engine* debug_ssgx_engine() { return &m_ssgx; }
	adpcmx_a_engine* debug_adpcmx_a_engine() { return &m_adpcmx_a; }
	adpcmx_b_engine* debug_adpcmx_b_engine() { return &m_adpcmx_b; }

protected:
	// internal helpers
	void update_prescale();
	void clock_fm_and_adpcm();

	// internal state
	opnx_fidelity m_fidelity;            // configured fidelity
	uint16_t m_address;                 // address register
	uint16_t m_fm_samples_per_output;    // how many samples to repeat
	uint8_t m_eos_status;               // end-of-sample signals
	uint8_t m_flag_mask;                // flag mask control
	fm_engine::output_data m_last_fm;   // last FM output
	fm_engine m_fm;                     // core FM engine
	ssgx_engine m_ssgx;                   // core FM engine
	ssgx_resampler<output_data, 2, true> m_ssgx_resampler; // SSG resampler helper
	adpcmx_a_engine m_adpcmx_a;           // ADPCM-A engine
	adpcmx_b_engine m_adpcmx_b;           // ADPCM-B engine
};

}


#endif // YMFM_OPNX_H
