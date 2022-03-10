/*
	License: BSD-3-Clause
	see https://github.com/cam900/vgsound_emu/LICENSE for more details

	Copyright holders: cam900
	Seta/Allumer X1-010 Emulation core

	See x1_010.cpp for more info.
*/

#include <algorithm>
#include <memory>

#ifndef _VGSOUND_EMU_X1_010_HPP
#define _VGSOUND_EMU_X1_010_HPP

#pragma once

typedef unsigned char       u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef signed char         s8;
typedef signed int         s32;

template<typename T> T bitfield(T in, u8 pos, u8 len = 1)
{
	return (in >> pos) & (len ? (T(1 << len) - 1) : 1);
}

class x1_010_mem_intf
{
public:
	virtual u8 read_byte(u32 address) { return 0; }
};

class x1_010_core
{
	friend class x1_010_mem_intf;
public:
	// constructor
	x1_010_core(x1_010_mem_intf &intf)
		: m_voice{*this,*this,*this,*this,
				  *this,*this,*this,*this,
				  *this,*this,*this,*this,
				  *this,*this,*this,*this}
		, m_intf(intf)
	{
		m_envelope = std::make_unique<u8[]>(0x1000);
		m_wave = std::make_unique<u8[]>(0x1000);

		std::fill_n(&m_envelope[0], 0x1000, 0);
		std::fill_n(&m_wave[0], 0x1000, 0);
	}

	// register accessor
	u8 ram_r(u16 offset);
	void ram_w(u16 offset, u8 data);

	// getters
	s32 output(u8 channel) { return m_out[channel & 1]; }

	// internal state
	void reset();
	void tick();

private:
	// 16 voices in chip
	struct voice_t
	{
		// constructor
		voice_t(x1_010_core &host) : m_host(host) {}

		// internal state
		void reset();
		void tick();

		// register accessor
		u8 reg_r(u8 offset);
		void reg_w(u8 offset, u8 data);

		// registers
		x1_010_core &m_host;
		struct flag_t
		{
			u8 div : 1;
			u8 env_oneshot : 1;
			u8 wavetable : 1;
			u8 keyon : 1;
			void reset()
			{
				div = 0;
				env_oneshot = 0;
				wavetable = 0;
				keyon = 0;
			}
			flag_t()
				: div(0)
				, env_oneshot(0)
				, wavetable(0)
				, keyon(0)
				{ }
		};
		flag_t flag;
		u8 vol_wave = 0;
		u16 freq = 0;
		u8 start_envfreq = 0;
		u8 end_envshape = 0;

		// internal registers
		u32 acc = 0;
		u32 env_acc = 0;
		s8 data = 0;
		u8 vol_out[2] = {0};
	};
	voice_t m_voice[16];

	// RAM
	std::unique_ptr<u8[]> m_envelope = nullptr;
	std::unique_ptr<u8[]> m_wave = nullptr;

	// output data
	s32 m_out[2] = {0};

	x1_010_mem_intf &m_intf;
};

#endif
