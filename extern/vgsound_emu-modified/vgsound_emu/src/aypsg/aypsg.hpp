/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	General Instrument PSG and variants emulation core
*/

#ifndef _VGSOUND_EMU_SRC_AYPSG_HPP
#define _VGSOUND_EMU_SRC_AYPSG_HPP

#pragma once

#include "../core/core.hpp"

using namespace vgsound_emu;

class aypsg_intf : public vgsound_emu_core
{
	public:
		aypsg_intf()
			: vgsound_emu_core("aypsg_intf")
		{
		}

		virtual u8 io_r(u8 port) { return 0xff; }

		virtual void io_w(u8 port, u8 io) {}
};

class aypsg_core : public vgsound_emu_core
{
		friend class aypsg_intf;

	private:
		const u32 m_clock_divider = 8;	// clock divider
		const u8 m_has_io_a	   : 1;		// I/O A exists?
		const u8 m_has_io_b	   : 1;		// I/O B exists?
		const u8 m_single_mix  : 1;		// mix in single channel?
		const u8 m_has_divider : 1;		// has clock divider?

		// Common ALU for sound channels
		class alu_t : public vgsound_emu_core
		{
			public:
				alu_t(std::string tag, aypsg_core &host)
					: vgsound_emu_core(tag)
					, m_host(host)
					, m_divider(0)
					, m_counter(0)
					, m_pulse(0)
				{
				}

				virtual void reset();
				virtual bool tick();

				// accessors
				inline void counter_reset() { m_counter = std::max<u16>(1, m_divider); }

				// setters
				inline void set_divider(const u16 divider, const u16 mask = ~0)
				{
					m_divider = (m_divider & ~mask) | (divider & mask);
					counter_reset();
				}

				inline void set_pulse(const u8 pulse) { m_pulse = pulse; }

				// getters
				inline u16 divider() const { return m_divider; }

				inline u16 counter() const { return m_counter; }

				inline u8 pulse() { return m_pulse; }

			protected:
				aypsg_core &m_host;
				u16 m_divider = 0;	// clock divider
				u16 m_counter = 0;	// clock counter
				u8 m_pulse	  = 0;	// clock pulse
		};

		// 3 Square channels
		class square_t : public alu_t
		{
			private:
				std::function<void(const bool, const u8)> &m_update;
				const u8 m_index = 0;  // voice index

			public:
				square_t(aypsg_core &host,
						 std::function<void(const bool, const u8)> update_func,
						 const u8 index)
					: alu_t("aypsg_square", host)
					, m_update(update_func)
					, m_index(index)
					, m_volume(0)
					, m_lvolume(0)
					, m_rvolume(0)
					, m_tone(1)
					, m_noise(1)
					, m_envelope(0)
					, m_left_en(1)
					, m_right_en(1)
					, m_duty(4)
					, m_pulse_out(0)
				{
				}

				virtual void reset() override;
				virtual bool tick() override;

				// setters
				inline void set_volume(const u8 volume) { m_volume = volume & 0xf; }

				inline void set_lvolume(const u8 volume) { m_lvolume = volume & 0xf; }

				inline void set_rvolume(const u8 volume) { m_rvolume = volume & 0xf; }

				inline void set_tone(const bool enable) { m_tone = boolmask<u8>(enable); }

				inline void set_noise(const bool enable) { m_noise = boolmask<u8>(enable); }

				inline void set_envelope(const u8 envelope) { m_envelope = envelope & 3; }

				inline void set_left_en(const bool enable) { m_left_en = boolmask<u8>(enable); }

				inline void set_right_en(const bool enable) { m_right_en = boolmask<u8>(enable); }

				inline void set_duty(const u8 duty) { m_duty = duty & 0xf; }

				inline void set_pulse_out(const bool pulse_out)
				{
					m_pulse_out = boolmask<u32>(pulse_out);
				}

				// getters
				inline u8 volume() const { return m_volume; }

				inline bool tone() const { return m_tone; }

				inline bool noise() const { return m_noise; }

				inline u8 envelope() const { return m_envelope; }

				inline bool left_en() const { return m_left_en; }

				inline bool right_en() const { return m_right_en; }

				inline u8 duty() const { return m_duty; }

				inline bool pulse_out() const { return m_pulse_out; }

