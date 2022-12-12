/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Konami K007232 core
*/

#ifndef _VGSOUND_EMU_SRC_K007232_HPP
#define _VGSOUND_EMU_SRC_K007232_HPP

#pragma once

#include "../core/core.hpp"

using namespace vgsound_emu;

class k007232_intf : public vgsound_emu_core
{
	public:
		k007232_intf()
			: vgsound_emu_core("k007232_intf")
		{
		}

		// NE pin is executing voice number, and used for per-voice sample bank.
		virtual u8 read_sample(u8 ne, u32 address) { return 0; }

		// SLEV pin actived when 0x0c register accessed
		virtual void write_slev(u8 out) {}
};

class k007232_core : public vgsound_emu_core
{
		friend class k007232_intf;	// k007232 specific interface

	private:
		class voice_t : public vgsound_emu_core
		{
			public:
				// constructor
				voice_t(k007232_core &host)
					: vgsound_emu_core("k007232_voice")
					, m_host(host)
					, m_busy(0)
					, m_loop(0)
					, m_counter(0)
					, m_addr(0)
					, m_pitch(0)
					, m_start(0)
					, m_data(0)
					, m_out(0)
				{
				}

				// internal state
				void reset();
				s8 tick(const u8 ne);

				// accessors
				void write(const u8 address, const u8 data);
				void keyon();

				// setters
				inline void set_loop(const bool loop) { m_loop = boolmask<u32>(loop); }

				// getters
				inline s8 out() const { return m_out; }

			private:
				// registers
				k007232_core &m_host;
				u32 m_busy	  : 1;	 // busy status
				u32 m_loop	  : 1;	 // loop flag
				u32 m_counter : 12;	 // frequency counter
				u32 m_addr	  : 17;	 // current address
				u32 m_dummy0  : 1;	 // dummy
				u32 m_pitch	  : 14;	 // pitch, frequency divider
				u32 m_start	  : 17;	 // start position when keyon or loop start position at
									 // when reach end marker if loop enabled
				u32 m_dummy1 : 1;	 // dummy
				u8 m_data = 0;		 // current data
				s8 m_out  = 0;		 // current output (7 bit unsigned)
		};

	public:
		// constructor
		k007232_core(k007232_intf &intf)
			: vgsound_emu_core("k007232")
			, m_voice{*this, *this}
			, m_intf(intf)
			, m_reg{0}
		{
		}

		// host accessors
		void keyon(const u8 voice) { m_voice[voice & 1].keyon(); }

		void write(const u8 address, const u8 data);

		// internal state
		void reset();
		void tick();

		template<typename T>
		void tick_stream(const std::size_t stream_len, T **out)
		{
			for (std::size_t s = 0; s < stream_len; s++)
			{
				for (u8 v = 0; v < 2; v++)
				{
					out[v][s] = m_voice[v].tick(v);
				}
			};
		}

		// output for each voices, ASD/BSD pin
		inline s32 output(const u8 voice) const { return m_voice[voice & 1].out(); }

		// getters for debug, trackers, etc
		inline u8 reg_r(const u8 address) const { return m_reg[address & 0xf]; }

	private:
		std::array<voice_t, 2> m_voice;

		k007232_intf &m_intf;  // common memory interface

		std::array<u8, 16> m_reg = {0};	 // register pool
};

#endif
