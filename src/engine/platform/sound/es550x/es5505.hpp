/*
	License: BSD-3-Clause
	see https://github.com/cam900/vgsound_emu/LICENSE for more details

	Copyright holder(s): cam900
	Ensoniq ES5504 emulation core

	See es550x.cpp for more info
*/

#include "es550x.hpp"

#ifndef _VGSOUND_EMU_ES5505_HPP
#define _VGSOUND_EMU_ES5505_HPP

#pragma once

// ES5505 specific
class es5505_core : public es550x_shared_core
{
public:
	// constructor
	es5505_core(es550x_intf &intf)
		: es550x_shared_core(intf)
		, m_voice{*this,*this,*this,*this,*this,*this,*this,*this,
		          *this,*this,*this,*this,*this,*this,*this,*this,
		          *this,*this,*this,*this,*this,*this,*this,*this,
		          *this,*this,*this,*this,*this,*this,*this,*this}
	{
	}
	// host interface
	u16 host_r(u8 address);
	void host_w(u8 address, u16 data);

	// internal state
	virtual void reset() override;
	virtual void tick() override;

	// less cycle accurate, but also less cpu heavy update routine
	void tick_perf();

	// clock outputs
	bool bclk() { return m_bclk.current_edge(); }
	bool bclk_rising_edge() { return m_bclk.rising_edge(); }
	bool bclk_falling_edge() { return m_bclk.falling_edge(); }

	// Input mode for Channel 3
	void lin(s32 in) { if (m_sermode.adc) { m_ch[3].m_left = in; } }
	void rin(s32 in) { if (m_sermode.adc) { m_ch[3].m_right = in; } }

	// 4 stereo output channels
	s32 lout(u8 ch) { return m_ch[ch & 0x3].m_left; }
	s32 rout(u8 ch) { return m_ch[ch & 0x3].m_right; }

	// bypass chips host interface for debug purpose only
	u16 read(u8 address, bool cpu_access = false);
	void write(u8 address, u16 data, bool cpu_access = false);

	u16 regs_r(u8 page, u8 address, bool cpu_access = false);
	void regs_w(u8 page, u8 address, u16 data, bool cpu_access = false);

	u16 regs_r(u8 page, u8 address) { u8 prev = m_page; m_page = page; u16 ret = read(address, false); m_page = prev; return ret; }

protected:
	virtual inline u8 max_voices() override { return 32; }
	virtual void voice_tick() override;

private:
	struct output_t
	{
		void reset()
		{
			m_left = 0;
			m_right = 0;
		};

		s32 m_left = 0;
		s32 m_right = 0;
	};

	// es5505 voice structs
	struct voice_t : es550x_voice_t
	{
		// constructor
		voice_t(es5505_core &host)
			: es550x_voice_t(20, 9, false)
			, m_host(host)
		{}

		// internal state
		virtual void reset() override;
		virtual void fetch(u8 voice, u8 cycle) override;
		virtual void tick(u8 voice) override;

		s32 volume_calc(u8 volume, s32 in);

		// registers
		es5505_core &m_host;
		u8 m_lvol = 0; // Left volume
		u8 m_rvol = 0; // Right volume
		output_t m_ch; // channel output
	};

	struct sermode_t
	{
		sermode_t()
			: adc(0)
			, test(0)
			, sony_bb(0)
			, msb(0)
		{};

		void reset()
		{
			adc = 0;
			test = 0;
			sony_bb = 0;
			msb = 0;
		}

		u8 adc     : 1; // A/D
		u8 test    : 1; // Test
		u8 sony_bb : 1; // Sony/BB format serial output
		u8 msb     : 5; // Serial output MSB
	};

	voice_t m_voice[32]; // 32 voices
	// Serial related stuffs
	sermode_t m_sermode; // Serial mode register
	clock_pulse_t<s8, 4, 0> m_bclk;  // BCLK clock (CLKIN / 4), freely running clock
	clock_pulse_t<s8, 16, 1> m_lrclk; // LRCLK
	s16 m_wclk = 0;               // WCLK
	bool m_wclk_lr = false;       // WCLK, L/R output select
	s8 m_output_bit = 0;          // Bit position in output
	output_t m_ch[4];   // 4 stereo output channels
	output_t m_output[4]; // Serial outputs
	output_t m_output_temp[4]; // temporary signal for serial output
	output_t m_output_latch[4]; // output latch
};

#endif
