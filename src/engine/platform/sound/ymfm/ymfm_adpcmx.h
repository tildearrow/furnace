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

#ifndef YMFM_ADPCMX_H
#define YMFM_ADPCMX_H

#pragma once

#include "ymfm.h"

namespace ymfm
{

//*********************************************************
//  INTERFACE CLASSES
//*********************************************************

// forward declarations
class adpcmx_a_engine;
class adpcmx_b_engine;


// ======================> adpcmx_a_registers

//
// ADPCM-A register map:
//
// Emulation mode:
//      System-wide registers:
//           00 x------- Dump (disable=1) or keyon (0) control
//              --xxxxxx Mask of channels to dump or keyon
//           01 --xxxxxx Total level
//           02 xxxxxxxx Test register
//        08-0D x------- Pan left
//              -x------ Pan right
//              ---xxxxx Instrument level
//        10-15 xxxxxxxx Start address (bit 8-15)
//        18-1D xxxxxxxx Start address (bit 16-23)
//        20-25 xxxxxxxx End address (bit 8-15)
//        28-2D xxxxxxxx End address (bit 16-23)
//           2F ------x- Register bank
//              -------x Emulation mode(0) or Native mode(1)
//
// Native mode:
//      System-wide registers:
//           00 x------- Dump (disable=1) or keyon (0) control
//              --xxxxxx Mask of channels to dump or keyon
//           01 --xxxxxx Total level
//           02 xxxxxxxx Test register
//        08-0D x------- Pan left
//              -x------ Pan right
//              ---xxxxx Instrument level
//        10-15 xxxxxxxx Start address (bit 8-15)
//        18-1D xxxxxxxx Start address (bit 16-23)
//        20-25 xxxxxxxx End address (bit 8-15)
//        28-2D xxxxxxxx End address (bit 16-23)
//           2F ------x- Register bank
//              -------x Emulation mode(0) or Native mode(1)
//           30 --xxxxxx Mask of channels to loop
//        40-45 xxxxxxxx Start address (bit 24-31)
//        48-4D xxxxxxxx Start address (bit 0-7)
//        50-55 xxxxxxxx End address (bit 24-31)
//        58-5D xxxxxxxx End address (bit 0-7)
//
class adpcmx_a_registers
{
public:
	// constants
	static constexpr uint32_t OUTPUTS = 2;
	static constexpr uint32_t CHANNELS = 6;
	static constexpr uint32_t REGISTERS = 0x60;
	static constexpr uint32_t ALL_CHANNELS = (1 << CHANNELS) - 1;

	// constructor
	adpcmx_a_registers() { }

	// reset to initial state
	void reset();

	// save/restore
	void save_restore(ymfm_saved_state &state);

	// map channel number to register offset
	static constexpr uint32_t channel_offset(uint32_t chnum)
	{
		assert(chnum < CHANNELS);
		return chnum;
	}

	// direct read/write access
	void write(uint32_t index, uint8_t data) { m_regdata[index] = data; }

	// system-wide registers
	uint32_t is_native_mode() const                     { return bitfield(m_regdata[0x2f], 0); }
	uint32_t register_bank() const                      { return bitfield(m_regdata[0x2f], 1); }
	uint32_t dump() const                               { return bitfield(m_regdata[0x00], 7); }
	uint32_t dump_mask() const                          { return bitfield(m_regdata[0x00], 0, 6); }
	uint32_t total_level() const                        { return bitfield(m_regdata[0x01], 0, 6); }
	uint32_t test() const                               { return m_regdata[0x02]; }

