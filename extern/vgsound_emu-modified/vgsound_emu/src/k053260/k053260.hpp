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

		//virtual void write_int(u8 out) {}  // timer interrupt
};

class k053260_core : public vgsound_emu_core
{
		friend class k053260_intf;	// k053260 specific interface

	private:
		const s32 m_pan_lut[8][2] = {
			{0x00, 0x00},
			{0x7f, 0x00},
			{0x74, 0x34},
			{0x68, 0x49},
			{0x5a, 0x5a},
			{0x49, 0x68},
			{0x34, 0x74},
			{0x00, 0x7f}
			};  // pan LUT

		const s8 m_adpcm_lut[16] =
			{0, 1, 2, 4, 8, 16, 32, 64, -128, -64, -32, -16, -8, -4, -2, -1};	 // ADPCM LUT

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
					, m_reverse(0)
					, m_start(0)
					, m_length(0)
					, m_volume(0)
					, m_pan(4)
					, m_counter(0)
					, m_addr(0)
					, m_remain(0)
					, m_bitpos(4)
					, m_data(0)
					, m_output(0)
				{
					std::fill_n(m_out, 2, 0);
				}

				// internal state
				void reset();
				void tick(u32 cycle);

				// accessors
				void write(u8 address, u8 data);
				void keyon();
				void keyoff();

				// setters
				inline void set_enable(bool enable) { m_enable = enable ? 1 : 0; }

				inline void set_busy(bool busy) { m_busy = busy ? 1 : 0; }

				inline void set_loop(bool loop) { m_loop = loop ? 1 : 0; }

				inline void set_adpcm(bool adpcm) { m_adpcm = adpcm ? 1 : 0; }

				inline void set_reverse(const bool reverse)
				{
					m_reverse = reverse ? 1 : 0;
				}

				inline void length_inc() { m_length = (m_length + 1) & 0xffff; }

				inline void set_pan(u8 pan) { m_pan = pan & 7; }

				// getters
				inline bool enable() { return m_enable; }

				inline bool busy() { return m_busy; }

				inline u32 start() { return m_start; }

				inline u16 length() { return m_length; }

				inline s32 out(u8 ch) { return m_out[ch & 1]; }

			private:
				// registers
				k053260_core &m_host;
				u16 m_enable  : 1;	 // enable flag
				u16 m_busy	  : 1;	 // busy status
				u16 m_loop	  : 1;	 // loop flag
				u16 m_adpcm	  : 1;	 // ADPCM flag
				u16 m_pitch	  : 12;	 // pitch
				u8 m_reverse  : 1;   // reverse playback
				u32 m_start	  = 0;	 // start position
				u16 m_length  = 0;	 // source length
				u8 m_volume	  = 0;	 // master volume
				s32 m_pan	  = 4;	 // master pan
				u16 m_counter = 0;	 // frequency counter
				u32 m_addr	  = 0;	 // current address
				s32 m_remain  = 0;	 // remain for end sample
				u8 m_bitpos	  = 4;	 // bit position for ADPCM decoding
				u8 m_data	  = 0;	 // current data
				s8 m_output   = 0;	 // ADPCM buffer
				s32 m_out[2];	 // current output
		};

		class ctrl_t
		{
			public:
				ctrl_t()
					: m_rom_read(0)
					, m_sound_en(0)
					, m_input_en(0)
				{
				}

				void reset()
				{
					m_rom_read = 0;
					m_sound_en = 0;
					m_input_en = 0;
				}

				void write(u8 data)
				{
					m_rom_read = (data >> 0) & 1;
					m_sound_en = (data >> 1) & 1;
					m_input_en = (data >> 2) & 3;
				}

				// getters
				inline bool rom_read() { return m_rom_read; }

				inline bool sound_en() { return m_sound_en; }

				inline u8 input_en() { return m_input_en; }

			private:
				u8 m_rom_read : 1;	// ROM readback
				u8 m_sound_en : 1;	// Sound enable
				u8 m_input_en : 2;	// Input enable
		};

		/*
		class ym3012_t
		{
			public:
				ym3012_t()
				{
					m_in.fill(0);
					m_out.fill(0);
				}

				void reset()
				{
					m_in.fill(0);
					m_out.fill(0);
				}

				void tick(u8 ch, s32 in)
				{
					m_out[(ch & 1)]	   = m_in[(ch & 1)];
					m_in[(ch & 1) ^ 1] = in;
				}

			private:
				std::array<s32, 2> m_in;
				std::array<s32, 2> m_out;
		};
		*/

		/*
		class dac_t
		{
			public:
				dac_t()
					: m_clock(0)
					, m_state(0)
				{
				}

				void reset()
				{
					m_clock = 0;
					m_state = 0;
				}

				inline void set_clock(u8 clock) { m_clock = clock; }

				inline void set_state(u8 state) { m_state = state; }

				inline u8 clock() { return m_clock; }

				inline u8 state() { return m_state; }

			private:
				u8 m_clock : 4;	 // DAC clock (16 clock)
				u8 m_state : 2;	 // DAC state (4 state - SAM1, SAM2)
		};
		*/

	public:
		// constructor
		k053260_core(k053260_intf &intf)
			: vgsound_emu_core("k053260")
			, m_voice{*this, *this, *this, *this}
			, m_intf(intf)
			, m_ctrl(ctrl_t())
			//, m_ym3012(ym3012_t())
			//, m_dac(dac_t())
		{
			std::fill_n(m_host2snd, 2, 0);
			std::fill_n(m_snd2host, 2, 0);
			std::fill_n(m_reg, 64, 0);
			std::fill_n(m_out, 2, 0);
		}

		// communications
		inline u8 snd2host_r(u8 address) { return m_snd2host[address & 1]; }

		inline void host2snd_w(u8 address, u8 data) { m_host2snd[address & 1] = data; }

		// sound accessors
		u8 read(u8 address);
		void write(u8 address, u8 data);

		// internal state
		void reset();
		void tick(u32 cycle);

		// getters for debug, trackers, etc
		inline s32 output(u8 ch) { return m_out[ch & 1]; }	// output for each channels

		inline u8 reg_r(u8 address) { return m_reg[address & 0x3f]; }

		inline s32 voice_out(u8 voice, u8 ch)
		{
			return (voice < 4) ? m_voice[voice].out(ch & 1) : 0;
		}

	protected:
		inline s32 pan_lut(const u8 pan, const u8 out) { return m_pan_lut[pan][out]; }

		inline s32 adpcm_lut(const u8 nibble) { return m_adpcm_lut[nibble]; }

	private:
		voice_t m_voice[4];
		k053260_intf &m_intf;  // common memory interface

		u8 m_host2snd[2];
		u8 m_snd2host[2];

		ctrl_t m_ctrl;	// chip control

		//ym3012_t m_ym3012;	// YM3012 output
		//dac_t m_dac;		// YM3012 interface

		u8 m_reg[64];	 // register pool
		s32 m_out[2];	 // stereo output
};

#endif