			private:
				u32 m_volume	: 5;  // volume
				u32 m_lvolume	: 5;  // left volume
				u32 m_rvolume	: 5;  // right volume
				u32 m_tone		: 1;  // tone enable (0 = enable)
				u32 m_noise		: 1;  // noise enable (0 = enable)
				u32 m_envelope	: 2;  // envelope enable
				u32 m_left_en	: 1;  // left output enable
				u32 m_right_en	: 1;  // right output enable
				u32 m_duty		: 4;  // pulse duty
				u32 m_pulse_out : 1;  // pulse output
		};

		// 1 Noise generator
		class noise_t : public alu_t
		{
			private:
				std::function<void(const bool)> &m_update;
				const u32 m_lfsr_feedback = 0x10000;  // LFSR feedback mask

			public:
				noise_t(aypsg_core &host,
						std::function<void(const bool)> update_func,
						const u32 lfsr_feedback)
					: alu_t("aypsg_noise", host)
					, m_update(update_func)
					, m_lfsr_feedback(lfsr_feedback)
					, m_and_mask(0)
					, m_or_mask(0)
					, m_mask_value(0)
					, m_noise_out(1)
					, m_lfsr(1)
				{
				}

				virtual void reset() override;
				virtual bool tick() override;

				// accessors
				inline void clear_lfsr() { m_lfsr = m_lfsr_feedback; }

				// setters
				inline void set_and_mask(const u8 mask) { m_and_mask = mask; }

				inline void set_or_mask(const u8 mask) { m_or_mask = mask; }

				inline void set_mask_value(const u8 value) { m_mask_value = value; }

				inline void set_noise_out(const bool value) { m_noise_out = boolmask<u32>(value); }

				inline void set_lfsr(const u32 lfsr) { m_lfsr = lfsr; }

				// getters
				inline u32 lfsr_feedback() const { return m_lfsr_feedback; }

				inline u8 and_mask() const { return m_and_mask; }

				inline u8 or_mask() const { return m_or_mask; }

				inline u8 mask_value() const { return m_mask_value; }

				inline bool noise_out() const { return m_noise_out; }

				inline u32 lfsr() const { return m_lfsr; }

			private:
				u8 m_and_mask	= 0;   // AND mask
				u8 m_or_mask	= 0;   // OR mask
				u8 m_mask_value = 0;   // Noise mask value
				u32 m_noise_out : 1;   // output
				u32 m_lfsr		: 31;  // LFSR state
		};

		// Envelope
		class envelope_t : public alu_t
		{
			private:

			public:
				envelope_t(aypsg_core &host)
					: alu_t("aypsg_envelope", host)
					, m_env_out(0)
					, m_holding(0)
					, m_hold(0)
					, m_alternate(0)
					, m_attack(0)
					, m_continue(0)
				{
				}

				virtual void reset() override;
				virtual bool tick() override;

				// setters
				inline void set_hold(const bool hold)
				{
					m_hold = boolmask<u8>(hold);
					if (!m_hold)
					{
						m_holding = 0;
					}
				}

				inline void set_alternate(const bool alternate)
				{
					m_alternate = boolmask<u8>(alternate);
				}

				inline void set_attack(const bool attack)
				{
					m_attack  = boolmask<u8>(attack);
					m_env_out = (m_attack ? 0x00 : 0x20);
				}

				inline void set_continue(const bool _continue)
				{
					m_continue = boolmask<u8>(_continue);
				}

				inline void ctrl_w(const u8 data)
				{
					const u8 prev = ctrl_r();
					if (prev != bitfield(data, 0, 4))
					{
						reset();
						set_hold(bitfield(data, 0));
						set_alternate(bitfield(data, 1));
						set_attack(bitfield(data, 2));
						set_continue(bitfield(data, 3));
					}
				}

				// getters
				inline u8 env_out() const
				{
					return (bitfield(m_env_out, 5)) ? (0x1f - bitfield(m_env_out, 0, 5))
													: bitfield(m_env_out, 0, 5);
				}

				inline bool hold() const { return m_hold; }

				inline bool alternate() const { return m_alternate; }

				inline bool attack() const { return m_attack; }

				inline bool _continue() const { return m_continue; }

				inline u8 ctrl_r() const
				{
					return (hold() ? 0x01 : 0x00) | (alternate() ? 0x02 : 0x00) |
						   (attack() ? 0x04 : 0x00) | (_continue() ? 0x08 : 0x00);
				}

