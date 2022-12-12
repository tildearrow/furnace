/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Ensoniq ES5506 emulation core
*/

#ifndef _VGSOUND_EMU_SRC_ES5506_HPP
#define _VGSOUND_EMU_SRC_ES5506_HPP

#pragma once

#include "es550x.hpp"

// ES5506 specific
class es5506_core : public es550x_shared_core
{
	private:
		class output_t : public vgsound_emu_core
		{
			public:
				output_t(s32 left = 0, s32 right = 0)
					: vgsound_emu_core("es5506_output")
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

				inline s32 clamp20(const s32 in) const { return clamp(in, -0x80000, 0x7ffff); }

				inline void clamp20(output_t &src)
				{
					m_left	= clamp20(src.left());
					m_right = clamp20(src.right());
				}

				inline void clamp20()
				{
					m_left	= clamp20(m_left);
					m_right = clamp20(m_right);
				}

				// setters
				inline void set_left(const s32 left) { m_left = clamp20(left); }

				inline void set_right(const s32 right) { m_right = clamp20(right); }

				inline void serial_in(const bool ch, const u8 in)
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
					m_left	= clamp20(m_left + src.left());
					m_right = clamp20(m_right + src.right());
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

		// es5506 voice classes
		class voice_t : public es550x_voice_t
		{
			private:
				// es5506 Filter ramp class
				class filter_ramp_t : public vgsound_emu_core
				{
					public:
						filter_ramp_t()
							: vgsound_emu_core("es5506_filter_ramp")
							, m_slow(0)
							, m_ramp(0)
						{
						}

						inline void reset()
						{
							m_slow = 0;
							m_ramp = 0;
						};

						// Setters
						inline void write(const u16 data)
						{
							m_slow = bitfield(data, 0);
							m_ramp = bitfield(data, 8, 8);
						}

						// Getters
						inline bool slow() const { return m_slow; }

						inline u16 ramp() const { return m_ramp; }

					private:
						u16 m_slow	: 1;  // Slow mode flag
						u16 m_dummy : 7;  // dummy
						u16 m_ramp	: 8;  // Ramp value
				};

			public:
				// constructor
				voice_t(es5506_core &host)
					: es550x_voice_t("es5506_voice", 21, 11, true, m_cr_r, m_cr_w)
					, m_host(host)
					, m_lvol(0)
					, m_rvol(0)
					, m_lvramp(0)
					, m_rvramp(0)
					, m_ecount(0)
					, m_k2ramp(filter_ramp_t())
					, m_k1ramp(filter_ramp_t())
					, m_filtcount(0)
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
				inline void set_lvol(const s32 lvol) { m_lvol = lvol; }

				inline void set_rvol(const s32 rvol) { m_rvol = rvol; }

				inline void set_lvramp(const s32 lvramp) { m_lvramp = lvramp; }

				inline void set_rvramp(const s32 rvramp) { m_rvramp = rvramp; }

				inline void set_ecount(const s16 ecount) { m_ecount = ecount; }

				// getters
				inline s32 lvol() const { return m_lvol; }

				inline s32 rvol() const { return m_rvol; }

				inline s32 lvramp() const { return m_lvramp; }

				inline s32 rvramp() const { return m_rvramp; }

				inline s16 ecount() const { return m_ecount; }

				inline filter_ramp_t &k2ramp() { return m_k2ramp; }

				inline filter_ramp_t &k1ramp() { return m_k1ramp; }

				output_t &ch() { return m_ch; }

				// for debug/preview only
				inline void set_mute(const bool mute) { m_mute = mute; }

				inline s32 left_out() const { return m_mute ? 0 : m_output[0]; }

				inline s32 right_out() const { return m_mute ? 0 : m_output[1]; }

			private:
				// accessors, getters, setters
				s16 decompress(const u8 sample) const;
				s32 volume_calc(const u16 volume, const s32 in) const;

				std::function<void(u32)> m_cr_w = [this](u32 data)
				{
					alu().set_stop(bitfield(data, 0, 2));
					alu().set_lei(bitfield(data, 2));
					alu().set_loop(bitfield(data, 3, 2));
					alu().set_irqe(bitfield(data, 5));
					alu().set_dir(bitfield(data, 6));
					alu().set_irq(bitfield(data, 7));
					filter().set_lp(bitfield(data, 8, 2));
					cr().set_ca(std::min<u8>(5, bitfield(data, 10, 3)));
					cr().set_cmpd(bitfield(data, 13));
					cr().set_bs(bitfield(data, 14, 2));
				};

				std::function<u32(u32)> m_cr_r = [this](u32 ret) -> u32
				{
					return (ret & ~0xffff) | (alu().stop() << 0) | (alu().lei() ? 0x0004 : 0x0000) |
						   (alu().loop() << 3) | (alu().irqe() ? 0x0020 : 0x0000) |
						   (alu().dir() ? 0x0040 : 0x0000) | (alu().irq() ? 0x0080 : 0x0000) |
						   (bitfield(filter().lp(), 0, 2) << 8) | (cr().ca() << 10) |
						   (cr().cmpd() ? 0x2000 : 0x0000) | (cr().bs() << 14);
				};