	// per-channel registers
	uint32_t ch_loop(uint32_t choffs) const             { return is_native_mode() ? bitfield(m_regdata[0x30], choffs) : 0; }
	uint32_t ch_pan_left(uint32_t choffs) const         { return bitfield(m_regdata[choffs + 0x08], 7); }
	uint32_t ch_pan_right(uint32_t choffs) const        { return bitfield(m_regdata[choffs + 0x08], 6); }
	uint32_t ch_instrument_level(uint32_t choffs) const { return bitfield(m_regdata[choffs + 0x08], 0, 5); }
	uint64_t ch_start(uint32_t choffs) const            { return is_native_mode() ? (m_regdata[choffs + 0x48] | (m_regdata[choffs + 0x10] << 8) | (m_regdata[choffs + 0x18] << 16) | (m_regdata[choffs + 0x40] << 24)) : ((m_regdata[choffs + 0x10] << 8) | (m_regdata[choffs + 0x18] << 16)); }
	uint64_t ch_end(uint32_t choffs) const              { return is_native_mode() ? (m_regdata[choffs + 0x58] | (m_regdata[choffs + 0x20] << 8) | (m_regdata[choffs + 0x28] << 16) | (m_regdata[choffs + 0x50] << 24)) : ((m_regdata[choffs + 0x20] << 8) | (m_regdata[choffs + 0x28] << 16)); }

	// per-channel writes
	void write_start(uint32_t choffs, uint64_t address)
	{
		write(choffs + 0x10, address >> 8);
		write(choffs + 0x18, address >> 16);
		if (is_native_mode())
		{
			write(choffs + 0x40, address >> 24);
			write(choffs + 0x48, address);
		}
	}
	void write_end(uint32_t choffs, uint64_t address)
	{
		write(choffs + 0x20, address >> 8);
		write(choffs + 0x28, address >> 16);
		if (is_native_mode())
		{
			write(choffs + 0x50, address >> 24);
			write(choffs + 0x58, address);
		}
	}

private:
	// internal state
	uint8_t m_regdata[REGISTERS];         // register data
};


// ======================> adpcmx_a_channel

class adpcmx_a_channel
{
public:
	// constructor
	adpcmx_a_channel(adpcmx_a_engine &owner, uint32_t choffs);

	// reset the channel state
	void reset();

	// save/restore
	void save_restore(ymfm_saved_state &state);

	// signal key on/off
	void keyonoff(bool on);

	// master clockingfunction
	bool clock();

	// return the computed output value, with panning applied
	template<int NumOutputs>
	void output(ymfm_output<NumOutputs> &output);

	// return the last output
	int32_t get_last_out(int ch) { return m_last_out[ch]; }

private:
	// internal state
	uint32_t const m_choffs;              // channel offset
	uint32_t m_playing;                   // currently playing?
	uint32_t m_curnibble;                 // index of the current nibble
	uint32_t m_curbyte;                   // current byte of data
	uint64_t m_curaddress;                // current address
	int32_t m_accumulator;                // accumulator
	int32_t m_step_index;                 // index in the stepping table
	int32_t m_last_out[2];                 // last output
	adpcmx_a_registers &m_regs;            // reference to registers
	adpcmx_a_engine &m_owner;              // reference to our owner
};


// ======================> adpcmx_a_engine

class adpcmx_a_engine
{
public:
	static constexpr int CHANNELS = adpcmx_a_registers::CHANNELS;

	// constructor
	adpcmx_a_engine(ymfm_interface &intf);

	// reset our status
	void reset();

	// save/restore
	void save_restore(ymfm_saved_state &state);

	// master clocking function
	uint32_t clock(uint32_t chanmask);

	// compute sum of channel outputs
	template<int NumOutputs>
	void output(ymfm_output<NumOutputs> &output, uint32_t chanmask);

	// write to the ADPCM-A registers
	void write(uint32_t regnum, uint8_t data);

	// set the start/end address for a channel (for hardcoded YM2608 percussion)
	void set_start_end(uint8_t chnum, uint16_t start, uint16_t end)
	{
		uint32_t choffs = adpcmx_a_registers::channel_offset(chnum);
		m_regs.write_start(choffs, start);
		m_regs.write_end(choffs, end);
	}

	// return a reference to our interface
	ymfm_interface &intf() { return m_intf; }

	// return a reference to our registers
	adpcmx_a_registers &regs() { return m_regs; }

