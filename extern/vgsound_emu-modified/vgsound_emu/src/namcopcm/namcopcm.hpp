/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Namco C140/C219 emulation core
*/

#ifndef _VGSOUND_EMU_SRC_NAMCOPCM_HPP
#define _VGSOUND_EMU_SRC_NAMCOPCM_HPP

#pragma once

#include "../core/core.hpp"

using namespace vgsound_emu;

class namcopcm_intf : public vgsound_emu_core
{
	public:
		namcopcm_intf()
			: vgsound_emu_core("namcopcm_intf")
		{
		}

		// NE pin is executing voice number, and used for per-voice sample bank.
		virtual u16 read_sample(u8 bank, u16 address) { return 0; }
};

class namcopcm_core : public vgsound_emu_core
{
		friend class namcopcm_intf;	 // c140 specific interface

	protected:
		class voice_t : public vgsound_emu_core
		{
			protected:
				class voice_mode_t : public vgsound_emu_core
				{
					public:
						voice_mode_t()
							: vgsound_emu_core("c140_voice_mode")
							, m_keyon(0)
							, m_busy(0)
							, m_loop(0)
							, m_compressed(0)
							, m_noise(0)
							, m_invert(0)
							, m_left_inv(0)
						{
						}

						inline void reset()
						{
							m_keyon		 = 0;
							m_busy		 = 0;
							m_loop		 = 0;
							m_compressed = 0;
							m_noise		 = 0;
							m_invert	 = 0;
							m_left_inv	 = 0;
						}

						// setters
						inline void set_keyon(const bool keyon) { m_keyon = boolmask<u8>(keyon); }

						inline void set_busy(const bool busy) { m_busy = boolmask<u8>(busy); }

						inline void set_loop(const bool loop) { m_loop = boolmask<u8>(loop); }

						inline void set_compressed(const bool compressed)
						{
							m_compressed = boolmask<u8>(compressed);
						}

						inline void set_noise(const bool noise) { m_noise = boolmask<u8>(noise); }

						inline void set_invert(const bool invert)
						{
							m_invert = boolmask<u8>(invert);
						}

						inline void set_left_inv(const bool left_inv)
						{
							m_left_inv = boolmask<u8>(left_inv);
						}

						// getters
						inline bool keyon() const { return m_keyon; }

						inline bool busy() const { return m_busy; }

						inline bool loop() const { return m_loop; }

						inline bool compressed() const { return m_compressed; }

						inline bool noise() const { return m_noise; }

						inline bool invert() const { return m_invert; }

						inline bool left_inv() const { return m_left_inv; }

					private:
						u8 m_keyon		: 1;  // Key on/off flag (write only)
						u8 m_busy		: 1;  // Busy flag (read only)
						u8 m_loop		: 1;  // Loop flag
						u8 m_compressed : 1;  // Compressed sample flag
						// C219
						u8 m_noise	  : 1;	// Noise flag
						u8 m_invert	  : 1;	// Invert output
						u8 m_left_inv : 1;	// Invert left output
				};

			public:
				// constructor
				voice_t(std::string tag, namcopcm_core &host)
					: vgsound_emu_core(tag)
					, m_host(host)
					, m_voice_mode(voice_mode_t())
					, m_lvolume(0)
					, m_rvolume(0)
					, m_freq(0)
					, m_bank(0)
					, m_mode(0)
					, m_start_addr(0)
					, m_loop_addr(0)
					, m_end_addr(0)
					, m_addr(0)
					, m_frac(0)
					, m_lout(0)
					, m_rout(0)
				{
				}

				// internal state
				void reset();
				virtual void tick() = 0;

				// accessors
				virtual u16 read(const u8 address)										  = 0;
				virtual void write(const u8 address, const u16 data, const u16 mask = ~0) = 0;
				void keyon();

				// setters
				inline void set_lvolume(const u8 lvolume) { m_lvolume = lvolume; }

				inline void set_rvolume(const u8 rvolume) { m_rvolume = rvolume; }

				inline void set_freq(const u16 freq, const u16 mask = ~0)
				{
					m_freq = merge_data(m_freq, freq, mask);
				}

