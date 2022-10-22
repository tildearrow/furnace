/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	OKI MSM6295 emulation core
*/

#ifndef _VGSOUND_EMU_SRC_MSM6295_HPP
#define _VGSOUND_EMU_SRC_MSM6295_HPP

#pragma once

#include "../core/core.hpp"
#include "../core/util/mem_intf.hpp"
#include "../core/vox/vox.hpp"

using namespace vgsound_emu;

class msm6295_core : public vox_core
{
		friend class vgsound_emu_mem_intf;	// common memory interface

	private:
		// Internal volume table, 9 step
		const s32 m_volume_table[9] = {32 /* 0.0dB */,
									   22 /* -3.2dB */,
									   16 /* -6.0dB */,
									   11 /* -9.2dB */,
									   8 /* -12.0dB */,
									   6 /* -14.5dB */,
									   4 /* -18.0dB */,
									   3 /* -20.5dB */,
									   2 /* -24.0dB */};  // scale out to 5 bit for optimization

		// msm6295 voice classes
		class voice_t : vox_decoder_t
		{
			public:
				// constructor
				voice_t(msm6295_core &host)
					: vox_decoder_t(host, false)
					, m_host(host)
					, m_clock(0)
					, m_busy(false)
					, m_command(0)
					, m_addr(0)
					, m_nibble(0)
					, m_end(0)
					, m_volume(0)
					, m_out(0)
					, m_mute(false)
				{
				}

				// internal state
				virtual void reset() override;
				void tick();

				// Setters
				inline void set_command(u8 command) { m_command = command; }

				inline void set_volume(s32 volume)
				{
					m_volume = (volume < 9) ? m_host.m_volume_table[volume] : 0;
				}

				inline void set_mute(bool mute) { m_mute = mute; }

				// Getters
				inline bool busy() { return m_busy; }

				inline s32 out() { return m_mute ? 0 : m_out; }

			private:
				// accessors, getters, setters
				// registers
				msm6295_core &m_host;  // host core
				u16 m_clock	 = 0;	   // clock counter
				bool m_busy	 = false;  // busy status
				u8 m_command = 0;	   // current command
				u32 m_addr	 = 0;	   // current address
				s8 m_nibble	 = 0;	   // current nibble
				u32 m_end	 = 0;	   // end address
				s32 m_volume = 0;	   // volume
				s32 m_out	 = 0;	   // output
				// for preview only
				bool m_mute = false;  // mute flag
		};

	public:
		// constructor
		msm6295_core(vgsound_emu_mem_intf &intf)
			: vox_core("msm6295")
			, m_voice{*this, *this, *this, *this}
			, m_intf(intf)
			, m_ss(false)
			, m_command(0)
			, m_next_command(0)
			, m_command_pending(false)
			, m_clock(0)
			, m_counter(0)
			, m_out(0)
			, m_out_temp(0)
		{
		}

		// accessors, getters, setters
		u8 busy_r();
		void command_w(u8 data);

		inline void ss_w(bool ss) { m_ss = ss; }  // SS pin

		// internal state
		void reset();
		void tick();

		inline s32 out() { return m_out; }	// built in 12 bit DAC

		// for preview
		inline void voice_mute(u8 voice, bool mute)
		{
			if (voice < 4)
			{
				m_voice[voice].set_mute(mute);
			}
		}

		inline s32 voice_out(u8 voice) { return (voice < 4) ? m_voice[voice].out() : 0; }

	private:
		std::array<voice_t, 4> m_voice;
		vgsound_emu_mem_intf &m_intf;  // common memory interface

		bool m_ss			   = false;	 // SS pin controls divider, input clock / 33 * (SS ? 5 : 4)
		u8 m_command		   = 0;		 // Command byte
		u8 m_next_command	   = 0;		 // Next command
		bool m_command_pending = false;	 // command pending flag
		u16 m_clock			   = 0;		 // clock counter
		u16 m_counter		   = 0;		 // another clock counter
		s32 m_out			   = 0;		 // 12 bit output
		s32 m_out_temp		   = 0;		 // temporary buffer of above
};

#endif