			private:
				// Internal states
				u8 m_env_out : 6;  // Envelope output
				u8 m_holding : 1;  // Holding status
				// Register
				u8 m_hold	   : 1;	 // Hold
				u8 m_alternate : 1;	 // Alternate
				u8 m_attack	   : 1;	 // Attack
				u8 m_continue  : 1;	 // Continue
				u8			   : 5;	 // Unused registers
		};

		// IO Ports
		class io_t : public vgsound_emu_core
		{
			public:
				io_t(aypsg_core &host)
					: vgsound_emu_core("aypsg_io")
					, m_host(host)
					, m_input(0)
					, m_io_sel(false)
				{
				}

				void reset();

				// setters
				inline void set_input(const u8 input) { m_input = input; }

				inline void set_io_sel(const bool sel) { m_io_sel = sel; }

				// getters
				inline u8 input() const { return m_input; }

				inline bool io_sel() const { return m_io_sel; }

			private:
				aypsg_core &m_host;
				u8 m_input	  = 0;
				bool m_io_sel = false;
		};

	public:
		// constructor
		aypsg_core(std::string tag,
				   aypsg_intf &intf,
				   const u32 clock_divider,
				   const bool has_io_a,
				   const bool has_io_b,
				   const bool single_mix,
				   const bool has_divider,
				   const u32 lfsr_feedback)
			: aypsg_core(tag,
						 intf,
						 clock_divider,
						 has_io_a,
						 has_io_b,
						 single_mix,
						 has_divider,
						 m_update_pulse,
						 m_update_noise,
						 lfsr_feedback)
		{
			calculate_volume_lut();
		}

		aypsg_core(std::string tag,
				   aypsg_intf &intf,
				   const u32 clock_divider,
				   const bool has_io_a,
				   const bool has_io_b,
				   const bool single_mix,
				   const bool has_divider,
				   std::function<void(bool, u8)> update_pulse,
				   std::function<void(bool)> update_noise,
				   const u32 lfsr_feedback)
			: vgsound_emu_core(tag)
			, m_clock_divider(clock_divider)
			, m_has_io_a(has_io_a)
			, m_has_io_b(has_io_b)
			, m_single_mix(single_mix)
			, m_has_divider(has_divider)
			, m_volume_lut{0}
			, m_env_volume_lut{0}
			, m_intf(intf)
			, m_square{square_t(*this, update_pulse, 0),
					   square_t(*this, update_pulse, 1),
					   square_t(*this, update_pulse, 2)}
			, m_noise(*this, update_noise, lfsr_feedback)
			, m_envelope{*this, *this, *this}
			, m_io{*this, *this}
			, m_voice_out{0}
			, m_out{0}
			, m_clock_counter(0)
			, m_pulse(0)
			, m_reg_addr(0)
			, m_half_div(0)
		{
			calculate_volume_lut();
		}

		aypsg_core(aypsg_intf &intf,
				   u32 clock_divider,
				   const bool has_io_a,
				   const bool has_io_b,
				   const bool single_mix,
				   const bool has_divider,
				   const u32 lfsr_feedback)
			: aypsg_core("aypsg",
						 intf,
						 clock_divider,
						 has_io_a,
						 has_io_b,
						 single_mix,
						 has_divider,
						 lfsr_feedback)
		{
		}

		// accessors, getters, setters
		u8 muxed_r(const u8 address);
		void muxed_w(const u8 address, const u8 data);

		u8 direct_r(const u8 address);
		void direct_w(const u8 address, const u8 data);

		// bypass host write for debug purpose only
		virtual u8 data_r();
		virtual void data_w(const u8 data);
		void addr_w(const u8 data);

		// internal state
		virtual void reset();
		void tick();

		// ignore dividers
		void tick_perf();

		// setters
		inline void set_half_divider(bool div) { m_half_div = boolmask<u8>(div); }

		// getters
		inline bool half_divider() const { return m_half_div; }

		inline u32 clock_divider() const { return m_clock_divider; }

		// sound output
		inline s32 out(const u8 ch) const { return m_out[ch]; }

		// for debug/preview only
		inline s32 voice_out(const u8 voice) const { return (voice < 3) ? m_voice_out[voice] : 0; }

	protected:
		// setters
		inline void set_reg_addr(const u8 reg_addr) { m_reg_addr = bitfield(reg_addr, 0, 4); }

