/*
	License: BSD-3-Clause
	see https://gitlab.com/cam900/vgsound_emu/-/blob/V1/LICENSE for more details

	Copyright holder(s): cam900
	Contributor(s): Natt Akuma, James Alan Nguyen, Laurens Holst
	Modifiers and Contributors for Furnace: Natt Akuma, tildearrow, Grauw
	Konami SCC emulation core

	See scc.cpp for more info.
*/

#include <algorithm>
#include <memory>

#ifndef _VGSOUND_EMU_SCC_HPP
#define _VGSOUND_EMU_SCC_HPP

#pragma once

namespace scc
{
	typedef unsigned char       u8;
	typedef signed char         s8;
	typedef unsigned short     u16;
	typedef signed short       s16;
	typedef unsigned int       u32;
	typedef signed int         s32;

	// get bitfield, bitfield(input, position, len)
	template<typename T> T bitfield(T in, u8 pos, u8 len = 1)
	{
		return (in >> pos) & (len ? (T(1 << len) - 1) : 1);
	}
}

using namespace scc;
// shared for SCCs
class scc_core
{
public:
	// constructor
	scc_core()
		: m_voice{*this,*this,*this,*this,*this}
	{};
	virtual ~scc_core(){};

	// accessors
	virtual u8 scc_r(bool is_sccplus, u8 address) = 0;
	virtual void scc_w(bool is_sccplus, u8 address, u8 data) = 0;

	// internal state
	virtual void reset();
	void tick();

	// getters
	s32 out() { return m_out; } // output to DA0...DA10 pin
	s32 chan_out(u8 ch) { return m_voice[ch].out; }
	u8 reg(u8 address) { return m_reg[address]; }

protected:
	// voice structs
	struct voice_t
	{
		// constructor
		voice_t(scc_core &host) : m_host(host) {};

		// internal state
		void reset();
		void tick();

		// registers
		scc_core &m_host;
		s8 wave[32] = {0};   // internal waveform
		bool enable = false; // output enable flag
		u16 pitch = 0;       // pitch
		u8 volume = 0;       // volume
		u8 addr = 0;         // waveform pointer
		u16 counter = 0;     // frequency counter
		s32 out = 0;         // current output
	};
	voice_t m_voice[5];    // 5 voices

	// accessor
	u8 wave_r(bool is_sccplus, u8 address);
	void wave_w(bool is_sccplus, u8 address, u8 data);
	void freq_vol_enable_w(u8 address, u8 data);

	struct test_t
	{
		// constructor
		test_t()
			: freq_4bit(0)
			, freq_8bit(0)
			, resetpos(0)
			, rotate(0)
			, rotate4(0)
		{ };

		void reset()
		{
			freq_4bit = 0;
			freq_8bit = 0;
			resetpos = 0;
			rotate = 0;
			rotate4 = 0;
		}

		u8 freq_4bit : 1; // 4 bit frequency
		u8 freq_8bit : 1; // 8 bit frequency
		u8 resetpos  : 1; // reset counter after pitch writes
		u8 rotate    : 1; // rotate and write protect waveform for all channels
		u8 rotate4   : 1; // same as above but for channel 4 only
	};

	test_t m_test;         // test register
	s32 m_out = 0;         // output to DA0...10

	u8 m_reg[256] = {0};   // register pool
};

// SCC core
class k051649_scc_core : public scc_core
{
public:
	// accessors
	virtual u8 scc_r(bool is_sccplus, u8 address) override;
	virtual void scc_w(bool is_sccplus, u8 address, u8 data) override;
};

class k052539_scc_core : public k051649_scc_core
{
public:
	// accessors
	virtual u8 scc_r(bool is_sccplus, u8 address) override;
	virtual void scc_w(bool is_sccplus, u8 address, u8 data) override;
};

#endif
