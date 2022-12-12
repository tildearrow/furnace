/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Contributor(s): Natt Akuma, James Alan Nguyen, Laurens Holst
	Konami SCC emulation core
*/

#ifndef _VGSOUND_EMU_SRC_SCC_HPP
#define _VGSOUND_EMU_SRC_SCC_HPP

#pragma once

#include "../core/core.hpp"

using namespace vgsound_emu;

// shared for SCCs
class scc_core : public vgsound_emu_core
{
	private:
		// classes
		class voice_t : public vgsound_emu_core
		{
			public:
				// constructor
				voice_t(scc_core &host)
					: vgsound_emu_core("scc_voice")
					, m_host(host)
					, m_wave{0}
					, m_enable(0)
					, m_addr(0)
					, m_pitch(0)
					, m_volume(0)
					, m_counter(0)
					, m_out(0)
				{
				}

				// internal state
				void reset();
				void tick();

				// accessors
				inline void reset_addr() { m_addr = 0; }

				// setters
				inline void set_wave(const u8 addr, const s8 wave) { m_wave[addr & 0x1f] = wave; }

				inline void set_enable(const bool enable) { m_enable = boolmask<u8>(enable); }

				inline void set_pitch(const u16 pitch, const u16 mask = 0xfff)
				{
					m_pitch	  = (m_pitch & ~(mask & 0xfff)) | (pitch & (mask & 0xfff));
					m_counter = m_pitch;
				}

				inline void set_volume(const u8 volume) { m_volume = volume & 0xf; }

				// getters
				inline s8 wave(const u8 addr) const { return m_wave[addr & 0x1f]; }

				inline u8 addr() const { return m_addr; }

				inline s32 out() const { return m_out; }

			private:
				// registers
				scc_core &m_host;
				std::array<s8, 32> m_wave = {0};  // internal waveform
				u8 m_enable	 : 1;				  // output enable flag
				u8 m_addr	 : 5;				  // waveform pointer
				u8			 : 2;				  // Dummy
				u16 m_pitch	 : 12;				  // pitch
				u16 m_volume : 4;				  // volume
				u16 m_counter = 0;				  // frequency counter
				s32 m_out	  = 0;				  // current output
		};

		class test_t : public vgsound_emu_core
		{
			public:
				// constructor
				test_t(std::function<bool(u8, u8)> &write)
					: vgsound_emu_core("scc_test")
					, m_write(write)
					, m_freq_4bit(0)
					, m_freq_8bit(0)
					, m_resetpos(0)
					, m_rotate(0)
					, m_rotate4(0)
				{
				}

				void reset()
				{
					m_freq_4bit = 0;
					m_freq_8bit = 0;
					m_resetpos	= 0;
					m_rotate	= 0;
					m_rotate4	= 0;
				}

				// setters
				inline void set_freq_4bit(const bool freq_4bit)
				{
					m_freq_4bit = boolmask<u8>(freq_4bit);
				}

				inline void set_freq_8bit(const bool freq_8bit)
				{
					m_freq_8bit = boolmask<u8>(freq_8bit);
				}

				inline void set_resetpos(const bool resetpos)
				{
					m_resetpos = boolmask<u8>(resetpos);
				}

				inline void set_rotate(const bool rotate) { m_rotate = boolmask<u8>(rotate); }

				inline void set_rotate4(const bool rotate4) { m_rotate4 = boolmask<u8>(rotate4); }

				inline bool write(u8 address, u8 data) { return m_write(address, data); }

				// getters
				inline bool freq_4bit() const { return m_freq_4bit; }

				inline bool freq_8bit() const { return m_freq_8bit; }

				inline bool resetpos() const { return m_resetpos; }

				inline bool rotate() const { return m_rotate; }

				inline bool rotate4() const { return m_rotate4; }

			private:
				std::function<bool(u8, u8)> &m_write;
				u8 m_freq_4bit : 1;	 // 4 bit frequency
				u8 m_freq_8bit : 1;	 // 8 bit frequency
				u8 m_resetpos  : 1;	 // reset counter after pitch writes
				u8 m_rotate	   : 1;	 // rotate and write protect waveform for all channels
				u8 m_rotate4   : 1;	 // same as above but for channel 4 only
				u8			   : 3;	 // Dummy
		};

	public:
		// constructor
		scc_core(const std::string tag, std::function<bool(u8, u8)> &test_w)
			: vgsound_emu_core(tag)
			, m_voice{*this, *this, *this, *this, *this}
			, m_test(test_w)
			, m_out(0)
			, m_reg{0}
		{
		}

		// destructor
		virtual ~scc_core() {}

		// accessors
		virtual u8 scc_r(const bool is_sccplus, const u8 address) const;
		virtual void scc_w(const bool is_sccplus, const u8 address, const u8 data);

		// internal state
		virtual void reset();
		s32 tick();

		template<typename T>
		void tick_stream(const std::size_t stream_len, T *out)
		{
			for (std::size_t s = 0; s < stream_len; s++)
			{
				out[s] = tick();
			};
		}

		// getters
		inline s32 out() const { return m_out; }  // output to DA0...DA10 pin

		inline u8 reg(const u8 address) const { return m_reg[address]; }

		// for preview
		inline s32 voice_out(const u8 voice) const
		{
			return (voice < 5) ? m_voice[voice].out() : 0;
		}

	protected:
		// virtual constants
		virtual inline bool has_sccplus() const { return false; }

		test_t &test() { return m_test; }

	private:
		// accessor
		u8 wave_r(const bool is_sccplus, const u8 address) const;
		void wave_w(const bool is_sccplus, const u8 address, const u8 data);
		void freq_vol_enable_w(const u8 address, const u8 data);

		// internal values
		std::array<voice_t, 5> m_voice;	 // 5 voices

		test_t m_test;					// test register
		s32 m_out				  = 0;	// output to DA0...10

		std::array<u8, 256> m_reg = {0};  // register pool
};

// SCC core
class k051649_scc_core : public scc_core
{
	public:
		// constructor
		k051649_scc_core(const std::string tag = "k051649_scc")
			: k051649_scc_core(tag, m_test_w)
		{
		}

	protected:
		k051649_scc_core(const std::string tag, std::function<bool(u8, u8)> &test_w)
			: scc_core(tag, test_w)
		{
		}

	private:
		std::function<bool(u8, u8)> m_test_w = [this](u8 address, u8 data) -> bool
		{
			if (bitfield(address, 5, 3) == 0b111)  // 0xe0-0xff
			{
				test().set_freq_4bit(bitfield(data, 0));
				test().set_freq_8bit(bitfield(data, 1));
				test().set_resetpos(bitfield(data, 5));
				test().set_rotate(bitfield(data, 6));
				test().set_rotate4(bitfield(data, 7));
				return true;
			}
			return false;
		};
};

class k052539_scc_core : public k051649_scc_core
{
	public:
		// constructor
		k052539_scc_core(const std::string tag = "k052539_scc")
			: k051649_scc_core(tag, m_test_w)
		{
		}

	protected:
		virtual inline bool has_sccplus() const override { return true; }

	private:
		std::function<bool(u8, u8)> m_test_w = [this](u8 address, u8 data) -> bool
		{
			if (bitfield(address, 5, 3) == 0b110)  // 0xc0-0xdf
			{
				test().set_freq_4bit(bitfield(data, 0));
				test().set_freq_8bit(bitfield(data, 1));
				test().set_resetpos(bitfield(data, 5));
				test().set_rotate(bitfield(data, 6));
				return true;
			}
			return false;
		};
};

#endif
