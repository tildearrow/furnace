/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Sharp SM8521 sound emulation core
*/

#ifndef _VGSOUND_EMU_SRC_SM8521_HPP
#define _VGSOUND_EMU_SRC_SM8521_HPP

#pragma once

#include "../core/core.hpp"

using namespace vgsound_emu;

class sm8521_core : public vgsound_emu_core
{
	private:
		// Common ALU for sound channels
		class alu_t : public vgsound_emu_core
		{
			private:
				class divider_t : public vgsound_emu_core
				{
					public:
						divider_t()
							: vgsound_emu_core("sm8521_frequency_divider")
							, m_divider(0)
						{
						}

						void reset() { m_divider = 0; }

						void write(const bool msb, const u8 data);

						// getters
						inline u16 divider() const { return m_divider; }

					private:
						u16 m_divider : 12;	 // Time constant register (pitch)
				};

			public:
				alu_t(std::string tag, sm8521_core &host)
					: vgsound_emu_core(tag)
					, m_host(host)
					, m_divider(divider_t())
					, m_out(0)
					, m_counter(0)
					, m_cycle(0)
					, m_volume(0)
				{
				}

				virtual void reset();
				virtual bool tick();

				virtual s16 get_output()
				{
					m_out = 0;
					return 0;
				}

				// accessors
				inline void clear_cycle() { m_cycle = 0; }

				// setters
				inline void set_volume(const u8 volume) { m_volume = volume & 0x1f; }

				// getters
				divider_t &divider() { return m_divider; }

				inline u16 counter() const { return m_counter; }

				inline u8 cycle() const { return m_cycle; }

				inline u8 volume() const { return m_volume; }

				// for previwe/debug only
				inline s16 out() const { return m_out; }

			protected:
				sm8521_core &m_host;
				divider_t m_divider;
				s16 m_out = 0;		 // 8 bit output per channel
				u16 m_counter : 12;	 // clock counter
				u16 m_cycle	  : 4;	 // clock cycle
				u8 m_volume	  : 5;	 // Output level control register (volume)
		};

		// 2 Pulse channels
		class wave_t : public alu_t
		{
			private:
				class waveform_t
				{
					public:
						waveform_t(u8 byte)
							: m_byte(byte)
						{
						}

						// setters
						inline void set_byte(const u8 byte) { m_byte = byte; }

						// getters
						inline u8 byte() const { return m_byte; }

						inline u8 nibble(const bool step) const { return step ? m_msb : m_lsb; }

					private:
						union
						{
								struct
								{
										u8 m_lsb : 4;
										u8 m_msb : 4;
								};

								u8 m_byte = 0;
						};
				};

			public:
				wave_t(sm8521_core &host)
					: alu_t("sm8521_wave", host)
					, m_wave{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
				{
				}

				virtual void reset() override;
				virtual s16 get_output() override;

				// accessors
				inline void clear_counter() { m_counter = 0; }

				// setters
				inline void set_byte(const u8 step, const u8 byte)
				{
					m_wave[step & 0xf].set_byte(byte);
				}

				// getters
				inline u8 byte(const u8 step) const { return m_wave[step & 0xf].byte(); }

				inline u8 nibble(const u8 step) const
				{
					return m_wave[(step >> 1) & 0xf].nibble(step & 1);
				}

			private:
				std::array<waveform_t, 16> m_wave;	// 4 bit waveform, 16 step
				u8 m_counter : 5;					// 5 bit counter
		};

		// 1 Sawtooth channel
		class noise_t : public alu_t
		{
			public:
				noise_t(sm8521_core &host)
					: alu_t("sm8521_noise", host)
					, m_lfsr(1)
				{
				}

				virtual void reset() override;
				virtual s16 get_output() override;

				// accessors
				inline void clear_lfsr() { m_lfsr = 1; }

			private:
				u32 m_lfsr = 1;	 // LFSR (unknown algorithm)
		};

		class global_control_t : public vgsound_emu_core
		{
			public:
				global_control_t()
					: vgsound_emu_core("sm8521_global_control")
					, m_sg0_en(0)
					, m_sg1_en(0)
					, m_sg2_en(0)
					, m_dac_en(0)
					, m_enable(0)
				{
				}

				void reset()
				{
					m_sg0_en = 0;
					m_sg1_en = 0;
					m_sg2_en = 0;
					m_dac_en = 0;
					m_enable = 0;
				}

				// accessors
				inline void write(const u8 data)
				{
					m_sg0_en = bitfield(data, 0);
					m_sg1_en = bitfield(data, 1);
					m_sg2_en = bitfield(data, 2);
					m_dac_en = bitfield(data, 3);
					m_enable = bitfield(data, 7);
				}

				// getters
				inline u8 byte() { return m_byte; }

				inline bool sg0_en() const { return m_sg0_en; }

				inline bool sg1_en() const { return m_sg1_en; }

				inline bool sg2_en() const { return m_sg2_en; }

				inline bool dac_en() const { return m_dac_en; }

				inline bool enable() const { return m_enable; }

			private:
				union
				{
						struct
						{
								u8 m_sg0_en : 1;  // SG0 output enable
								u8 m_sg1_en : 1;  // SG1 output enable
								u8 m_sg2_en : 1;  // SG2 output enable
								u8 m_dac_en : 1;  // D/A direct output enable
								u8			: 3;  // unused
								u8 m_enable : 1;  // Sound output enable
						};

						u8 m_byte;
				};
		};

	public:
		// constructor
		sm8521_core()
			: vgsound_emu_core("vrc_vi")
			, m_wave{*this, *this}
			, m_noise(*this)
			, m_control(global_control_t())
			, m_out(0)
		{
		}

		// accessors, getters, setters
		void wave_w(const u8 voice, const u8 address, const u8 data);
		void waveform_w(const u8 voice, const u8 address, const u8 data);
		void noise_w(const u8 address, const u8 data);
		void control_w(const u8 data);
		void dac_w(const u8 data);

		u8 wave_r(const u8 voice, const u8 address);
		u8 waveform_r(const u8 voice, const u8 address);
		u8 noise_r(const u8 address);
		u8 control_r();

		// internal state
		void reset();
		s8 tick();

		template<typename T>
		void tick_stream(const std::size_t stream_len, T *out)
		{
			for (std::size_t s = 0; s < stream_len; s++)
			{
				out[s] = tick();
			}
		}

		// 10 bit output
		inline s16 out() const { return m_out; }

		// for debug/preview only
		inline s8 wave_out(const u8 wave) const { return (wave < 2) ? m_wave[wave].out() : 0; }

		inline s8 noise_out() const { return m_noise.out(); }

	protected:
		global_control_t &control() { return m_control; }

	private:
		std::array<wave_t, 2> m_wave;  // 2 pulse channels
		noise_t m_noise;			   // noise channel
		global_control_t m_control;	   // control

		s16 m_out = 0;	// 10 bit output
		s8 m_dac  = 0;	// D/A direct output
};

#endif