	// debug functions
	adpcmx_a_channel* debug_channel(uint32_t index) const { return m_channel[index].get(); }

private:
	// internal state
	ymfm_interface &m_intf;                                 // reference to the interface
	std::unique_ptr<adpcmx_a_channel> m_channel[CHANNELS]; // array of channels
	adpcmx_a_registers m_regs;                             // registers
};


// ======================> adpcmx_b_registers

//
// ADPCM-B register map:
//
// Emulation mode:
//      System-wide registers:
//           00 x------- Start of synthesis/analysis
//              ---x---- Repeat playback
//              -------x Reset
//           01 x------- Pan left
//              -x------ Pan right
//           02 xxxxxxxx Start address (bit 8-15)
//           03 xxxxxxxx Start address (bit 16-23)
//           04 xxxxxxxx End address (bit 8-15)
//           05 xxxxxxxx End address (bit 16-23)
//           09 xxxxxxxx Delta-N frequency scale (low)
//           0a xxxxxxxx Delta-N frequency scale (high)
//           0b xxxxxxxx Level control
//           0f ------x- Register bank
//              -------x Emulation mode(0) or Native mode(1)
//
// Native mode:
//      System-wide registers:
//           00 x------- Start of synthesis/analysis
//              ---x---- Repeat playback
//              -------x Reset
//           01 x------- Pan left
//              -x------ Pan right
//           02 xxxxxxxx Start address (bit 8-15)
//           03 xxxxxxxx Start address (bit 16-23)
//           04 xxxxxxxx End address (bit 8-15)
//           05 xxxxxxxx End address (bit 16-23)
//           06 xxxxxxxx Loop address (bit 8-15)
//           07 xxxxxxxx Loop address (bit 16-23)
//           09 xxxxxxxx Delta-N frequency scale (low)
//           0a xxxxxxxx Delta-N frequency scale (high)
//           0b xxxxxxxx Level control
//           0f ------x- Register bank
//              -------x Emulation mode(0) or Native mode(1)
//           12 xxxxxxxx Start address (bit 24-31)
//           13 xxxxxxxx Start address (bit 0-7)
//           14 xxxxxxxx End address (bit 24-31)
//           15 xxxxxxxx End address (bit 0-7)
//           16 xxxxxxxx Loop address (bit 24-31)
//           17 xxxxxxxx Loop address (bit 0-7)
//
class adpcmx_b_registers
{
public:
	// constants
	static constexpr uint32_t REGISTERS = 0x20;

	// constructor
	adpcmx_b_registers() { }

	// reset to initial state
	void reset();

	// save/restore
	void save_restore(ymfm_saved_state &state);

	// direct read/write access
	void write(uint32_t index, uint8_t data) { m_regdata[index] = data; }

	// system-wide registers
	uint32_t is_native_mode() const   { return bitfield(m_regdata[0x0f], 0); }
	uint32_t register_bank() const    { return bitfield(m_regdata[0x0f], 1); }
	uint32_t execute() const          { return bitfield(m_regdata[0x00], 7); }
	uint32_t repeat() const           { return bitfield(m_regdata[0x00], 4); }
	uint32_t resetflag() const        { return bitfield(m_regdata[0x00], 0); }
	uint32_t pan_left() const         { return bitfield(m_regdata[0x01], 7); }
	uint32_t pan_right() const        { return bitfield(m_regdata[0x01], 6); }
	uint64_t start() const            { return is_native_mode() ? (m_regdata[0x13] | (m_regdata[0x02] << 8) | (m_regdata[0x03] << 16) | (m_regdata[0x12] << 24)) : ((m_regdata[0x02] << 8) | (m_regdata[0x03] << 16)); }
	uint64_t end() const              { return is_native_mode() ? (m_regdata[0x15] | (m_regdata[0x04] << 8) | (m_regdata[0x05] << 16) | (m_regdata[0x14] << 24)) : ((m_regdata[0x04] << 8) | (m_regdata[0x05] << 16)); }
	uint64_t loopaddr() const         { return is_native_mode() ? (m_regdata[0x17] | (m_regdata[0x06] << 8) | (m_regdata[0x07] << 16) | (m_regdata[0x16] << 24)) : start(); }
	uint32_t delta_n() const          { return m_regdata[0x09] | (m_regdata[0x0a] << 8); }
	uint32_t level() const            { return m_regdata[0x0b]; }

private:
	// internal state
	uint8_t m_regdata[REGISTERS];         // register data
};


// ======================> adpcmx_b_channel

class adpcmx_b_channel
{
	static constexpr int32_t STEP_MIN = 127;
	static constexpr int32_t STEP_MAX = 24576;

public:
	static constexpr uint8_t STATUS_EOS = 0x01;
	static constexpr uint8_t STATUS_BRDY = 0x02;
	static constexpr uint8_t STATUS_PLAYING = 0x04;

