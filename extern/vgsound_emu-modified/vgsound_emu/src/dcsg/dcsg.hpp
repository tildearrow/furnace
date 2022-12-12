/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Texas instruments digital complex sound generators and variants emulation core
*/

#ifndef _VGSOUND_EMU_SRC_DCSG_HPP
#define _VGSOUND_EMU_SRC_DCSG_HPP

#pragma once

#include "../core/core.hpp"

using namespace vgsound_emu;

class dcsg_intf : public vgsound_emu_core
{
	public:
		dcsg_intf()
			: vgsound_emu_core("dcsg_intf")
		{
		}

		virtual void ready_w(bool ready) {}
};

class dcsg_core : public vgsound_emu_core
{
		friend class dcsg_intf;

	private:
		const u32 m_clock_divider = 16;	 // clock divider
		const u8 m_has_audio_in : 1;	 // audio input exists?
		const u8 m_negative_out : 1;	 // negative output?
		const u8 m_is_ncr		: 1;	 // ncr/tandy variant?
		const u8 m_is_sega		: 1;	 // sega variant?
		const u8 m_is_gg		: 1;	 // game gear variant?
		const u8 m_is_ngp		: 1;	 // neogeo pocket variant?

		// Common ALU for sound channels
		class alu_t : public vgsound_emu_core
		{
			public:
				alu_t(std::string tag, dcsg_core &host)
					: vgsound_emu_core(tag)
					, m_host(host)
					, m_divider(0)
					, m_counter(0)
					, m_volume(0)
					, m_lvolume(0)
					, m_rvolume(0)
					, m_out(0)
					, m_lout(0)
					, m_rout(0)
					, m_left_en(1)
					, m_right_en(1)
					, m_pulse(0)
				{
				}

				virtual void reset();
				virtual bool tick();

				void get_output()
				{
					const bool pulse = tick();
					m_out			 = pulse ? m_host.volume_value(m_volume) : 0;
					m_lout			 = pulse ? m_host.volume_value(m_lvolume) : 0;
					m_rout			 = pulse ? m_host.volume_value(m_rvolume) : 0;
				}

				// accessors
				inline void counter_reset()
				{
					m_counter = ((!m_host.is_sega()) || (m_divider > 0)) ? m_divider : 1;
				}

				// setters
				inline void set_divider(const u16 divider, const u16 mask = 0x3ff)
				{
					m_divider = (m_divider & ~(mask & 0x3ff)) | (divider & (mask & 0x3ff));
					counter_reset();
				}

				inline void set_volume(const u8 volume) { m_volume = volume & 0xf; }

				inline void set_lvolume(const u8 volume) { m_lvolume = volume & 0xf; }

				inline void set_rvolume(const u8 volume) { m_rvolume = volume & 0xf; }

				inline void set_left_en(const bool enable) { m_left_en = boolmask<u8>(enable); }

				inline void set_right_en(const bool enable) { m_right_en = boolmask<u8>(enable); }

				// getters
				inline u16 divider() const { return m_divider; }

				inline u16 counter() const { return m_counter; }

				inline s32 out() const { return m_out; }

				inline s32 lout() const { return m_lout; }

				inline s32 rout() const { return m_rout; }

				inline bool left_en() const { return m_left_en; }

				inline bool right_en() const { return m_right_en; }

			protected:
				dcsg_core &m_host;
				u32 m_divider : 10;	 // clock divider
				u32 m_counter : 10;	 // clock counter
				u32 m_volume  : 4;	 // volume
				u32 m_lvolume : 4;	 // left volume
				u32 m_rvolume : 4;	 // right volume
				s32 m_out  = 0;		 // output per channel
				s32 m_lout = 0;		 // left output per channel
				s32 m_rout = 0;		 // right output per channel
				u8 m_left_en  : 1;	 // left output enable
				u8 m_right_en : 1;	 // right output enable
				u8 m_pulse	  : 6;	 // clock pulse
		};

		// 3 Square channels
		class square_t : public alu_t
		{
			private:

			public:
				square_t(dcsg_core &host)
					: alu_t("dcsg_square", host)
				{
				}

				virtual bool tick() override;
		};

		// 1 Noise channel
		class noise_t : public alu_t
		{
			private:
				const u32 m_lfsr_feedback = 0x10000;  // LFSR feedback mask

			public:
				noise_t(dcsg_core &host, const u32 lfsr_feedback)
					: alu_t("dcsg_noise", host)
					, m_lfsr_feedback(lfsr_feedback)
					, m_lfsr(0)
					, m_shift(0)
					, m_mode(0)
					, m_fixed_count(0)
				{
				}

				virtual void reset() override;
				virtual bool tick() override;

				// accessors
				inline void clear_lfsr() { m_lfsr = m_lfsr_feedback; }

				inline void clear_fixed_counter() { m_fixed_count = 1 << 4; }

				// setters
				inline void set_shift(const u8 shift)
				{
					m_shift = shift & 3;
					clear_fixed_counter();
				}

