/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Namco 163 Sound emulation core
*/

#ifndef _VGSOUND_EMU_SRC_N163_HPP
#define _VGSOUND_EMU_SRC_N163_HPP

#pragma once

#include "../core/core.hpp"

using namespace vgsound_emu;

class n163_core : public vgsound_emu_core
{
	private:
		// Address latch
		class addr_latch_t : public vgsound_emu_core
		{
			public:
				addr_latch_t()
					: vgsound_emu_core("namco_163_addr_latch")
					, m_addr(0)
					, m_incr(0)
				{
				}

				void reset()
				{
					m_addr = 0;
					m_incr = 0;
				}

				// accessors
				inline void write(u8 data)
				{
					m_addr = (data >> 0) & 0x7f;
					m_incr = (data >> 7) & 0x01;
				}

				inline void addr_inc() { m_addr = (m_addr + 1) & 0x7f; }

				// getters
				inline u8 addr() { return m_addr; }

				inline bool incr() { return m_incr; }

			private:
				u8 m_addr : 7;
				u8 m_incr : 1;
		};

	public:
		n163_core()
			: vgsound_emu_core("namco_163")
			, m_disable(false)
			, m_voice_cycle(0x78)
			, m_addr_latch(addr_latch_t())
			, m_out(0)
			, m_multiplex(true)
			, m_acc(0)
		{
			m_ram.fill(0);
			m_voice_out.fill(0);
		}

		// accessors, getters, setters
		void addr_w(u8 data);
		void data_w(u8 data, bool cpu_access = false);
		u8 data_r(bool cpu_access = false);

		inline void set_disable(bool disable) { m_disable = disable; }

		// internal state
		void reset();
		void tick();

		// sound output pin
		inline s16 out() { return m_out; }

		// register pool
		inline u8 reg(u8 addr) { return m_ram[addr & 0x7f]; }

		inline void set_multiplex(bool multiplex = true) { m_multiplex = multiplex; }

		// preview only
		inline u8 voice_cycle() { return m_voice_cycle; }

		inline s16 voice_out(u8 voice)
		{
			return (voice <= ((m_ram[0x7f] >> 4) & 7)) ? m_voice_out[7 - voice] : 0;
		}

	private:
		bool m_disable			   = false;
		std::array<u8, 0x80> m_ram;	 // internal 128 byte RAM
		u8 m_voice_cycle		   = 0x78;	 // Voice cycle for processing
		addr_latch_t m_addr_latch;			 // address latch
		s16 m_out					   = 0;	 // output

		std::array<s16, 8> m_voice_out;  // per-voice output, for preview only
		// demultiplex related
		bool m_multiplex = true;  // multiplex flag, but less noisy = inaccurate!
		s16 m_acc		 = 0;	  // accumulated output
};

#endif
