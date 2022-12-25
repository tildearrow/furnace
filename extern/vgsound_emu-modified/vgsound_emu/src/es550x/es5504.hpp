/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Ensoniq ES5504 emulation core
*/

#ifndef _VGSOUND_EMU_SRC_ES5504_HPP
#define _VGSOUND_EMU_SRC_ES5504_HPP

#pragma once

#include "es550x.hpp"

// ES5504 specific
class es5504_core : public es550x_shared_core
{
	private:
		// es5504 voice classes
		class voice_t : public es550x_voice_t
		{
			public:
				// constructor
				voice_t(es5504_core &host)
					: es550x_voice_t("es5504_voice", 20, 9, false)
					, m_host(host)
					, m_volume(0)
					, m_out(0)
				{
				}

				// internal state
				virtual void reset() override;
				virtual void fetch(u8 voice, u8 cycle) override;
				virtual void tick(u8 voice) override;

				// setters
				inline void set_volume(u16 volume) { m_volume = volume; }

				// getters
				inline u16 volume() { return m_volume; }

				inline s32 out() { return m_out; }

			private:
				void adc_exec();

				// registers
				es5504_core &m_host;
				u16 m_volume = 0;  // 12 bit Volume
				s32 m_out	 = 0;  // channel outputs
		};

	public:
		// constructor
		es5504_core(es550x_intf &intf)
			: es550x_shared_core("es5504", 25, intf)
			, m_voice{*this, *this, *this, *this, *this, *this, *this, *this, *this,
					  *this, *this, *this, *this, *this, *this, *this, *this, *this,
					  *this, *this, *this, *this, *this, *this, *this}
			, m_adc(0)
		{
			m_out.fill(0);
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
		inline s32 out(u8 ch) { return m_out[ch & 0xf]; }

		//-----------------------------------------------------------------
		//
		//	for preview/debug purpose only, not for serious emulators
		//
		//-----------------------------------------------------------------

		// bypass chips host interface for debug purpose only
		u16 read(u8 address, bool cpu_access = false);
		void write(u8 address, u16 data, bool cpu_access = false);

		u16 regs_r(u8 page, u8 address, bool cpu_access = false);
		void regs_w(u8 page, u8 address, u16 data, bool cpu_access = false);

		u16 regs_r(u8 page, u8 address)
		{
			u8 prev = m_page;
			m_page	= page;
			u16 ret = read(address, false);
			m_page	= prev;
			return ret;
		}

		// per-voice outputs
		inline s32 voice_out(u8 voice) { return (voice < 25) ? m_voice[voice].out() : 0; }

	protected:
		virtual inline u8 max_voices() override { return 25; }

		virtual void voice_tick() override;

	private:
		std::array<voice_t, 25> m_voice;  // 25 voices
		u16 m_adc				  = 0;	  // ADC register
		std::array<s32, 16> m_out;  // 16 channel outputs
};

#endif
