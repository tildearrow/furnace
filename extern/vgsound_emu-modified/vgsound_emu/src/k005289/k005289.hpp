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
					, m_addr(0)
					, m_pitch(0)
					, m_freq(0)
					, m_counter(0)
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
				inline void load(u16 pitch) { m_pitch = pitch; }

				// getters
				inline u8 addr() { return m_addr; }

			private:
				// registers
				u8 m_addr	  = 0;	// external address pin
				u16 m_pitch	  = 0;	// pitch
				u16 m_freq	  = 0;	// current frequency
				s16 m_counter = 0;	// frequency counter
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

		// accessors
		// TG1/2 pin
		inline void update(int voice) { m_timer[voice & 1].update(); }

		// setters
		// LD1/2 pin, A0...11 pin
		inline void load(int voice, u16 addr) { m_timer[voice & 1].load(addr); }

		// getters
		// 1QA...E/2QA...E pin
		inline u8 addr(int voice) { return m_timer[voice & 1].addr(); }

	private:
		std::array<timer_t, 2> m_timer;
};

#endif
