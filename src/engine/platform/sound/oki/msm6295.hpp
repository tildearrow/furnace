/*
	License: BSD-3-Clause
	see https://github.com/cam900/vgsound_emu/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	OKI MSM6295 emulation core

	See msm6295.cpp for more info.
*/

#include "util.hpp"
#include "vox.hpp"
#include <algorithm>
#include <memory>

#ifndef _VGSOUND_EMU_MSM6295_HPP
#define _VGSOUND_EMU_MSM6295_HPP

#pragma once

class msm6295_core : public vox_core
{
	friend class vgsound_emu_mem_intf; // common memory interface
public:
	// constructor
	msm6295_core(vgsound_emu_mem_intf &intf)
		: m_voice{{*this,*this},{*this,*this},{*this,*this},{*this,*this}}
		, m_intf(intf)
	{
	}
	// accessors, getters, setters
	u8 busy_r();
	void command_w(u8 data);
	void ss_w(bool ss) { m_ss = ss; } // SS pin

	// internal state
	void reset();
	void tick();

	s32 out() { return m_out; } // built in 12 bit DAC

private:
	// Internal volume table, 9 step
	const s32 m_volume_table[9] = {
		32/* 0.0dB */,
		22/* -3.2dB */,
		16/* -6.0dB */,
		11/* -9.2dB */,
		8/* -12.0dB */,
		6/* -14.5dB */,
		4/* -18.0dB */,
		3/* -20.5dB */,
		2/* -24.0dB */ }; // scale out to 5 bit for optimization

	// msm6295 voice structs
	struct voice_t : vox_decoder_t
	{
		// constructor
		voice_t(vox_core &vox, msm6295_core &host)
			: vox_decoder_t(vox)
			, m_host(host)
		{};

		// internal state
		virtual void reset() override;
		void tick();

		// accessors, getters, setters
		// registers
		msm6295_core &m_host;
		u16  m_clock   = 0;     // clock counter
		bool m_busy    = false; // busy status
		u8   m_command = 0;     // current command
		u32  m_addr    = 0;     // current address
		s8   m_nibble  = 0;     // current nibble
		u32  m_end     = 0;     // end address
		s32  m_volume  = 0;   // volume
		s32  m_out     = 0;     // output
	};
	voice_t m_voice[4];
	vgsound_emu_mem_intf &m_intf; // common memory interface

	bool m_ss              = false;  // SS pin controls divider, input clock / 33 * (SS ? 5 : 4)
	u8   m_command         = 0;      // Command byte
	u8   m_next_command    = 0;      // Next command
	bool m_command_pending = false;  // command pending flag
	u16  m_clock           = 0;      // clock counter
	s32  m_out             = 0;      // 12 bit output
};

#endif