				inline void set_bank(const u8 bank) { m_bank = bank; }

				inline void set_mode(const u8 mode) { m_mode = mode; }

				inline void set_start_addr(const u16 start_addr, const u16 mask = ~0)
				{
					m_start_addr = merge_data(m_start_addr, start_addr, mask);
				}

				inline void set_loop_addr(const u16 loop_addr, const u16 mask = ~0)
				{
					m_loop_addr = merge_data(m_loop_addr, loop_addr, mask);
				}

				inline void set_end_addr(const u16 end_addr, const u16 mask = ~0)
				{
					m_end_addr = merge_data(m_end_addr, end_addr, mask);
				}

				inline void set_keyon(const bool keyon) { m_voice_mode.set_keyon(keyon); }

				inline void set_busy(const bool busy) { m_voice_mode.set_busy(busy); }

				inline void set_loop(const bool loop) { m_voice_mode.set_loop(loop); }

				inline void set_compressed(const bool compressed)
				{
					m_voice_mode.set_compressed(compressed);
				}

				inline void set_noise(const bool noise) { m_voice_mode.set_noise(noise); }

				inline void set_invert(const bool invert) { m_voice_mode.set_invert(invert); }

				inline void set_left_inv(const bool left_inv)
				{
					m_voice_mode.set_left_inv(left_inv);
				}

				// getters
				inline u8 lvolume() const { return m_lvolume; }

				inline u8 rvolume() const { return m_rvolume; }

				inline u16 freq() const { return m_freq; }

				inline u8 bank() const { return m_bank; }

				inline u8 mode() const { return m_mode; }

				inline u16 start_addr() const { return m_start_addr; }

				inline u16 loop_addr() const { return m_loop_addr; }

				inline u16 end_addr() const { return m_end_addr; }

				inline bool keyon() const { return m_voice_mode.keyon(); }

				inline bool busy() const { return m_voice_mode.busy(); }

				inline bool loop() const { return m_voice_mode.loop(); }

				inline bool compressed() const { return m_voice_mode.compressed(); }

				inline bool noise() const { return m_voice_mode.noise(); }

				inline bool invert() const { return m_voice_mode.invert(); }

				inline bool left_inv() const { return m_voice_mode.left_inv(); }

				inline s16 lout() const { return m_lout; }

				inline s16 rout() const { return m_rout; }

			protected:
				// host
				namcopcm_core &m_host;

				// registers
				voice_mode_t m_voice_mode;
				u8 m_lvolume	 = 0;  // Left volume
				u8 m_rvolume	 = 0;  // Right volume
				u16 m_freq		 = 0;  // Frequency
				u8 m_bank		 = 0;  // Bank
				u8 m_mode		 = 0;  // Voice mode
				u16 m_start_addr = 0;  // Start address
				u16 m_loop_addr	 = 0;  // Loop address
				u16 m_end_addr	 = 0;  // End address

				// Internal states
				u32 m_addr = 0;	 // Current address
				u32 m_frac = 0;	 // ALU fraction
				s16 m_lout = 0;	 // Left output
				s16 m_rout = 0;	 // Left output
		};

	public:
		namcopcm_intf &intf() { return m_intf; }

		inline s16 compressed_table(u8 sample) { return m_compressed_table[sample]; }

		// internal state
		virtual void reset();
		virtual void tick() = 0;

		template<typename T>
		void tick_stream(const std::size_t stream_len, T **out)
		{
		}

	protected:
		// constructor
		namcopcm_core(std::string tag, namcopcm_intf &intf)
			: vgsound_emu_core(tag)
			, m_intf(intf)
			, m_reg{0}
		{
		}

		virtual inline u8 max_voices() { return 24; }

		namcopcm_intf &m_intf;	// common memory interface

		std::array<s16, 256> m_compressed_table;
		std::array<u16, 512> m_reg;
};

class c140_core : public namcopcm_core
{
	private:
		class voice_t : public namcopcm_core::voice_t
		{
			public:
				voice_t(c140_core &host)
					: namcopcm_core::voice_t("c140_voice", host)
				{
				}

