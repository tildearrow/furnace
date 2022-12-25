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
					, m_pan(-1)
					, m_counter(0)
					, m_addr(0)
					, m_remain(0)
					, m_bitpos(4)
					, m_data(0)
					, m_adpcm_buf(0)
				{
					m_out.fill(0);
				}

				// internal state
				void reset();
				void tick();

				// accessors
				void write(u8 address, u8 data);
				void keyon();
				void keyoff();

				// setters
				inline void set_enable(bool enable) { m_enable = enable ? 1 : 0; }

				inline void set_busy(bool busy) { m_busy = busy ? 1 : 0; }

				inline void set_loop(bool loop) { m_loop = loop ? 1 : 0; }

				inline void set_adpcm(bool adpcm) { m_adpcm = adpcm ? 1 : 0; }

				inline void length_inc() { m_length = (m_length + 1) & 0xffff; }

				inline void set_pan(u8 pan) { m_pan = m_host.pan_dir[pan & 7]; }

				// getters
				inline bool enable() { return m_enable; }

				inline bool busy() { return m_busy; }

				inline u32 start() { return m_start; }

				inline u16 length() { return m_length; }

				inline s32 out(u8 ch) { return m_out[ch & 1]; }

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
				int m_pan				 = -1;	 // master pan
				u16 m_counter			 = 0;	 // frequency counter
				u32 m_addr				 = 0;	 // current address
				s32 m_remain			 = 0;	 // remain for end sample
				u8 m_bitpos				 = 4;	 // bit position for ADPCM decoding
				u8 m_data				 = 0;	 // current data
				s8 m_adpcm_buf			 = 0;	 // ADPCM buffer
				std::array<s32, 2> m_out;	 // current output
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

	public:
		// constructor
		k053260_core(k053260_intf &intf)
			: vgsound_emu_core("k053260")
			, m_voice{*this, *this, *this, *this}
			, m_intf(intf)
			, m_ctrl(ctrl_t())
			, m_ym3012(ym3012_t())
			, m_dac(dac_t())
		{
			m_host2snd.fill(0);
			m_snd2host.fill(0);
			m_reg.fill(0);
			m_out.fill(0);
		}

		// communications
		inline u8 snd2host_r(u8 address) { return m_snd2host[address & 1]; }

		inline void host2snd_w(u8 address, u8 data) { m_host2snd[address & 1] = data; }

		// sound accessors
		u8 read(u8 address);
		void write(u8 address, u8 data);

		// internal state
		void reset();
		void tick();

		// getters for debug, trackers, etc
		inline s32 output(u8 ch) { return m_out[ch & 1]; }	// output for each channels

		inline u8 reg_r(u8 address) { return m_reg[address & 0x3f]; }

		inline s32 voice_out(u8 voice, u8 ch)
		{
			return (voice < 4) ? m_voice[voice].out(ch & 1) : 0;
		}

	private:
		std::array<voice_t, 4> m_voice;
		k053260_intf &m_intf;  // common memory interface

		std::array<u8, 2> m_host2snd;
		std::array<u8, 2> m_snd2host;

		ctrl_t m_ctrl;	// chip control

		ym3012_t m_ym3012;	// YM3012 output
		dac_t m_dac;		// YM3012 interface

		std::array<u8, 64> m_reg;	 // register pool
		std::array<s32, 2> m_out;	 // stereo output
};

#endif
