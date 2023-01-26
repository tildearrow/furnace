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
#include "../core/util/mem_intf.hpp"

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
					, m_enable(false)
					, m_pitch(0)
					, m_volume(0)
					, m_addr(0)
					, m_counter(0)
					, m_out(0)
				{
					m_wave.fill(0);
				}

				// internal state
				void reset();
				void tick();

				// accessors
				inline void reset_addr() { m_addr = 0; }

				// setters
				inline void set_wave(u8 addr, s8 wave) { m_wave[addr & 0x1f] = wave; }

				inline void set_enable(bool enable) { m_enable = enable; }

				inline void set_pitch(u16 pitch, u16 mask = 0xfff)
				{
					m_pitch	  = (m_pitch & ~(mask & 0xfff)) | (pitch & (mask & 0xfff));
					m_counter = m_pitch;
				}

				inline void set_volume(u8 volume) { m_volume = volume & 0xf; }

				// getters
				inline s8 wave(u8 addr) { return m_wave[addr & 0x1f]; }

				inline u8 addr() { return m_addr; }

				inline s32 out() { return m_out; }

			private:
				// registers
				scc_core &m_host;
				std::array<s8, 32> m_wave;	// internal waveform
				bool m_enable			  = false;	// output enable flag
				u16 m_pitch	 : 12;					// pitch
				u16 m_volume : 4;					// volume
				u8 m_addr	  = 0;					// waveform pointer
				u16 m_counter = 0;					// frequency counter
				s32 m_out	  = 0;					// current output
		};

		class test_t : public vgsound_emu_core
		{
			public:
				// constructor
				test_t()
					: vgsound_emu_core("scc_test")
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
				inline void set_freq_4bit(bool freq_4bit) { m_freq_4bit = freq_4bit; }

				inline void set_freq_8bit(bool freq_8bit) { m_freq_8bit = freq_8bit; }

				inline void set_resetpos(bool resetpos) { m_resetpos = resetpos; }

				inline void set_rotate(bool rotate) { m_rotate = rotate; }

				inline void set_rotate4(bool rotate4) { m_rotate4 = rotate4; }

				// getters
				inline bool freq_4bit() { return m_freq_4bit; }

				inline bool freq_8bit() { return m_freq_8bit; }

				inline bool resetpos() { return m_resetpos; }

				inline bool rotate() { return m_rotate; }

				inline bool rotate4() { return m_rotate4; }

			private:
				u8 m_freq_4bit : 1;	 // 4 bit frequency
				u8 m_freq_8bit : 1;	 // 8 bit frequency
				u8 m_resetpos  : 1;	 // reset counter after pitch writes
				u8 m_rotate	   : 1;	 // rotate and write protect waveform for all channels
				u8 m_rotate4   : 1;	 // same as above but for channel 4 only
		};

	public:
		// constructor
		scc_core(std::string tag)
			: vgsound_emu_core(tag)
			, m_voice{*this, *this, *this, *this, *this}
			, m_test(test_t())
			, m_out(0)
		{
			m_reg.fill(0);
		}

		// destructor
		virtual ~scc_core() {}

		// accessors
		virtual u8 scc_r(bool is_sccplus, u8 address)			 = 0;
		virtual void scc_w(bool is_sccplus, u8 address, u8 data) = 0;

		// internal state
		virtual void reset();
		void tick();

		// getters
		inline s32 out() { return m_out; }	// output to DA0...DA10 pin

		inline u8 reg(u8 address) { return m_reg[address]; }

		// for preview
		inline s32 voice_out(u8 voice) { return (voice < 5) ? m_voice[voice].out() : 0; }

	protected:
		// accessor
		u8 wave_r(bool is_sccplus, u8 address);
		void wave_w(bool is_sccplus, u8 address, u8 data);
		void freq_vol_enable_w(u8 address, u8 data);

		// internal values
		std::array<voice_t, 5> m_voice;	 // 5 voices

		test_t m_test;					// test register
		s32 m_out				  = 0;	// output to DA0...10

		std::array<u8, 256> m_reg;  // register pool
};