				inline void set_mode(const bool mode)
				{
					const u16 prev = m_mode;
					m_mode		   = boolmask<u16>(mode);
					if (m_mode ^ prev)
					{
						if (m_host.is_ncr())
						{
							clear_lfsr();
						}
					}
				}

				// getters
				inline u32 lfsr() const { return m_lfsr; }

				inline u8 shift() const { return m_shift; }

			private:
				u32 m_lfsr = 0;			 // LFSR state
				u16 m_shift		  : 2;	 // Frequency shift
				u16 m_mode		  : 1;	 // LFSR mode
				u16 m_fixed_count : 13;	 // fixed counter
		};

	public:
		// constructor
		dcsg_core(std::string tag,
				  dcsg_intf &intf,
				  const u32 clock_divider,
				  const bool has_audio_in,
				  const bool negative_out,
				  const bool is_ncr,
				  const bool is_sega,
				  const bool is_gg,
				  const bool is_ngp,
				  const u32 lfsr_feedback)
			: vgsound_emu_core(tag)
			, m_clock_divider(clock_divider)
			, m_has_audio_in(has_audio_in)
			, m_negative_out(negative_out)
			, m_is_ncr(is_ncr)
			, m_is_sega(is_sega)
			, m_is_gg(is_gg)
			, m_is_ngp(is_ngp)
			, m_intf(intf)
			, m_square{*this, *this, *this}
			, m_noise(*this, lfsr_feedback)
			, m_in(0)
			, m_out(0)
			, m_lout(0)
			, m_rout(0)
			, m_clock_counter(0)
			, m_reg_voice{u8(is_sega ? 0 : 3)}
			, m_addr_latch(0)
			, m_reg_latch(0)
			, m_write_pending(1)
			, m_ready(1)
		{
			calculate_volume_lut();
		}

		dcsg_core(dcsg_intf &intf,
				  u32 clock_divider,
				  bool has_audio_in,
				  bool negative_out,
				  bool is_ncr,
				  bool is_sega,
				  bool is_gg,
				  bool is_ngp,
				  const u32 lfsr_feedback)
			: dcsg_core("dcsg",
						intf,
						clock_divider,
						has_audio_in,
						negative_out,
						is_ncr,
						is_sega,
						is_gg,
						is_ngp,
						lfsr_feedback)
		{
		}

		// accessors, getters, setters
		void register_w(const u8 address, const u8 data);
		void square_w(const u8 voice, const u8 address, const u8 data);
		void noise_w(const u8 address, const u8 data);

		// bypass chips host interface for debug purpose only
		void write(const u8 address, const u8 data);

		// internal state
		void reset();
		void tick();

		// less cycle accurate, but also less cpu heavy update routine
		void tick_perf();

		inline void set_audio_in(const s32 in)
		{
			if (m_has_audio_in)
			{
				m_in = in;
			}
		}

		// getters
		inline u32 clock_divider() const { return m_clock_divider; }

		// sound output
		inline s32 out() const { return m_out; }

		inline s32 lout() const { return m_lout; }

		inline s32 rout() const { return m_rout; }

		// for debug/preview only
		inline s32 square_out(const u8 square) const
		{
			return (square < 3) ? m_square[square].out() : 0;
		}

		inline s32 square_lout(const u8 square) const
		{
			return (square < 3) ? m_square[square].lout() : 0;
		}

		inline s32 square_rout(const u8 square) const
		{
			return (square < 3) ? m_square[square].rout() : 0;
		}

		inline s32 noise_out() const { return m_noise.out(); }

		inline s32 noise_lout() const { return m_noise.lout(); }

		inline s32 noise_rout() const { return m_noise.rout(); }

	protected:
		// calculate volume LUT
		virtual void calculate_volume_lut()
		{
			// default (-2dB per step)
			for (int v = 0; v < 15; v++)
			{
				m_volume_lut[v] = s32((dB_to_gain(f32(v) * -2.0f) * 32767.0f) / 4.0f);
			}
			m_volume_lut[15] = 0;
		}

		// calculate noise
		virtual bool calculate_noise(const u32 lfsr, const bool mode) const
		{
			// default (SN76494/SN76496 behavior)
			return (bitfield(lfsr, 2) ^ (mode && bitfield(lfsr, 3)));
		}

		// configuration getters
		inline bool is_ncr() const { return m_is_ncr; }

		inline bool is_sega() const { return m_is_sega; }

		inline s32 volume_value(const u8 volume) const { return m_volume_lut[volume & 0xf]; }

		// volume LUT
		std::array<s32, 16> m_volume_lut = {0};

	private:
		// execute output
		void output_exec();

		dcsg_intf &m_intf;

		std::array<square_t, 3> m_square;  // 3 square channels
		noise_t m_noise;				   // noise channel