				virtual void tick() override;
				virtual u16 read(const u8 address) override;
				virtual void write(const u8 address, const u16 data, const u16 mask = ~0) override;
		};

	public:
		c140_core(namcopcm_intf &intf)
			: namcopcm_core("c140", intf)
			, m_voice{
				*this, *this, *this, *this, *this, *this, *this, *this, *this, *this, *this, *this,
				*this, *this, *this, *this, *this, *this, *this, *this, *this, *this, *this, *this,
			  }
		{
			for (int i = 0; i < 256; i++)
			{
				const u8 exponent = bitfield(i, 0, 3);
				const u8 mantissa = bitfield(i, 3, 4);
				if (bitfield(i, 7))
				{
					m_compressed_table[i] = s16(((exponent ? 0xfe00 : 0xff00) | (mantissa << 4))
												<< (exponent ? exponent - 1 : 0));
				}
				else
				{
					m_compressed_table[i] = s16(((exponent ? 0x100 : 0) | (mantissa << 4))
												<< (exponent ? exponent - 1 : 0));
				}
			}
		}

		virtual void reset() override;
		virtual void tick() override;

		u16 read(const u16 address);
		void write(const u16 address, const u16 data, const u16 mask = ~0);

	protected:
		// virtual function
		virtual inline u8 max_voices() override { return 24; }

	private:
		std::array<voice_t, 24> m_voice;
};

class c219_core : public namcopcm_core
{
	private:
		class voice_t : public namcopcm_core::voice_t
		{
			public:
				voice_t(c219_core &host, u8 bank_slot)
					: namcopcm_core::voice_t("c219_voice", host)
					, m_bank_slot(bank_slot)
				{
				}

				virtual void tick() override;
				virtual u16 read(const u8 address) override;
				virtual void write(const u8 address, const u16 data, const u16 mask = ~0) override;

			private:
				const u8 m_bank_slot = 0;
		};

	public:
		c219_core(namcopcm_intf &intf)
			: namcopcm_core("c219", intf)
			, m_voice{voice_t(*this, 3),
					  voice_t(*this, 3),
					  voice_t(*this, 3),
					  voice_t(*this, 3),
					  voice_t(*this, 0),
					  voice_t(*this, 0),
					  voice_t(*this, 0),
					  voice_t(*this, 0),
					  voice_t(*this, 1),
					  voice_t(*this, 1),
					  voice_t(*this, 1),
					  voice_t(*this, 1),
					  voice_t(*this, 2),
					  voice_t(*this, 2),
					  voice_t(*this, 2),
					  voice_t(*this, 2)}
		{
			int j = 0;
			for (int i = 0; i < 128; i++)
			{
				s32 compressed_sample = 0;
				if (i < 16)
				{
					compressed_sample = 0x20 * i;
				}
				else if (i < 24)
				{
					compressed_sample = (0x200 + (0x40 * i)) - 0x400;
				}
				else if (i < 48)
				{
					compressed_sample = (0x400 + (0x80 * i)) - 0xc00;
				}
				else if (i < 100)
				{
					compressed_sample = (0x1000 + (0x100 * i)) - 0x3000;
				}
				else
				{
					compressed_sample = (0x4400 + (0x200 * i)) - 0xc800;
				}
				m_compressed_table[i]		= compressed_sample;
				m_compressed_table[i + 128] = (~compressed_sample) & 0xffe0;
			}
		}

		// virtual functions
		virtual void reset() override;
		virtual void tick() override;

		u16 read(const u16 address);
		void write(const u16 address, const u16 data, const u16 mask = ~0);

	protected:
		// virtual function
		virtual inline u8 max_voices() override { return 16; }

		// setters
		inline void set_lfsr(u16 lfsr) { m_lfsr = lfsr; }

		// getters
		inline u8 bank(const u8 slot) const { return m_bank[slot & 3]; }

		inline u16 lfsr() { return m_lfsr; }

	private:
		std::array<voice_t, 16> m_voice;
		std::array<u8, 4> m_bank;
		u16 m_lfsr = 0x1234;
};

#endif