// SCC core
class k051649_scc_core : public scc_core
{
	public:
		// constructor
		k051649_scc_core(std::string tag = "k051649_scc")
			: scc_core(tag)
		{
		}

		// accessors
		virtual u8 scc_r(bool is_sccplus, u8 address) override;
		virtual void scc_w(bool is_sccplus, u8 address, u8 data) override;
};

class k052539_scc_core : public k051649_scc_core
{
	public:
		// constructor
		k052539_scc_core(std::string tag = "k052539_scc")
			: k051649_scc_core(tag)
		{
		}

		// accessors
		virtual u8 scc_r(bool is_sccplus, u8 address) override;
		virtual void scc_w(bool is_sccplus, u8 address, u8 data) override;
};

// MegaROM Mapper with SCC
class k051649_core : public k051649_scc_core
{
		friend class vgsound_emu_mem_intf;	// for megaROM mapper

	private:
		// mapper classes
		class k051649_mapper_t : public vgsound_emu_core
		{
			public:
				k051649_mapper_t()
					: vgsound_emu_core("k051649_mapper")
					, m_bank{0, 1, 2, 3}
				{
				}

				// internal state
				void reset();

				// setters
				inline void set_bank(u8 slot, u8 bank) { m_bank[slot & 3] = bank; }

				// getters
				inline u8 bank(u8 slot) { return m_bank[slot & 3]; }

			private:
				// registers
				u8 m_bank[4] = {0, 1, 2, 3};
		};

	public:
		// constructor
		k051649_core(vgsound_emu_mem_intf &intf)
			: k051649_scc_core("k051649")
			, m_intf(intf)
			, m_mapper(k051649_mapper_t())
			, m_scc_enable(false)
		{
		}

		// accessors
		u8 read(u16 address);
		void write(u16 address, u8 data);

		virtual void reset() override;

	private:
		vgsound_emu_mem_intf m_intf;
		k051649_mapper_t m_mapper;
		bool m_scc_enable = false;
};

// MegaRAM Mapper with SCC
class k052539_core : public k052539_scc_core
{
		friend class vgsound_emu_mem_intf;	// for megaRAM mapper

	private:
		// mapper classes
		class k052539_mapper_t : public vgsound_emu_core
		{
			public:
				k052539_mapper_t()
					: vgsound_emu_core("k052539_mapper")
				{
					m_bank[0] = 0;
					m_bank[1] = 1;
					m_bank[2] = 2;
					m_bank[3] = 3;
					m_ram_enable.fill(false);
				}

				// internal state
				void reset();

				// setters
				inline void set_bank(u8 slot, u8 bank) { m_bank[slot & 3] = bank; }

				inline void set_ram_enable(u8 slot, bool ram_enable)
				{
					m_ram_enable[slot & 3] = ram_enable;
				}

				// getters
				inline u8 bank(u8 slot) { return m_bank[slot & 3]; }

				inline bool ram_enable(u8 slot) { return m_ram_enable[slot & 3]; }

			private:
				// registers
				std::array<u8, 4> m_bank;
				std::array<bool, 4> m_ram_enable;
		};

	public:
		// constructor
		k052539_core(vgsound_emu_mem_intf &intf)
			: k052539_scc_core("k052539")
			, m_intf(intf)
			, m_mapper(k052539_mapper_t())
			, m_scc_enable(false)
			, m_is_sccplus(false)
		{
		}

		// accessors
		u8 read(u16 address);
		void write(u16 address, u8 data);

		virtual void reset() override;

	private:
		vgsound_emu_mem_intf m_intf;
		k052539_mapper_t m_mapper;
		bool m_scc_enable = false;
		bool m_is_sccplus = false;
};

#endif
