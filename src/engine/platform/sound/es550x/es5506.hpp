/*
	License: BSD-3-Clause
	see https://github.com/cam900/vgsound_emu/LICENSE for more details

	Copyright holder(s): cam900
	Ensoniq ES5504 emulation core

	See es550x.cpp for more info
*/

#include "es550x.hpp"

#ifndef _VGSOUND_EMU_ES5506_HPP
#define _VGSOUND_EMU_ES5506_HPP

#pragma once

// ES5506 specific
class es5506_core : public es550x_shared_core
{
public:
	// constructor
	es5506_core(es550x_intf &intf)
		: es550x_shared_core(intf)
		, m_voice{*this,*this,*this,*this,*this,*this,*this,*this,
		          *this,*this,*this,*this,*this,*this,*this,*this,
		          *this,*this,*this,*this,*this,*this,*this,*this,
		          *this,*this,*this,*this,*this,*this,*this,*this}
	{
	}
	// host interface
	u8 host_r(u8 address);
	void host_w(u8 address, u8 data);

	// internal state
	virtual void reset() override;
	virtual void tick() override;

	// less cycle accurate, but also less cpu heavy update routine
	void tick_perf();

	// clock outputs
	bool bclk() { return m_bclk.current_edge(); }
	bool bclk_rising_edge() { return m_bclk.rising_edge(); }
	bool bclk_falling_edge() { return m_bclk.falling_edge(); }

	// 6 stereo output channels
	s32 lout(u8 ch) { return m_output[std::min<u8>(5, ch & 0x7)].m_left; }
	s32 rout(u8 ch) { return m_output[std::min<u8>(5, ch & 0x7)].m_right; }

	// bypass chips host interface for debug purpose only
	u8 read(u8 address, bool cpu_access = false);
	void write(u8 address, u8 data, bool cpu_access = false);

	u32 regs_r(u8 page, u8 address, bool cpu_access = false);
	void regs_w(u8 page, u8 address, u32 data, bool cpu_access = false);

	u8 regs8_r(u8 page, u8 address) { u8 prev = m_page; m_page = page; u8 ret = read(address, false); m_page = prev; return ret; }
	void set_mute(u8 ch, bool mute) { m_voice[ch & 0x1f].m_mute = mute; }

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

	// es5506 voice structs
	struct voice_t : es550x_voice_t
	{
		// constructor
		voice_t(es5506_core &host)
			: es550x_voice_t(21, 11, true)
			, m_host(host) {}

		// internal state
		virtual void reset() override;
		virtual void fetch(u8 voice, u8 cycle) override;
		virtual void tick(u8 voice) override;

		// accessors, getters, setters
		s16 decompress(u8 sample);
		s32 volume_calc(u16 volume, s32 in);

		struct filter_ramp_t
		{
			filter_ramp_t()
				: slow(0)
				, ramp(0)
			{ };

			void reset()
			{
				slow = 0;
				ramp = 0;
			};

			u16 slow : 1; // Slow mode flag
			u16 ramp = 8; // Ramp value
		};

		// registers
		es5506_core &m_host;
		s32 m_lvol = 0; // Left volume - 4 bit exponent, 8 bit mantissa, 4 LSBs are used for fine control of ramp increment for hardware envelope
		s32 m_lvramp = 0; // Left volume ramp
		s32 m_rvol = 0; // Right volume
		s32 m_rvramp = 0; // Righr volume ramp
		s16 m_ecount = 0; // Envelope counter
		filter_ramp_t m_k2ramp; // Filter coefficient 2 Ramp
		filter_ramp_t m_k1ramp; // Filter coefficient 1 Ramp
		u8 m_filtcount = 0; // Internal counter for slow mode
		output_t m_ch; // channel output
		bool m_mute = false; // mute flag (for debug purpose)
	};

	// 5 bit mode
	struct mode_t
	{
		mode_t()
			: bclk_en(0)
			, wclk_en(0)
			, lrclk_en(0)
			, master(0)
			, dual(0)
		{ };

		void reset()
		{
			bclk_en = 1;
			wclk_en = 1;
			lrclk_en = 1;
			master = 0;
			dual = 0;
		}

		u8 bclk_en  : 1; // Set BCLK to output
		u8 wclk_en  : 1; // Set WCLK to output
		u8 lrclk_en : 1; // Set LRCLK to output
		u8 master   : 1; // Set memory mode to master
		u8 dual     : 1; // Set dual chip config
	};

	voice_t m_voice[32]; // 32 voices

	// Host interfaces
	u32 m_read_latch = 0; // 32 bit register latch for host read
	u32 m_write_latch = 0; // 32 bit register latch for host write

	// Serial register
	u8 m_w_st = 0; // Word clock start register
	u8 m_w_end = 0; // Word clock end register
	u8 m_lr_end = 0; // Left/Right clock end register
	mode_t m_mode; // Global mode

	// Serial related stuffs
	u8 m_w_st_curr = 0; // Word clock start, current status
	u8 m_w_end_curr = 0; // Word clock end register
	clock_pulse_t<s8, 4, 0> m_bclk;  // BCLK clock (CLKIN / 4), freely running clock
	clock_pulse_t<s8, 32, 1> m_lrclk; // LRCLK
	s16 m_wclk = 0;               // WCLK
	bool m_wclk_lr = false;       // WCLK, L/R output select
	u8 m_output_bit = 0;          // Bit position in output
	output_t m_ch[6]; // 6 stereo output channels
	output_t m_output[6]; // Serial outputs
	output_t m_output_temp[6]; // temporary signal for serial output
	output_t m_output_latch[6]; // output latch
};

#endif
