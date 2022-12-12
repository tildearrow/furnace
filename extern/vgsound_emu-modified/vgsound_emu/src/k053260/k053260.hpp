/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Konami K053260 core
*/

#ifndef _VGSOUND_EMU_SRC_K053260_HPP
#define _VGSOUND_EMU_SRC_K053260_HPP

#pragma once

#include "../core/core.hpp"

using namespace vgsound_emu;

class k053260_intf : public vgsound_emu_core
{
	public:
		k053260_intf()
			: vgsound_emu_core("k053260_intf")
		{
		}

		virtual u8 read_sample(u32 address) { return 0; }  // sample fetch

		virtual void write_int(u8 out) {}  // timer interrupt
};

class k053260_core : public vgsound_emu_core
{
		friend class k053260_intf;	// k053260 specific interface

	private:
		const int pan_dir[8] = {-1, 0, 24, 35, 45, 55, 66, 90};	 // pan direction

		class voice_t : public vgsound_emu_core
		{
			public:
				// constructor
				voice_t(k053260_core &host)
					: vgsound_emu_core("k053260_voice")
					, m_host(host)
					, m_enable(0)
					, m_busy(0)
					, m_loop(0)
					, m_adpcm(0)
					, m_pitch(0)
					, m_start(0)
					, m_length(0)
					, m_volume(0)
					, m_pan(4)
					, m_counter(0)
					, m_addr(0)
					, m_remain(0)
					, m_bitpos(4)
					, m_data(0)
					, m_adpcm_buf(0)
					, m_out{0}
				{
				}

				// internal state
				void reset();
				void tick();

				// accessors
				void write(const u8 address, const u8 data);
				void keyon();
				void keyoff();

				// setters
				inline void set_enable(const bool enable) { m_enable = boolmask<u16>(enable); }

				inline void set_busy(const bool busy) { m_busy = boolmask<u16>(busy); }

				inline void set_loop(const bool loop) { m_loop = boolmask<u16>(loop); }

				inline void set_adpcm(const bool adpcm) { m_adpcm = boolmask<u16>(adpcm); }

				inline void length_inc() { m_length = (m_length + 1) & 0xffff; }

				inline void set_pan(const u8 pan) { m_pan = pan & 7; }

				// getters
				inline bool enable() const { return m_enable; }

				inline bool busy() const { return m_busy; }

				inline u32 start() const { return m_start; }

				inline u16 length() const { return m_length; }

				inline s32 out(const u8 ch) const { return m_out[ch & 1]; }

			private:
				// registers
				k053260_core &m_host;
				u16 m_enable : 1;				 // enable flag
				u16 m_busy	 : 1;				 // busy status
				u16 m_loop	 : 1;				 // loop flag
				u16 m_adpcm	 : 1;				 // ADPCM flag
				u16 m_pitch	 : 12;				 // pitch
				u32 m_start				 = 0;	 // start position
				u16 m_length			 = 0;	 // source length
				u8 m_volume				 = 0;	 // master volume
				u8 m_pan				 = 4;	 // master pan
				u16 m_counter			 = 0;	 // frequency counter
				u32 m_addr				 = 0;	 // current address
				s32 m_remain			 = 0;	 // remain for end sample
				u8 m_bitpos				 = 4;	 // bit position for ADPCM decoding
				u8 m_data				 = 0;	 // current data
				s8 m_adpcm_buf			 = 0;	 // ADPCM buffer
				std::array<s32, 2> m_out = {0};	 // current output
		};

		class ctrl_t : public vgsound_emu_core
		{
			public:
				ctrl_t()
					: vgsound_emu_core("k053260_ctrl")
					, m_rom_read(0)
					, m_sound_en(0)
					, m_input_en(0)
				{
				}

				inline void reset()
				{
					m_rom_read = 0;
					m_sound_en = 0;
					m_input_en = 0;
				}

				inline void write(const u8 data)
				{
					m_rom_read = bitfield(data, 0);
					m_sound_en = bitfield(data, 1);
					m_input_en = bitfield(data, 2, 2);
				}

				// getters
				inline bool rom_read() { return m_rom_read; }

				inline bool sound_en() { return m_sound_en; }

				inline u8 input_en() const { return m_input_en; }