		// getters
		inline bool has_io_a() { return m_has_io_a; }

		inline bool has_io_b() { return m_has_io_b; }

		inline aypsg_intf &intf() { return m_intf; }

		inline square_t &square(const u8 voice) { return m_square[voice]; }

		inline noise_t &noise() { return m_noise; }

		inline envelope_t &envelope(const u8 voice) { return m_envelope[voice]; }

		inline io_t &io(const u8 port) { return m_io[port]; }

		inline u8 reg_addr() const { return m_reg_addr; }

		inline u8 &clock_pulse() { return m_pulse; }

		virtual inline bool is_expanded_mode() const { return false; }

		virtual inline bool get_divider()
		{
			return ((!m_has_divider) || ((!half_divider()) || bitfield(clock_pulse()++, 0)));
		}

		// calculate volume LUT
		virtual void calculate_volume_lut()
		{
			// default (-3dB per step)
			for (int v = 0; v < 15; v++)
			{
				m_volume_lut[15 - v]		= s32(dB_to_gain(f32(v) * -3.0f) * 32767.0f);
				m_volume_lut[16 + (15 - v)] = m_volume_lut[15 - v];
			}
			m_volume_lut[0] = m_volume_lut[16] = 0;
			for (int e = 0; e < 32; e++)
			{
				m_env_volume_lut[e] = m_volume_lut[e >> 1];
			}
		}

		// calculate noise
		virtual inline bool calculate_noise(const u32 lfsr) const
		{
			// default (AY-3-8910 behavior)
			return (bitfield(lfsr, 0) ^ bitfield(lfsr, 3));
		}

		// tone volume getter
		virtual inline s32 get_volume(const u8 voice)
		{
			return square(voice).envelope() ? m_env_volume_lut[envelope(0).env_out()]
											: m_volume_lut[square(voice).volume()];
		}

		// configuration getters
		inline s32 volume_value(const u8 volume) const { return m_volume_lut[volume & 0x1f]; }

		inline s32 env_volume_value(const u8 volume) const
		{
			return m_env_volume_lut[volume & 0x1f];
		}

		inline void io_w(const u8 address, const u8 data)
		{
			const u8 port = bitfield(address, 0);
			if (((!port) && has_io_a()) || (port && has_io_b()))
			{
				io(port).set_input(data);
				if (io(port).io_sel())
				{
					intf().io_w(port, io(port).input());
				}
			}
		}

		inline void control_enable_w(const u8 data)
		{
			io(1).set_io_sel(bitfield(data, 7));
			io(0).set_io_sel(bitfield(data, 6));
			square(2).set_noise(bitfield(data, 5));
			square(1).set_noise(bitfield(data, 4));
			square(0).set_noise(bitfield(data, 3));
			square(2).set_tone(bitfield(data, 2));
			square(1).set_tone(bitfield(data, 1));
			square(0).set_tone(bitfield(data, 0));
		}

		inline u8 io_r(const u8 address)
		{
			const u8 port = bitfield(address, 0);
			if (((!port) && has_io_a()) || (port && has_io_b()))
			{
				if (!io(port).io_sel())
				{
					io(port).set_input(intf().io_r(port));
				}
				return io(port).input();
			}
			return 0xff;
		}

		inline u8 control_enable_r()
		{
			return (square(0).tone() ? 0x01 : 0x00) | (square(1).tone() ? 0x02 : 0x00) |
				   (square(2).tone() ? 0x04 : 0x00) | (square(0).noise() ? 0x08 : 0x00) |
				   (square(1).noise() ? 0x10 : 0x00) | (square(2).noise() ? 0x20 : 0x00) |
				   (io(0).io_sel() ? 0x40 : 0x00) | (io(1).io_sel() ? 0x80 : 0x00);
		}

		// volume LUT
		std::array<s32, 32> m_volume_lut	 = {0};
		std::array<s32, 32> m_env_volume_lut = {0};

	private:
		// execute output
		void output_exec();

		std::function<void(const bool, const u8)> m_update_pulse =
		  [this](const bool tick, const u8 voice)
		{
			if (tick)
			{
				square(voice).set_pulse(bitfield(square(voice).pulse() + 1, 0, 5));
				square(voice).set_pulse_out(bitfield(square(voice).pulse(), 4));
			}
		};

