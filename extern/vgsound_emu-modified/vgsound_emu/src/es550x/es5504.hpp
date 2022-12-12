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
					: es550x_voice_t("es5504_voice", 20, 9, false, m_cr_r, m_cr_w)
					, m_host(host)
					, m_volume(0)
					, m_out(0)
				{
				}

				// internal state
				virtual void reset() override;
				virtual void fetch(const u8 voice, const u8 cycle) override;
				virtual void tick(const u8 voice) override;

				// setters
				inline void set_volume(const u16 volume) { m_volume = volume; }

				// getters
				inline u16 volume() const { return m_volume; }

				inline s32 out() const { return m_out; }

			private:
				void adc_exec();

				std::function<u32(u32)> m_cr_r = [this](u32 ret) -> u32
				{
					return (ret & ~0xff) | bitfield(alu().stop(), 0, 2) |
						   (cr().adc() ? 0x04 : 0x00) | (alu().lpe() ? 0x08 : 0x00) |
						   (alu().ble() ? 0x10 : 0x00) | (alu().irqe() ? 0x20 : 0x00) |
						   (alu().dir() ? 0x40 : 0x00) | (alu().irq() ? 0x80 : 0x00);
				};

				std::function<void(u32)> m_cr_w = [this](u32 data)
				{
					alu().set_stop(bitfield(data, 0, 2));
					cr().set_adc(bitfield(data, 2));
					alu().set_lpe(bitfield(data, 3));
					alu().set_ble(bitfield(data, 4));
					alu().set_irqe(bitfield(data, 5));
					alu().set_dir(bitfield(data, 6));
					alu().set_irq(bitfield(data, 7));
				};

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
			, m_out{0}
		{
		}

		// host interface
		u16 host_r(const u8 address);
		void host_w(const u8 address, const u16 data);

		// internal state
		virtual void reset() override;
		virtual void tick() override;

		// less cycle accurate, but also less cpu heavy update routine
		void tick_perf();

		template<typename T>
		void tick_stream(const std::size_t stream_len, T **out)
		{
			for (std::size_t s = 0; s < stream_len; s++)
			{
				tick();
				for (u8 c = 0; c < 16; c++)
				{
					out[c][s] += this->out(c);
				}
			}
		}

		template<typename T>
		void tick_stream_perf(const std::size_t stream_len, T **out)
		{
			for (std::size_t s = 0; s < stream_len; s++)
			{
				tick_perf();
				for (u8 c = 0; c < 16; c++)
				{
					out[c][s] += this->out(c);
				}
			}
		}

		// 16 analog output channels
		inline s32 out(const u8 ch) const { return m_out[ch & 0xf]; }

		//-----------------------------------------------------------------
		//
		//	for preview/debug purpose only, not for serious emulators
		//
		//-----------------------------------------------------------------

		// bypass chips host interface for debug purpose only
		u16 read(const u8 address, const bool cpu_access = false);
		void write(const u8 address, const u16 data, const bool cpu_access = false);

		u16 regs_r(const u8 page, const u8 address, const bool cpu_access = false);
		void regs_w(const u8 page, const u8 address, const u16 data, const bool cpu_access = false);

		u16 regs_r(const u8 page, const u8 address)
		{
			u8 prev = m_page;
			m_page	= page;
			u16 ret = read(address, false);
			m_page	= prev;
			return ret;
		}

		// per-voice outputs
		inline s32 voice_out(const u8 voice) const
		{
			return (voice < 25) ? m_voice[voice].out() : 0;
		}

	protected:
		virtual inline u8 max_voices() const override { return 25; }

		virtual void voice_tick() override;

	private:
		std::array<voice_t, 25> m_voice;  // 25 voices
		u16 m_adc				  = 0;	  // ADC register
		std::array<s32, 16> m_out = {0};  // 16 channel outputs
};

#endif