			private:
				u8 m_rom_read : 1;	// ROM readback
				u8 m_sound_en : 1;	// Sound enable
				u8 m_input_en : 2;	// Input enable
				u8 m_dummy	  : 4;	// dummy
		};

		class ym3012_t : public vgsound_emu_core
		{
			public:
				ym3012_t()
					: vgsound_emu_core("k053260_ym3012_out")
					, m_in{0}
					, m_out{0}
				{
				}

				inline void reset()
				{
					m_in.fill(0);
					m_out.fill(0);
				}

				inline void tick(const u8 _ch, const s32 in)
				{
					const u8 ch	 = bitfield(_ch, 0);
					m_out[ch]	 = m_in[ch];
					m_in[ch ^ 1] = in;
				}

			private:
				std::array<s32, 2> m_in	 = {0};
				std::array<s32, 2> m_out = {0};
		};

		class dac_t : public vgsound_emu_core
		{
			public:
				dac_t()
					: vgsound_emu_core("k053260_dac_intf")
					, m_clock(0)
					, m_state(0)
				{
				}

				inline void reset()
				{
					m_clock = 0;
					m_state = 0;
				}

				inline void set_clock(const u8 clock) { m_clock = bitfield(clock, 0, 4); }

				inline void set_state(const u8 state) { m_state = bitfield(state, 0, 2); }

				inline u8 clock() const { return m_clock; }

				inline u8 state() const { return m_state; }

			private:
				u8 m_clock : 4;	 // DAC clock (16 clock)
				u8 m_state : 2;	 // DAC state (4 state - SAM1, SAM2)
				u8 m_dummy : 2;	 // Dummy
		};

	public:
		// constructor
		k053260_core(k053260_intf &intf)
			: vgsound_emu_core("k053260")
			, m_voice{*this, *this, *this, *this}
			, m_intf(intf)
			, m_host2snd{0}
			, m_snd2host{0}
			, m_ctrl(ctrl_t())
			, m_ym3012(ym3012_t())
			, m_dac(dac_t())
			, m_reg{0}
			, m_out{0}
		{
			// generate pan LUT
			for (int p = 0; p < 8; p++)
			{
				m_pan_lut[p][0] = (pan_dir[p] < 0) ? 0 : s32(cos(f64(p) * PI / 180) * 32767.0);
				m_pan_lut[p][1] = (pan_dir[p] < 0) ? 0 : s32(sin(f64(p) * PI / 180) * 32767.0);
			}
		}

		// communications
		inline u8 snd2host_r(const u8 address) const { return m_snd2host[address & 1]; }

		inline void host2snd_w(const u8 address, const u8 data) { m_host2snd[address & 1] = data; }

		// sound accessors
		u8 read(const u8 address);
		void write(const u8 address, const u8 data);

		// internal state
		void reset();
		void tick();

		template<typename T>
		void tick_stream(const std::size_t stream_len, T **out)
		{
			for (std::size_t s = 0; s < stream_len; s++)
			{
				tick();
				out[0][s] = this->output(0);
				out[1][s] = this->output(1);
			};
		}

		// getters for debug, trackers, etc
		inline s32 output(const u8 ch) const { return m_out[ch & 1]; }	// output for each channels

		inline u8 reg_r(const u8 address) const { return m_reg[address & 0x3f]; }

		inline s32 voice_out(const u8 voice, const u8 ch) const
		{
			return (voice < 4) ? m_voice[voice].out(ch & 1) : 0;
		}

	protected:
		inline s32 pan_lut(const u8 pan, const u8 out) { return m_pan_lut[pan][out]; }

	private:
		std::array<voice_t, 4> m_voice;
		std::array<std::array<s32, 2>, 8> m_pan_lut;
		k053260_intf &m_intf;  // common memory interface

		std::array<u8, 2> m_host2snd = {0};
		std::array<u8, 2> m_snd2host = {0};

		ctrl_t m_ctrl;	// chip control

		ym3012_t m_ym3012;	// YM3012 output
		dac_t m_dac;		// YM3012 interface

		std::array<u8, 64> m_reg = {0};	 // register pool
		std::array<s32, 2> m_out = {0};	 // stereo output
};

#endif