	// constructor
	adpcmx_b_channel(adpcmx_b_engine &owner);

	// reset the channel state
	void reset();

	// save/restore
	void save_restore(ymfm_saved_state &state);

	// signal key on/off
	void keyonoff(bool on);

	// master clocking function
	void clock();

	// return the computed output value, with panning applied
	template<int NumOutputs>
	void output(ymfm_output<NumOutputs> &output, uint32_t rshift);

	// return the status register
	uint8_t status() const { return m_status; }

	// handle special register reads
	uint8_t read(uint32_t regnum);

	// handle special register writes
	void write(uint32_t regnum, uint8_t value);

	// return the last output
	int32_t get_last_out(int ch) { return m_last_out[ch]; }

private:
	// load the start address
	void load_start();

	// loop execute
	void loop_start();

	// end checker; stops at the last byte of the chunk described by address_shift()
	bool at_end() const { return (m_curaddress == (((m_regs.end() | (m_regs.is_native_mode() ? 0xff : 0)) + 1) - 1)); }

	// internal state
	uint32_t m_status;              // currently playing?
	uint32_t m_curnibble;           // index of the current nibble
	uint32_t m_loopnibble;          // index of the current nibble to loop point
	uint32_t m_curbyte;             // current byte of data
	uint32_t m_loopbyte;            // current byte of data to loop point
	uint32_t m_position;            // current fractional position
	uint64_t m_curaddress;          // current address
	uint64_t m_loopaddress;         // current address (for loop)
	int32_t m_accumulator;          // accumulator
	int32_t m_prev_accum;           // previous accumulator (for linear interp)
	int32_t m_adpcm_step;           // next forecast
	int32_t m_loop_accumulator;     // accumulator (for loop)
	int32_t m_loop_adpcm_step;      // next forecast (for loop)
	bool m_loop_point;              // loop pointer
	int32_t m_last_out[2];          // last output
	adpcmx_b_registers &m_regs;     // reference to registers
	adpcmx_b_engine &m_owner;       // reference to our owner
};


// ======================> adpcmx_b_engine

class adpcmx_b_engine
{
public:
	// constructor
	adpcmx_b_engine(ymfm_interface &intf);

	// reset our status
	void reset();

	// save/restore
	void save_restore(ymfm_saved_state &state);

	// master clocking function
	void clock();

	// compute sum of channel outputs
	template<int NumOutputs>
	void output(ymfm_output<NumOutputs> &output, uint32_t rshift);

  // get last output
  int32_t get_last_out(int ch) { return m_channel->get_last_out(ch); }

	// read from the ADPCM-B registers
	uint32_t read(uint32_t regnum) { return m_channel->read(regnum); }

	// write to the ADPCM-B registers
	void write(uint32_t regnum, uint8_t data);

	// status
	uint8_t status() const { return m_channel->status(); }

	// return a reference to our interface
	ymfm_interface &intf() { return m_intf; }

	// return a reference to our registers
	adpcmx_b_registers &regs() { return m_regs; }

private:
	// internal state
	ymfm_interface &m_intf;                     // reference to our interface
	std::unique_ptr<adpcmx_b_channel> m_channel; // channel pointer
	adpcmx_b_registers m_regs;                   // registers
};

}

#endif // YMFM_ADPCMX_H