				// registers
				es5506_core &m_host;
				// Volume register: 4 bit exponent, 8 bit mantissa
				// 4 LSBs are used for fine control of ramp increment for hardware envelope
				s32 m_lvol = 0;	 // Left volume
				s32 m_rvol = 0;	 // Right volume
				// Envelope
				s32 m_lvramp = 0;			  // Left volume ramp
				s32 m_rvramp = 0;			  // Righr volume ramp
				s16 m_ecount = 0;			  // Envelope counter
				filter_ramp_t m_k2ramp;		  // Filter coefficient 2 Ramp
				filter_ramp_t m_k1ramp;		  // Filter coefficient 1 Ramp
				u8 m_filtcount = 0;			  // Internal counter for slow mode
				output_t m_ch;				  // channel output
				bool m_mute = false;		  // mute flag (for debug purpose)
				std::array<s32, 2> m_output;  // output preview (for debug purpose)
		};

		// 5 bit mode
		class mode_t : public vgsound_emu_core
		{
			public:
				mode_t()
					: vgsound_emu_core("es5506_mode")
					, m_lrclk_en(1)
					, m_wclk_en(1)
					, m_bclk_en(1)
					, m_master(0)
					, m_dual(0)
				{
				}

				// internal states
				inline void reset()
				{
					m_lrclk_en = 1;
					m_wclk_en  = 1;
					m_bclk_en  = 1;
					m_master   = 0;
					m_dual	   = 0;
				}

				// accessors
				inline void write(const u8 data)
				{
					m_lrclk_en = bitfield(data, 0);
					m_wclk_en  = bitfield(data, 1);
					m_bclk_en  = bitfield(data, 2);
					m_master   = bitfield(data, 3);
					m_dual	   = bitfield(data, 4);
				}

				// getters
				inline bool lrclk_en() const { return m_lrclk_en; }

				inline bool wclk_en() const { return m_wclk_en; }

				inline bool bclk_en() const { return m_bclk_en; }

				inline bool master() const { return m_master; }

				inline bool dual() const { return m_dual; }

			private:
				u8 m_lrclk_en : 1;	// Set LRCLK to output
				u8 m_wclk_en  : 1;	// Set WCLK to output
				u8 m_bclk_en  : 1;	// Set BCLK to output
				u8 m_master	  : 1;	// Set memory mode to master
				u8 m_dual	  : 1;	// Set dual chip config
				u8 m_dummy	  : 3;	// dummy
		};

	public:
		// constructor
		es5506_core(es550x_intf &intf)
			: es550x_shared_core("es5506", 32, intf)
			, m_voice{*this, *this, *this, *this, *this, *this, *this, *this, *this, *this, *this,
					  *this, *this, *this, *this, *this, *this, *this, *this, *this, *this, *this,
					  *this, *this, *this, *this, *this, *this, *this, *this, *this, *this}
			, m_read_latch(0)
			, m_write_latch(0)
			, m_w_st(0)
			, m_w_end(0)
			, m_lr_end(0)
			, m_mode(mode_t())
			, m_w_st_curr(0)
			, m_w_end_curr(0)
			, m_bclk(clock_pulse_t<s8>(4, 0))
			, m_lrclk(clock_pulse_t<s8>(32, 1))
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
		u8 host_r(const u8 address);
		void host_w(const u8 address, const u8 data);

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
				for (u8 c = 0; c < 12; c += 2)
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
				for (u8 c = 0; c < 12; c += 2)
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

		// 6 stereo output channels
		inline s32 lout(const u8 ch) const { return m_output[std::min<u8>(5, ch & 0x7)].left(); }

		inline s32 rout(const u8 ch) const { return m_output[std::min<u8>(5, ch & 0x7)].right(); }

		//-----------------------------------------------------------------
		//
		//	for preview/debug purpose only, not for serious emulators
		//
		//-----------------------------------------------------------------

		// bypass chips host interface for debug purpose only
		u8 read(const u8 address, const bool cpu_access = false);
		void write(const u8 address, const u8 data);

		u32 regs_r(const u8 page, const u8 address, const bool cpu_access = false);
		void regs_w(const u8 page, const u8 address, const u32 data);

		u8 regs8_r(const u8 page, const u8 address)
		{
			u8 prev = m_page;
			m_page	= page;
			u8 ret	= read(address, false);
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

		// Host interfaces
		u32 m_read_latch  = 0;	// 32 bit register latch for host read
		u32 m_write_latch = 0;	// 32 bit register latch for host write

		// Serial register
		u8 m_w_st	= 0;  // Word clock start register
		u8 m_w_end	= 0;  // Word clock end register
		u8 m_lr_end = 0;  // Left/Right clock end register
		mode_t m_mode;	  // Global mode

		// Serial related stuffs
		u8 m_w_st_curr	= 0;					 // Word clock start, current status
		u8 m_w_end_curr = 0;					 // Word clock end register
		clock_pulse_t<s8> m_bclk;				 // BCLK clock (CLKIN / 4), freely running clock
		clock_pulse_t<s8> m_lrclk;				 // LRCLK
		s16 m_wclk		= 0;					 // WCLK
		bool m_wclk_lr	= false;				 // WCLK, L/R output select
		s8 m_output_bit = 0;					 // Bit position in output
		std::array<output_t, 6> m_ch;			 // 6 stereo output channels
		std::array<output_t, 6> m_output;		 // Serial outputs
		std::array<output_t, 6> m_output_temp;	 // temporary signal for serial output
		std::array<output_t, 6> m_output_latch;	 // output latch
};

#endif
