/*
	License: BSD-3-Clause
	see https://github.com/cam900/vgsound_emu/LICENSE for more details

	Copyright holder(s): cam900
	Ensoniq ES5504 emulation core

	See es550x.cpp for more info
*/

#include "es550x.hpp"

#ifndef _VGSOUND_EMU_ES5504_HPP
#define _VGSOUND_EMU_ES5504_HPP

#pragma once

// ES5504 specific
class es5504_core : public es550x_shared_core
{
public:
	// constructor
	es5504_core(es550x_intf &intf)
		: es550x_shared_core(intf)
		, m_voice{*this,*this,*this,*this,*this,
		          *this,*this,*this,*this,*this,
		          *this,*this,*this,*this,*this,
		          *this,*this,*this,*this,*this,
		          *this,*this,*this,*this,*this}
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

	// 16 analog output channels
	s32 out(u8 ch) { return m_ch[ch & 0xf]; }

	// bypass chips host interface for debug purpose only
	u16 read(u8 address, bool cpu_access = false);
	void write(u8 address, u16 data, bool cpu_access = false);

	u16 regs_r(u8 page, u8 address, bool cpu_access = false);
	void regs_w(u8 page, u8 address, u16 data, bool cpu_access = false);

	u16 regs_r(u8 page, u8 address) { u8 prev = m_page; m_page = page; u16 ret = read(address, false); m_page = prev; return ret; }

protected:
	virtual inline u8 max_voices() override { return 25; }
	virtual void voice_tick() override;

private:
	// es5504 voice structs
	struct voice_t : es550x_voice_t
	{
		// constructor
		voice_t(es5504_core &host)
			: es550x_voice_t(20, 9, false)
			, m_host(host)
		{}

		// internal state
		virtual void reset() override;
		virtual void fetch(u8 voice, u8 cycle) override;
		virtual void tick(u8 voice) override;

		void adc_exec();

		// registers
		es5504_core &m_host;
		u16 m_volume = 0; // 12 bit Volume
		s32 m_ch = 0; // channel outputs
	};

	voice_t m_voice[25]; // 25 voices
	u16 m_adc = 0; // ADC register
	s32 m_ch[16] = {0}; // 16 channel outputs
};

#endif