		std::function<void(const bool)> m_update_noise = [this](const bool tick)
		{
			if (tick)
			{
				noise().set_pulse(bitfield(noise().pulse() + 1, 0));
				if (noise().pulse() == 0)
				{
					noise().set_lfsr(
					  (noise().lfsr() >> 1) |
					  (calculate_noise(noise().lfsr()) ? noise().lfsr_feedback() : 0));
				}
				noise().set_noise_out(bitfield(noise().lfsr(), 0));
			}
		};

		aypsg_intf &m_intf;

		std::array<square_t, 3> m_square;	   // 3 square channels
		noise_t m_noise;					   // noise generator
		std::array<envelope_t, 3> m_envelope;  // envelope generator

		std::array<io_t, 2> m_io;  // 2x8 bit IO ports

		std::array<s32, 3> m_voice_out = {0};  // voice output
		std::array<s32, 3> m_out	   = {0};  // analog output (1 to 3)
		u32 m_clock_counter			   = 0;	   // clock counter
		u8 m_pulse					   = 0;	   // clock pulse
		u8 m_reg_addr : 4;					   // register address
		u8 m_half_div : 1;					   // clock divider
};

// device defines

// AY-3-8910
class ay_3_8910_core : public aypsg_core
{
	public:
		ay_3_8910_core(aypsg_intf &intf)
			: ay_3_8910_core("ay_3_8910", intf, true, true, false)
		{
		}

	protected:
		ay_3_8910_core(std::string tag,
					   aypsg_intf &intf,
					   const bool has_io_a,
					   const bool has_io_b,
					   const bool single_mix)
			: aypsg_core(tag, intf, 8, has_io_a, has_io_b, single_mix, false, 0x10000)
		{
		}
};

// AY-3-8912
class ay_3_8912_core : public ay_3_8910_core
{
	public:
		ay_3_8912_core(aypsg_intf &intf)
			: ay_3_8910_core("ay_3_8912", intf, true, false, false)
		{
		}
};

// AY-3-8913
class ay_3_8913_core : public ay_3_8910_core
{
	public:
		ay_3_8913_core(aypsg_intf &intf)
			: ay_3_8910_core("ay_3_8913", intf, false, false, false)
		{
		}
};

// AY-3-8914
class ay_3_8914_core : public ay_3_8910_core
{
	public:
		ay_3_8914_core(aypsg_intf &intf)
			: ay_3_8910_core("ay_3_8914", intf, true, false, false)
		{
		}

		virtual u8 data_r() override;
		virtual void data_w(const u8 data) override;

	protected:
		virtual inline s32 get_volume(const u8 voice) override
		{
			return square(voice).envelope()
				   ? m_env_volume_lut[envelope(0).env_out() >> (3 - square(voice).envelope())]
				   : m_volume_lut[square(voice).volume()];
		}
};

// YM2149
class ym2149_core : public aypsg_core
{
	public:
		ym2149_core(aypsg_intf &intf)
			: ym2149_core("ym2149", intf, true, true, true, false)
		{
		}

	protected:
		ym2149_core(std::string tag,
					aypsg_intf &intf,
					const bool has_io_a,
					const bool has_io_b,
					const bool has_divider,
					const bool single_mix)
			: aypsg_core(tag, intf, 8, has_io_a, has_io_b, single_mix, has_divider, 0x10000)
		{
		}

		// calculate volume LUT
		virtual void calculate_volume_lut() override
		{
			// 5 bit envelope
			for (int v = 0; v < 31; v++)
			{
				m_env_volume_lut[31 - v] = s32(dB_to_gain(f32(v) * -1.5f) * 32767.0f);
			}
			m_env_volume_lut[0] = 0;
			for (int e = 0; e < 16; e++)
			{
				m_volume_lut[e] = m_volume_lut[16 + e] = m_env_volume_lut[(e << 1) | 1];
			}
		}
};

// AY8930
class ay8930_core : public aypsg_core
{
	public:
		ay8930_core(aypsg_intf &intf)
			: aypsg_core("ay8930",
						 intf,
						 4,
						 true,
						 true,
						 false,
						 false,
						 m_update_pulse,
						 m_update_noise,
						 0x10000)
			, m_test(0)
			, m_expanded_mode(0)
			, m_bank(0)
		{
		}

		virtual void reset() override;

		virtual u8 data_r() override;
		virtual void data_w(const u8 data) override;

