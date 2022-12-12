/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Ensoniq ES5504 emulation core
*/

#ifndef _VGSOUND_EMU_SRC_ES5505_HPP
#define _VGSOUND_EMU_SRC_ES5505_HPP

#pragma once

#include "es550x.hpp"

// ES5505 specific
class es5505_core : public es550x_shared_core
{
	private:
		class output_t : public vgsound_emu_core
		{
			public:
				output_t(s32 left = 0, s32 right = 0)
					: vgsound_emu_core("es5505_output")
					, m_left(left)
					, m_right(right)
				{
				}

				inline void reset()
				{
					m_left	= 0;
					m_right = 0;
				}

				inline void copy_output(const output_t &src)
				{
					m_left	= src.left();
					m_right = src.right();
				}

				inline s32 clamp16(const s32 in) const { return clamp(in, -0x8000, 0x7fff); }

				inline void clamp16(output_t &src)
				{
					m_left	= clamp16(src.left());
					m_right = clamp16(src.right());
				}

				inline void clamp16()
				{
					m_left	= clamp16(m_left);
					m_right = clamp16(m_right);
				}

				// setters
				inline void set_left(const s32 left) { m_left = left; }

				inline void set_right(const s32 right) { m_right = right; }

				inline void serial_in(bool ch, u8 in)
				{
					if (ch)	 // Right output
					{
						m_right = (m_right << 1) | boolmask<s32>(in);
					}
					else  // Left output
					{
						m_left = (m_left << 1) | boolmask<s32>(in);
					}
				}

				// getters
				inline s32 left() const { return m_left; }

				inline s32 right() const { return m_right; }

				output_t &operator+=(output_t &src)
				{
					m_left	= clamp16(m_left + src.left());
					m_right = clamp16(m_right + src.right());
					return *this;
				}

				output_t &operator>>(const s32 shift)
				{
					m_left	>>= shift;
					m_right >>= shift;
					return *this;
				}

			private:
				s32 m_left	= 0;
				s32 m_right = 0;
		};

		// es5505 voice classes
		class voice_t : public es550x_voice_t
		{
			public:
				// constructor
				voice_t(es5505_core &host)
					: es550x_voice_t("es5505_voice", 20, 9, false, m_cr_r, m_cr_w)
					, m_host(host)
					, m_lvol(0)
					, m_rvol(0)
					, m_ch(output_t())
					, m_mute(false)
					, m_output{0}
				{
				}

				// internal state
				virtual void reset() override;
				virtual void fetch(const u8 voice, const u8 cycle) override;
				virtual void tick(const u8 voice) override;

				// setters
				inline void set_lvol(const u8 lvol) { m_lvol = lvol; }

				inline void set_rvol(const u8 rvol) { m_rvol = rvol; }

				// getters
				inline u8 lvol() const { return m_lvol; }

				inline u8 rvol() const { return m_rvol; }

				output_t &ch() { return m_ch; }

				// for debug/preview only
				inline void set_mute(const bool mute) { m_mute = mute; }

				inline s32 left_out() const { return m_mute ? 0 : m_output[0]; }

				inline s32 right_out() const { return m_mute ? 0 : m_output[1]; }

			private:
				s32 volume_calc(u8 volume, s32 in) const;

				std::function<u32(u32)> m_cr_r = [this](u32 ret) -> u32
				{
					return (ret & ~0xfff) | (alu().stop() << 0) |
						   (bitfield(cr().bs(), 0) ? 0x04 : 0x00) | (alu().lpe() ? 0x08 : 0x00) |
						   (alu().ble() ? 0x10 : 0x00) | (alu().irqe() ? 0x20 : 0x00) |
						   (alu().dir() ? 0x40 : 0x00) | (alu().irq() ? 0x80 : 0x00) |
						   (bitfield(cr().ca(), 0, 2) << 8) | (bitfield(filter().lp(), 0, 2) << 10);
				};

				std::function<void(u32)> m_cr_w = [this](u32 data)
				{
					alu().set_stop(bitfield(data, 0, 2));
					cr().set_bs(bitfield(data, 2));
					alu().set_lpe(bitfield(data, 3));
					alu().set_ble(bitfield(data, 4));
					alu().set_irqe(bitfield(data, 5));
					alu().set_dir(bitfield(data, 6));
					alu().set_irq(bitfield(data, 7));
					cr().set_ca(bitfield(data, 8, 2));
					filter().set_lp(bitfield(data, 10, 2));
				};

				// registers
				es5505_core &m_host;
				u8 m_lvol = 0;				  // Left volume
				u8 m_rvol = 0;				  // Right volume
				output_t m_ch;				  // channel output
				bool m_mute = false;		  // mute flag (for debug purpose)
				std::array<s32, 2> m_output;  // output preview (for debug purpose)
		};

		class sermode_t : public vgsound_emu_core
		{
			public:
				sermode_t()
					: vgsound_emu_core("es5505_sermode")
					, m_adc(0)
					, m_test(0)
					, m_sony_bb(0)
					, m_msb(0)
				{
				}

				inline void reset()
				{
					m_adc	  = 0;
					m_test	  = 0;
					m_sony_bb = 0;
					m_msb	  = 0;
				}

