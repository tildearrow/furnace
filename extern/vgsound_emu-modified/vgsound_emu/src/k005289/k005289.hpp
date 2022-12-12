/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Konami K005289 emulation core
*/

#ifndef _VGSOUND_EMU_SRC_K005289_HPP
#define _VGSOUND_EMU_SRC_K005289_HPP

#pragma once

#include "../core/core.hpp"
using namespace vgsound_emu;

class k005289_core : public vgsound_emu_core
{
	private:
		// k005289 timer classes
		class timer_t : public vgsound_emu_core
		{
			public:
				timer_t()
					: vgsound_emu_core("k005289_timer")
					, m_counter(0)
					, m_addr(0)
					, m_pitch(0)
					, m_freq(0)
				{
				}

				// internal state
				void reset();
				void tick();

				// accessors
				// Replace current frequency to lastest loaded pitch
				inline void update() { m_freq = m_pitch; }

				// setters
				// Load pitch data (address pin)
				inline void load(const u16 pitch) { m_pitch = bitfield(pitch, 0, 12); }

				// getters
				inline u8 addr() const { return bitfield(m_addr, 0, 5); }

			private:
				// registers
				u32 m_counter : 12;	 // frequency counter
				u32 m_addr	  : 5;	 // external address pin
				u32 m_pitch	  : 12;	 // pitch
				u32 m_dummy0  : 3;	 // dummy
				u16 m_freq	  : 12;	 // current frequency
				u16 m_dummy1  : 4;	 // dummy
		};

	public:
		// constructor
		k005289_core()
			: vgsound_emu_core("k005289")
			, m_timer{timer_t()}
		{
		}

		// internal state
		void reset();
		void tick();

		void tick_stream(const std::size_t stream_len)
		{
			for (std::size_t s = 0; s < stream_len; s++)
			{
				tick();
			};
		}

		// accessors
		// TG1/2 pin
		inline void update(const u8 voice) { m_timer[voice & 1].update(); }

		// setters
		// LD1/2 pin, A0...11 pin
		inline void load(const u8 voice, const u16 addr) { m_timer[voice & 1].load(addr); }

		// getters
		// 1QA...E/2QA...E pin
		inline u8 addr(const u8 voice) const { return m_timer[voice & 1].addr(); }

	private:
		std::array<timer_t, 2> m_timer;
};

#endif