	protected:
		virtual inline bool is_expanded_mode() const override { return m_expanded_mode == 0b101; }

		virtual inline bool get_divider() override
		{
			return is_expanded_mode() ? (((!half_divider()) || bitfield(clock_pulse()++, 0)))
									  : (bitfield(++clock_pulse(), 0, half_divider() ? 2 : 1) == 0);
		}

		// calculate volume LUT
		virtual void calculate_volume_lut() override
		{
			// 5 bit envelope and volume
			for (int v = 0; v < 31; v++)
			{
				m_env_volume_lut[31 - v] = s32(dB_to_gain(f32(v) * -1.5f) * 32767.0f);
				m_volume_lut[31 - v]	 = m_env_volume_lut[31 - v];
			}
			m_volume_lut[0] = m_env_volume_lut[0] = 0;
		}

		// calculate noise
		virtual inline bool calculate_noise(const u32 lfsr) const override
		{
			// default (AY-3-8910 behavior)
			return (bitfield(lfsr, 0) ^ bitfield(lfsr, 2));
		}

		// tone volume getter
		virtual inline s32 get_volume(const u8 voice) override
		{
			return square(voice).envelope()
				   ? (m_env_volume_lut[(is_expanded_mode() ? envelope(voice).env_out()
														   : (envelope(voice).env_out() & ~1)) &
									   0x1f])
				   : (m_volume_lut[(is_expanded_mode() ? square(voice).volume()
													   : ((square(voice).volume() << 1) | 1)) &
								   0x1f]);
		}

	private:
		inline void write_env_mode(const u8 data)
		{
			const bool prev_expanded_mode = is_expanded_mode();
			m_expanded_mode				  = bitfield(data, 5, 3);
			m_bank						  = bitfield(data, 4);
			if (prev_expanded_mode != is_expanded_mode())
			{
				reset();
			}
			envelope(0).ctrl_w(data);
		}

		inline u8 read_env_mode()
		{
			return (m_expanded_mode << 5) | (m_bank ? 0x10 : 0x00) | envelope(0).ctrl_r();
		}

		std::function<void(const bool, const u8)> m_update_pulse =
		  [this](const bool tick, const u8 voice)
		{
			if (tick)
			{
				square(voice).set_pulse(bitfield(square(voice).pulse() + 1, 0, 5));
				if (is_expanded_mode())
				{
					square(voice).set_pulse_out(
					  bitfield(m_pulse_duty_lut[bitfield(square(voice).duty(), 0, 4)],
							   bitfield(square(voice).pulse(), 0, 5)));
				}
				else
				{
					square(voice).set_pulse_out(bitfield(square(voice).pulse(), 4));
				}
			}
		};

		std::function<void(const bool)> m_update_noise = [this](const bool tick)
		{
			if (tick)
			{
				if (is_expanded_mode())
				{
					noise().set_mask_value(noise().mask_value() + 1);
					if (noise().mask_value() >=
						((u8(noise().lfsr()) & noise().and_mask()) | noise().or_mask()))
					{
						noise().set_mask_value(0);
						noise().set_noise_out(!noise().noise_out());
						noise().set_lfsr(
						  (noise().lfsr() >> 1) |
						  (calculate_noise(noise().lfsr()) ? noise().lfsr_feedback() : 0));
					}
				}
				else
				{
					noise().set_pulse(bitfield(noise().pulse() + 1, 0));
					if (noise().pulse() == 0)
					{
						noise().set_lfsr(
						  (noise().lfsr() >> 1) |
						  (calculate_noise(noise().lfsr()) ? noise().lfsr_feedback() : 0));
					}
					noise().set_noise_out(bitfield(noise().lfsr(), 0));
				}
			}
		};

		// duty cycles
		std::array<u32, 16> m_pulse_duty_lut = {0x00000001,
												0x00000003,
												0x0000000f,
												0x000000ff,
												0x0000ffff,	 // 4
												0x00ffffff,
												0x0fffffff,
												0x3fffffff,
												0x7fffffff,	 // 8
												0x7fffffff,
												0x7fffffff,
												0x7fffffff,
												0x7fffffff,
												0x7fffffff,
												0x7fffffff,
												0x7fffffff};
		// AY8930 specific registers
		u8 m_test = 0;			 // TEST register
		u8 m_expanded_mode : 3;	 // mode/bank flags
		u8 m_bank		   : 1;
};

#endif
