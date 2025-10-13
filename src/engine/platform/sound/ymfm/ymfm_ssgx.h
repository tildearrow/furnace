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

#ifndef YMFM_SSGX_H
#define YMFM_SSGX_H

#pragma once

#include "ymfm.h"

namespace ymfm
{

//*********************************************************
//  OVERRIDE INTERFACE
//*********************************************************

// ======================> ssgx_override

// this class describes a simple interface to allow the internal SSG to be
// overridden with another implementation
class ssgx_override
{
public:
	// reset our status
	virtual void ssgx_reset() = 0;

	// read/write to the SSG registers
	virtual uint8_t ssgx_read(uint32_t regnum) = 0;
	virtual void ssgx_write(uint32_t regnum, uint8_t data) = 0;

	// notification when the prescale has changed
	virtual void ssgx_prescale_changed() = 0;
};


//*********************************************************
//  REGISTER CLASS
//*********************************************************

// ======================> ssgx_registers

//
// SSGX register map:
//
// Emulation mode:
//      System-wide registers:
//           06 ---xxxxx Noise period
//           07 --x----- Noise enable(0) or disable(1) for channel C
//              ---x---- Noise enable(0) or disable(1) for channel B
//              ----x--- Noise enable(0) or disable(1) for channel A
//              -----x-- Tone enable(0) or disable(1) for channel C
//              ------x- Tone enable(0) or disable(1) for channel B
//              -------x Tone enable(0) or disable(1) for channel A
//           0B xxxxxxxx Envelope period fine
//           0C xxxxxxxx Envelope period coarse
//           0D ----x--- Envelope shape: continue
//              -----x-- Envelope shape: attack/decay
//              ------x- Envelope shape: alternate
//              -------x Envelope shape: hold
//           0F ------x- Register bank
//              -------x Emulation mode(0) or Native mode(1)
//           19 xxxxxxxx Noise AND mask
//           1A xxxxxxxx Noise OR mask
//
//      Per-channel registers:
//     00,02,04 xxxxxxxx Tone period (fine) for channel A,B,C
//     01,03,05 ----xxxx Tone period (coarse) for channel A,B,C
//     08,09,0A ---x---- Mode: fixed(0) or variable(1) for channel A,B,C
//              ----xxxx Amplitude for channel A,B,C
//
// Native mode:
//      System-wide registers:
//           07 --x----- Noise enable(0) or disable(1) for channel C
//              ---x---- Noise enable(0) or disable(1) for channel B
//              ----x--- Noise enable(0) or disable(1) for channel A
//              -----x-- Tone enable(0) or disable(1) for channel C
//              ------x- Tone enable(0) or disable(1) for channel B
//              -------x Tone enable(0) or disable(1) for channel A
//           0F ------x- Register bank
//              -------x Emulation mode(0) or Native mode(1)
//
//      Per-channel registers:
//     00,02,04 xxxxxxxx Tone period (fine) for channel A,B,C
//     01,03,05 xxxxxxxx Tone period (coarse) for channel A,B,C
//     08,09,0A x------- Right output disable for channel A,B,C
//              -x------ Left output disable for channel A,B,C
//              --x----- Mode: fixed(0) or variable(1) for channel A,B,C
//              ---xxxxx Amplitude for channel A,B,C
//     06,1B,1C xxxxxxxx Noise period for channel A,B,C
//     0B,10,12 xxxxxxxx Envelope period fine for channel A,B,C
//     0C,11,13 xxxxxxxx Envelope period coarse for channel A,B,C
//     0D,14,15 ----x--- Envelope shape: continue for channel A,B,C
//              -----x-- Envelope shape: attack/decay for channel A,B,C
//              ------x- Envelope shape: alternate for channel A,B,C
//              -------x Envelope shape: hold for channel A,B,C
//     0E,1D,1E -----xxx Envelope volume for channel A,B,C
//     16,17,18 ---xxxxx Pulse duty cycle for channel A,B,C
//
class ssgx_registers
{
public:
	// constants
	static constexpr uint32_t OUTPUTS = 6;
	static constexpr uint32_t CHANNELS = 3;
	static constexpr uint32_t REGISTERS = 0x20;
	static constexpr uint32_t ALL_CHANNELS = (1 << CHANNELS) - 1;

	// constructor
	ssgx_registers() { }

	// reset to initial state
	void reset();

	// save/restore
	void save_restore(ymfm_saved_state &state);

	// direct read/write access
	uint8_t read(uint32_t index) { return m_regdata[index]; }
	void write(uint32_t index, uint8_t data) { m_regdata[index] = data; }

	// system-wide registers
	uint32_t is_native_mode() const                     { return bitfield(m_regdata[0x0f], 0); }
	uint32_t register_bank() const                      { return bitfield(m_regdata[0x0f], 1); }
	uint32_t noise_and_mask() const                     { return m_regdata[0x19]; }
	uint32_t noise_or_mask() const                      { return m_regdata[0x1a]; }