				// setters
				inline void write(const u16 data)
				{
					m_adc	  = bitfield(data, 0);
					m_test	  = bitfield(data, 1);
					m_sony_bb = bitfield(data, 2);
					m_msb	  = bitfield(data, 11, 5);
				}

				inline void set_adc(const bool adc) { m_adc = boolmask<u8>(adc); }

				inline void set_test(const bool test) { m_test = boolmask<u8>(test); }

				inline void set_sony_bb(const bool sony_bb) { m_sony_bb = boolmask<u8>(sony_bb); }

				inline void set_msb(const u8 msb) { m_msb = bitfield(msb, 0, 5); }

				// getters
				inline bool adc() const { return m_adc; }

				inline bool test() const { return m_test; }

				inline bool sony_bb() const { return m_sony_bb; }

				inline u8 msb() const { return m_msb; }

			private:
				u8 m_adc	 : 1;  // A/D
				u8 m_test	 : 1;  // Test
				u8 m_sony_bb : 1;  // Sony/BB format serial output
				u8 m_msb	 : 5;  // Serial output MSB
		};

	public:
		// constructor
		es5505_core(es550x_intf &intf)
			: es550x_shared_core("es5505", 32, intf)
			, m_voice{*this, *this, *this, *this, *this, *this, *this, *this, *this, *this, *this,
					  *this, *this, *this, *this, *this, *this, *this, *this, *this, *this, *this,
					  *this, *this, *this, *this, *this, *this, *this, *this, *this, *this}
			, m_sermode(sermode_t())
			, m_bclk(clock_pulse_t<s8>(4, 0))
			, m_lrclk(clock_pulse_t<s8>(16, 1))
			, m_wclk(0)
			, m_wclk_lr(false)
			, m_output_bit(0)
			, m_ch{output_t()}
			, m_output{output_t()}
			, m_output_temp{output_t()}
			, m_output_latch{output_t()}
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
				for (u8 c = 0; c < 8; c += 2)
				{
					out[c + 0][s] += this->lout(c);
					out[c + 1][s] += this->rout(c);
				}
			}
		}

		template<typename T>
		void tick_stream_perf(const std::size_t stream_len, T **out)
		{
			for (std::size_t s = 0; s < stream_len; s++)
			{
				tick_perf();
				for (u8 c = 0; c < 8; c += 2)
				{
					out[c + 0][s] += this->lout(c);
					out[c + 1][s] += this->rout(c);
				}
			}
		}

		// clock outputs
		inline bool bclk() const { return m_bclk.current_edge(); }

		inline bool bclk_rising_edge() const { return m_bclk.rising_edge(); }

		inline bool bclk_falling_edge() const { return m_bclk.falling_edge(); }

		// Input mode for Channel 3
		inline void lin(const s32 in)
		{
			if (m_sermode.adc())
			{
				m_ch[3].set_left(in);
			}
		}

		inline void rin(const s32 in)
		{
			if (m_sermode.adc())
			{
				m_ch[3].set_right(in);
			}
		}

		// 4 stereo output channels
		inline s32 lout(const u8 ch) const { return m_ch[ch & 0x3].left(); }

		inline s32 rout(const u8 ch) const { return m_ch[ch & 0x3].right(); }

		//-----------------------------------------------------------------
		//
		//	for preview/debug purpose only, not for serious emulators
		//
		//-----------------------------------------------------------------

		// bypass chips host interface for debug purpose only
		u16 read(const u8 address, const bool cpu_access = false);
		void write(const u8 address, const u16 data);

		u16 regs_r(const u8 page, const u8 address, const bool cpu_access = false);
		void regs_w(const u8 page, const u8 address, const u16 data);

		u16 regs_r(const u8 page, const u8 address)
		{
			u8 prev = m_page;
			m_page	= page;
			u16 ret = read(address, false);
			m_page	= prev;
			return ret;
		}

		inline void set_mute(const u8 ch, const bool mute) { m_voice[ch & 0x1f].set_mute(mute); }

		// per-voice outputs
		inline s32 voice_lout(const u8 voice) const
		{
			return (voice < 32) ? m_voice[voice].left_out() : 0;
		}

		inline s32 voice_rout(const u8 voice) const
		{
			return (voice < 32) ? m_voice[voice].right_out() : 0;
		}

	protected:
		virtual inline u8 max_voices() const override { return 32; }

		virtual void voice_tick() override;

	private:
		std::array<voice_t, 32> m_voice;  // 32 voices
		// Serial related stuffs
		sermode_t m_sermode;					 // Serial mode register
		clock_pulse_t<s8> m_bclk;				 // BCLK clock (CLKIN / 4), freely running clock
		clock_pulse_t<s8> m_lrclk;				 // LRCLK
		s16 m_wclk		= 0;					 // WCLK
		bool m_wclk_lr	= false;				 // WCLK, L/R output select
		s8 m_output_bit = 0;					 // Bit position in output
		std::array<output_t, 4> m_ch;			 // 4 stereo output channels
		std::array<output_t, 4> m_output;		 // Serial outputs
		std::array<output_t, 4> m_output_temp;	 // temporary signal for serial output
		std::array<output_t, 4> m_output_latch;	 // output latch
};

#endif