		s32 m_in					  = 0;		 // audio input
		s32 m_out					  = 0;		 // analog output
		s32 m_lout					  = 0;		 // analog left output
		s32 m_rout					  = 0;		 // analog right output
		u32 m_clock_counter			  = 0;		 // clock counter
		std::array<u8, 2> m_reg_voice = {0, 0};	 // last selected voice
		u8 m_addr_latch				  = 0;		 // address latch
		u8 m_reg_latch				  = 0;		 // register latch
		u8 m_write_pending : 1;					 // write pending
		u8 m_ready		   : 1;					 // ready flag
};

// device defines

// SN76489
class sn76489_core : public dcsg_core
{
	public:
		// constructor
		sn76489_core(dcsg_intf &intf)
			: sn76489_core("sn76489", intf, 16)
		{
		}

	protected:
		// constructor
		sn76489_core(std::string tag, dcsg_intf &intf, u32 clock_divider)
			: dcsg_core(tag, intf, clock_divider, false, true, false, false, false, false, 0x4000)
		{
		}

		virtual bool calculate_noise(const u32 lfsr, const bool mode) const override
		{
			return (bitfield(lfsr, 0) ^ (mode && bitfield(lfsr, 1)));
		}
};

// SN94624
class sn94624_core : public sn76489_core
{
	public:
		// constructor
		sn94624_core(dcsg_intf &intf)
			: sn76489_core("sn94624", intf, 2)
		{
		}
};

// TMS9919
class tms9919_core : public sn76489_core
{
	public:
		// constructor
		tms9919_core(dcsg_intf &intf)
			: sn76489_core("tms9919", intf, 2)
		{
		}
};

// SN76489A
class sn76489a_core : public dcsg_core
{
	public:
		sn76489a_core(dcsg_intf &intf)
			: sn76489a_core("sn76489a", intf, 16, false)
		{
		}

	protected:
		sn76489a_core(std::string tag, dcsg_intf &intf, u32 clock_divider, bool has_audio_in)
			: dcsg_core(tag,
						intf,
						clock_divider,
						has_audio_in,
						false,
						false,
						false,
						false,
						false,
						0x10000)
		{
		}
};

// SN76494/A
class sn76494_core : public sn76489a_core
{
	public:
		sn76494_core(dcsg_intf &intf)
			: sn76489a_core("sn76494", intf, 2, true)
		{
		}
};

// SN76496/A
class sn76496_core : public sn76489a_core
{
	public:
		sn76496_core(dcsg_intf &intf)
			: sn76489a_core("sn76496", intf, 16, true)
		{
		}
};

// NCR 8496
class ncr8496_core : public dcsg_core
{
	public:
		// constructor
		ncr8496_core(dcsg_intf &intf)
			: ncr8496_core("ncr8496", intf, true)
		{
		}

	protected:
		// constructor
		ncr8496_core(std::string tag, dcsg_intf &intf, bool negative_out)
			: dcsg_core(tag, intf, 16, true, negative_out, false, false, false, false, 0x8000)
		{
		}

		virtual bool calculate_noise(const u32 lfsr, const bool mode) const override
		{
			return (bitfield(lfsr, 1) ^ (mode && bitfield(~lfsr, 5)));
		}
};

// Tandy PSSJ-3
class pssj3_core : public ncr8496_core
{
	public:
		// constructor
		pssj3_core(dcsg_intf &intf)
			: ncr8496_core("ncr8496", intf, false)
		{
		}
};

// Sega VDP PSG
class sega_vdp_psg_core : public dcsg_core
{
	public:
		// constructor
		sega_vdp_psg_core(dcsg_intf &intf)
			: sega_vdp_psg_core("sega_vdp_psg", intf, false)
		{
		}

	protected:
		// constructor
		sega_vdp_psg_core(std::string tag, dcsg_intf &intf, bool is_gg)
			: dcsg_core(tag, intf, 16, false, true, false, true, is_gg, false, 0x8000)
		{
		}

		virtual bool calculate_noise(const u32 lfsr, const bool mode) const override
		{
			return (bitfield(lfsr, 0) ^ (mode && bitfield(lfsr, 2)));
		}
};

// Sega Game Gear PSG
class gamegear_psg_core : public sega_vdp_psg_core
{
	public:
		// constructor
		gamegear_psg_core(dcsg_intf &intf)
			: sega_vdp_psg_core("gamegear_psg", intf, true)
		{
		}
};

// Neo Geo Pocket PSG
class neogeo_pocket_psg_core : public dcsg_core
{
	public:
		// constructor
		neogeo_pocket_psg_core(dcsg_intf &intf)
			: dcsg_core("neogeo_pocket_psg",
						intf,
						16,
						false,
						false,
						false,
						false,
						false,
						true,
						0x4000)
		{
		}

	protected:
		virtual bool calculate_noise(const u32 lfsr, const bool mode) const override
		{
			// same as SN76489/SN94624?
			return (bitfield(lfsr, 0) ^ (mode && bitfield(lfsr, 1)));
		}
};

#endif