	// per-channel registers
	uint32_t ch_noise_enable_n(uint32_t choffs) const     { return bitfield(m_regdata[0x07], 3 + choffs); }
	uint32_t ch_tone_enable_n(uint32_t choffs) const      { return bitfield(m_regdata[0x07], 0 + choffs); }
	uint32_t ch_tone_period(uint32_t choffs) const        { return m_regdata[0x00 + 2 * choffs] | (bitfield(m_regdata[0x01 + 2 * choffs], 0, is_native_mode() ? 8 : 4) << 8); }
	uint32_t ch_right_disable(uint32_t choffs) const      { return is_native_mode() ? bitfield(m_regdata[0x08 + choffs], 7) : 0; }
	uint32_t ch_left_disable(uint32_t choffs) const       { return is_native_mode() ? bitfield(m_regdata[0x08 + choffs], 6) : 0; }
	uint32_t ch_envelope_enable(uint32_t choffs) const    { return bitfield(m_regdata[0x08 + choffs], is_native_mode() ? 5 : 4); }
	uint32_t ch_amplitude(uint32_t choffs) const          { return bitfield(m_regdata[0x08 + choffs], 0, is_native_mode() ? 5 : 4); }
	uint32_t ch_noise_period(uint32_t choffs) const       { return bitfield(m_regdata[(is_native_mode() && (choffs >= 1)) ? (0x1b + (choffs - 1)) : 0x06], 0, is_native_mode() ? 8 : 5); }
	uint32_t ch_envelope_period(uint32_t choffs) const    { return m_regdata[(is_native_mode() && (choffs >= 1)) ? (0x10 + (2 * (choffs - 1))) : 0x0b] | (m_regdata[(is_native_mode() && (choffs >= 1)) ? (0x11 + (2 * (choffs - 1))) : 0x0c] << 8); }
	uint32_t ch_envelope_continue(uint32_t choffs) const  { return bitfield(m_regdata[(is_native_mode() && (choffs >= 1)) ? (0x14 + (choffs - 1)) : 0x0d], 3); }
	uint32_t ch_envelope_attack(uint32_t choffs) const    { return bitfield(m_regdata[(is_native_mode() && (choffs >= 1)) ? (0x14 + (choffs - 1)) : 0x0d], 2); }
	uint32_t ch_envelope_alternate(uint32_t choffs) const { return bitfield(m_regdata[(is_native_mode() && (choffs >= 1)) ? (0x14 + (choffs - 1)) : 0x0d], 1); }
	uint32_t ch_envelope_hold(uint32_t choffs) const      { return bitfield(m_regdata[(is_native_mode() && (choffs >= 1)) ? (0x14 + (choffs - 1)) : 0x0d], 0); }
	uint32_t ch_envelope_volume(uint32_t choffs) const    { return is_native_mode() ? bitfield(m_regdata[0x0e], 0, 3) : 0; }
	uint32_t ch_pulse_duty(uint32_t choffs) const         { return is_native_mode() ? bitfield(m_regdata[0x16 + choffs], 0, 5) : 0x10; }

private:
	// internal state
	uint8_t m_regdata[REGISTERS];         // register data
};


// ======================> ssgx_engine

class ssgx_engine
{
public:
	static constexpr int OUTPUTS = ssgx_registers::OUTPUTS;
	static constexpr int CHANNELS = ssgx_registers::CHANNELS;
	static constexpr int CLOCK_DIVIDER = 1;

	using output_data = ymfm_output<OUTPUTS>;

	// constructor
	ssgx_engine(ymfm_interface &intf);

	// configure an override
	void override(ssgx_override &override) { m_override = &override; }

	// reset our status
	void reset();

	// save/restore
	void save_restore(ymfm_saved_state &state);

	// master clocking function
	void clock(uint32_t tick = 1);

	// compute sum of channel outputs
	void output(output_data &output);

	// read/write to the SSG registers
	uint8_t read(uint32_t regnum);
	void write(uint32_t regnum, uint8_t data);

	// return a reference to our interface
	ymfm_interface &intf() { return m_intf; }

	// return a reference to our registers
	ssgx_registers &regs() { return m_regs; }

	// true if we are overridden
	bool overridden() const { return (m_override != nullptr); }

	// indicate the prescale has changed
	void prescale_changed() { if (m_override != nullptr) m_override->ssgx_prescale_changed(); }

	// get the last output
	void get_last_out(output_data& out) { out = m_last_out; }

private:
	// internal state
	struct tone_t
	{
		tone_t()
			: m_tone_count(0)
			, m_tone_state(0)
			, m_tone_duty(0)
		{
		}

		void reset()
		{
			m_tone_count = 0;
			m_tone_state = 0;
			m_tone_duty = 0;
		}

		uint32_t m_tone_count;               // current tone counter
		uint32_t m_tone_state;               // current tone state
		uint32_t m_tone_duty;                // current tone duty
	};
	struct envelope_t
	{
		envelope_t()
			: m_envelope_count(0)
			, m_envelope_state(0)
		{
		}

		void reset()
		{
			m_envelope_count = 0;
			m_envelope_state = 0;
		}

		uint32_t m_envelope_count;              // envelope counter
		uint32_t m_envelope_state;              // envelope state
	};
	struct noise_t
	{
		noise_t()
			: m_noise_count(0)
			, m_noise_state(0x1ffff)
			, m_noise_compare(0)
			, m_noise_output(0)
		{
		}

		void reset()
		{
			m_noise_count = 0;
			m_noise_state = 0x1ffff;
			m_noise_compare = 0;
			m_noise_output = 0;
		}

		uint32_t m_noise_count;                 // current noise counter
		uint32_t m_noise_state;                 // current noise state
		uint32_t m_noise_compare;               // current noise compare value
		uint32_t m_noise_output;                // current noise output
	};

	ymfm_interface &m_intf;                   // reference to the interface
	tone_t m_tone[3];
	envelope_t m_envelope[3];
	noise_t m_noise[3];                      
	ssgx_registers m_regs;                   // registers
	ssgx_override *m_override;               // override interface
	output_data m_last_out;
};

}

#endif // YMFM_SSGX_H
