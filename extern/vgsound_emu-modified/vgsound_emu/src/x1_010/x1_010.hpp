/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Seta/Allumer X1-010 Emulation core
*/

#ifndef _VGSOUND_EMU_SRC_X1_010_HPP
#define _VGSOUND_EMU_SRC_X1_010_HPP

#pragma once

#include "../core/core.hpp"
#include "../core/util/mem_intf.hpp"

using namespace vgsound_emu;

class x1_010_core : public vgsound_emu_core
{
		friend class vgsound_emu_mem_intf;

	private:
		// 16 voices in chip
		class voice_t : public vgsound_emu_core
		{
			private:
				class flag_t : public vgsound_emu_core
				{
					public:
						flag_t()
							: vgsound_emu_core("x1_010_voice_flag")
							, m_div(0)
							, m_env_oneshot(0)
							, m_wavetable(0)
							, m_keyon(0)
						{
						}

						// internal state
						void reset()
						{
							m_div		  = 0;
							m_env_oneshot = 0;
							m_wavetable	  = 0;
							m_keyon		  = 0;
						}

						// register accessor
						inline void write(u8 data)
						{
							m_div		  = (data >> 7) & 1;
							m_env_oneshot = (data >> 2) & 1;
							m_wavetable	  = (data >> 1) & 1;
							m_keyon		  = (data >> 0) & 1;
						}

						// Setters
						inline void set_keyon(bool keyon) { m_keyon = keyon; }

						// Getters
						inline bool div() { return m_div; }

						inline bool env_oneshot() { return m_env_oneshot; }

						inline bool wavetable() { return m_wavetable; }

						inline bool keyon() { return m_keyon; }

					private:
						u8 m_div		 : 1;
						u8 m_env_oneshot : 1;
						u8 m_wavetable	 : 1;
						u8 m_keyon		 : 1;
				};

			public:
				// constructor
				voice_t(x1_010_core &host)
					: vgsound_emu_core("x1_010_voice")
					, m_host(host)
					, m_flag(flag_t())
					, m_vol_wave(0)
					, m_freq(0)
					, m_start_envfreq(0)
					, m_end_envshape(0)
					, m_acc(0)
					, m_env_acc(0)
					, m_data(0)
				{
					m_vol_out.fill(0);
					m_out.fill(0);
				}

				// internal state
				void reset();
				void tick();

				// register accessor
				u8 reg_r(u8 offset);
				void reg_w(u8 offset, u8 data);

				// getters
				inline s32 out(u8 ch) { return m_out[ch & 1]; }

			private:
				// host flag
				x1_010_core &m_host;
				// registers
				flag_t m_flag;
				u8 m_vol_wave	   = 0;
				u16 m_freq		   = 0;
				u8 m_start_envfreq = 0;
				u8 m_end_envshape  = 0;

				// internal registers
				u32 m_acc					= 0;
				u32 m_env_acc				= 0;
				s8 m_data					= 0;
				std::array<u8, 2> m_vol_out;

				// for preview only
				std::array<s32, 2> m_out;
		};

	public:
		// constructor
		x1_010_core(vgsound_emu_mem_intf &intf)
			: vgsound_emu_core("x1_010")
			, m_voice{*this,
					  *this,
					  *this,
					  *this,
					  *this,
					  *this,
					  *this,
					  *this,
					  *this,
					  *this,
					  *this,
					  *this,
					  *this,
					  *this,
					  *this,
					  *this}
			, m_intf(intf)
		{
			m_envelope.fill(0);
			m_wave.fill(0);
			m_out.fill(0);
		}

		// register accessor
		u8 ram_r(u16 offset);
		void ram_w(u16 offset, u8 data);

		// getters
		inline s32 output(u8 ch) { return m_out[ch & 1]; }

		// internal state
		void reset();
		void tick();

		// for preview only
		inline s32 voice_out(u8 voice, u8 ch)
		{
			return (voice < 16) ? m_voice[voice].out(ch & 1) : 0;
		}

	private:
		std::array<voice_t, 16> m_voice;
		vgsound_emu_mem_intf &m_intf;

		// RAM
		std::array<u8, 0x1000> m_envelope;
		std::array<u8, 0x1000> m_wave;

		// output data
		std::array<s32, 2> m_out;
};

#endif
