/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Nanao GA20 core
*/

#ifndef _VGSOUND_EMU_SRC_GA20_HPP
#define _VGSOUND_EMU_SRC_GA20_HPP

#pragma once

#include "../core/core.hpp"
#include "../core/util/mem_intf.hpp"

using namespace vgsound_emu;

class ga20_core : public vgsound_emu_core
{
		friend class vgsound_emu_mem_intf;	// common memory interface

	private:
		class voice_t : public vgsound_emu_core
		{
			public:
				// constructor
				voice_t(ga20_core &host)
					: vgsound_emu_core("ga20_voice")
					, m_host(host)
					, m_counter(0)
					, m_pitch(0)
					, m_busy(0)
					, m_loop(0)
					, m_addr(0)
					, m_start(0)
					, m_end(0)
					, m_volume(0)
					, m_data(0)
					, m_out(0)
				{
				}

				// internal state
				void reset();
				s32 tick();

				// accessors
				void write(const u8 address, const u8 data);
				void keyon();

				// setters
				inline void set_loop(const bool loop) { m_loop = boolmask<u32>(loop); }

				// getters
				inline bool busy() const { return m_busy; }

				inline s32 out() const { return m_out; }

			private:
				// registers
				ga20_core &m_host;
				u8 m_counter = 0;	 // frequency counter
				u8 m_pitch	 = 0;	 // pitch, frequency divider
				u32 m_busy : 1;		 // busy status
				u32 m_loop : 1;		 // loop flag
				u32 m_addr : 20;	 // current address
				u32		   : 10;	 // dummy
				u32 m_start	 = 0;	 // start position
				u32 m_end	 = 0xf;	 // end position
				s32 m_volume = 0;	 // current volume
				u8 m_data	 = 0;	 // current data
				s32 m_out	 = 0;	 // current output (8 bit unsigned)
		};

	public:
		// constructor
		ga20_core(vgsound_emu_mem_intf &intf)
			: vgsound_emu_core("ga20")
			, m_voice{*this, *this, *this, *this}
			, m_intf(intf)
			, m_reg{0}
		{
		}

		// host accessors
		u8 read(const u8 address) const;
		void write(const u8 address, const u8 data);

		// internal state
		void reset();
		void tick();

		template<typename T>
		void tick_stream(const std::size_t stream_len, T **out)
		{
			for (std::size_t s = 0; s < stream_len; s++)
			{
				for (u8 v = 0; v < 4; v++)
				{
					out[v][s] = m_voice[v].tick();
				}
			};
		}

		// output for each voices, ASD/BSD pin
		inline s32 output(const u8 voice) const { return m_voice[voice & 3].out(); }

		// getters for debug, trackers, etc
		inline u8 reg_r(const u8 address) const { return m_reg[address & 0xf]; }

	private:
		std::array<voice_t, 4> m_voice;
		vgsound_emu_mem_intf &m_intf;  // common memory interface

		std::array<u8, 32> m_reg = {0};	 // register pool
};

#endif
