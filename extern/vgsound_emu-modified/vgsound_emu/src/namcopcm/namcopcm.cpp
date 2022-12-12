/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Namco C140/C219 emulation core
*/

#include "namcopcm.hpp"

void c140_core::tick()
{
	for (int i = 0; i < max_voices(); i++)
	{
		m_voice[i].tick();
	}
}

void c219_core::tick()
{
	for (int i = 0; i < max_voices(); i++)
	{
		m_voice[i].tick();
	}
}

void c140_core::voice_t::tick()
{
	c140_core &host = (c140_core &)(m_host);
	if (m_voice_mode.busy())
	{
		// fetch sample
		s16 curr_sample = s16(host.intf().read_sample(m_bank, m_addr) & 0xfff0);
		s16 next_sample = s16(host.intf().read_sample(m_bank, m_addr + 1) & 0xfff0);
		if (m_voice_mode.compressed())
		{
			curr_sample = host.compressed_table(bitfield(curr_sample, 8, 8));
			next_sample = host.compressed_table(bitfield(next_sample, 8, 8));
		}
		// scale out samples
		curr_sample			  >>= 4;
		next_sample			  >>= 4;
		const s16 lerp_sample = curr_sample + (((next_sample - curr_sample) * m_frac) >> 15);
		// advance
		m_frac += m_freq;
		if (m_frac > 0x7fff)
		{
			m_addr += (m_frac >> 15);
			m_frac &= 0x7fff;
			if (m_addr >= m_end_addr)
			{
				if (m_voice_mode.loop())
				{
					m_addr = (m_addr + m_loop_addr) - m_end_addr;
				}
				else
				{
					m_voice_mode.set_busy(false);
					m_lout = 0;
					m_rout = 0;
					return;
				}
			}
		}
		m_lout = (lerp_sample * m_lvolume) >> 8;
		m_rout = (lerp_sample * m_rvolume) >> 8;
	}
	else
	{
		m_lout = 0;
		m_rout = 0;
	}
}

void c219_core::voice_t::tick()
{
	c219_core &host = (c219_core &)(m_host);
	if (m_voice_mode.busy())
	{
		// fetch sample
		s16 curr_sample =
		  m_voice_mode.noise()
			? s16(host.lfsr())
			: s16(bitfield(host.intf().read_sample(host.bank(m_bank_slot) + m_bank, m_addr),
						   bitfield(~m_frac, 15) << 3,
						   8));
		s16 next_sample = m_voice_mode.noise()
						  ? s16((host.lfsr() >> 1) ^ ((-(host.lfsr() & 1)) & 0xfff6))
						  : s16(bitfield(host.intf().read_sample(host.bank(m_bank_slot) + m_bank,
																 m_addr + bitfield(m_frac, 15)),
										 bitfield(m_frac, 15) << 3,
										 8));
		// scale out
		if (!m_voice_mode.noise())
		{
			if (m_voice_mode.compressed())
			{
				curr_sample = host.compressed_table(curr_sample);
				next_sample = host.compressed_table(next_sample);
			}
			else
			{
				curr_sample <<= 8;
				next_sample <<= 8;
			}
		}
		s16 lerp_sample = curr_sample + (((next_sample - curr_sample) * (m_frac & 0x7fff)) >> 15);
		if (m_voice_mode.invert())
		{
			lerp_sample = -lerp_sample;
		}
		// advance
		m_frac += m_freq;
		if (m_voice_mode.noise())
		{
			if (m_frac > 0x7fff)
			{
				m_frac &= 0x7fff;
				host.set_lfsr(bitfield(host.lfsr(), 1, 15) ^
							  ((-bitfield(host.lfsr(), 0)) & 0xfff6));
			}
		}
		else if (m_frac > 0xffff)
		{
			m_addr += (m_frac >> 16);
			m_frac &= 0xffff;
			if (m_addr >= m_end_addr)
			{
				if (m_voice_mode.loop())
				{
					m_addr = (m_addr + m_loop_addr) - m_end_addr;
				}
				else
				{
					m_voice_mode.set_busy(false);
					m_lout = 0;
					m_rout = 0;
					return;
				}
			}
		}
		m_lout = ((m_voice_mode.left_inv() ? -lerp_sample : lerp_sample) * m_lvolume) >> 12;
		m_rout = (lerp_sample * m_rvolume) >> 12;
	}
	else
	{
		m_lout = 0;
		m_rout = 0;
	}
}

void c140_core::write(const u16 address, const u16 data, const u16 mask)
{
	m_reg[address] = merge_data(m_reg[address], data, mask);
	if (bitfield(~address, 8))
	{
		const u16 voice = bitfield(address, 3, 5);
		if (voice < max_voices())
		{
			const u8 regs = bitfield(address, 0, 3);
			m_voice[voice].write(regs, data, mask);
		}
	}
	// Register 0x100 - 0x1ff unknown
}

u16 c140_core::read(const u16 address)
{
	if (bitfield(~address, 8))
	{
		const u16 voice = bitfield(address, 3, 5);
		if (voice < max_voices())
		{
			const u8 regs = bitfield(address, 0, 3);
			return m_voice[voice].read(regs);
		}
	}
	// Register 0x100 - 0x1ff unknown
	return m_reg[address];
}

// write registers on each voices
void c140_core::voice_t::write(const u8 address, const u16 data, const u16 mask)
{
	switch (address)
	{
		case 0:	 // Volume
			if (bitfield(mask, 0, 8) != 0)
			{
				set_lvolume(bitfield(data, 0, 8));
			}
			if (bitfield(mask, 8, 8) != 0)
			{
				set_rvolume(bitfield(data, 8, 8));
			}
			break;
		case 1:	 // Frequency
			set_freq(data, mask);
			break;
		case 2:	 // Bank/Mode
			if (bitfield(mask, 8, 8) != 0)
			{
				set_bank(bitfield(data, 8, 8));
			}
			if (bitfield(mask, 0, 8) != 0)
			{
				m_voice_mode.set_keyon(bitfield(data, 7));
				// bit 6 unknown writes
				m_voice_mode.set_loop(bitfield(data, 4));
				m_voice_mode.set_compressed(bitfield(data, 3));
				if (m_voice_mode.keyon())
				{
					m_addr = m_start_addr;
					m_frac = 0;

					m_voice_mode.set_busy(true);
					m_voice_mode.set_keyon(false);
				}
				else
				{
					m_voice_mode.set_busy(false);
				}
			}
			break;
		case 3:	 // Start address
			set_start_addr(data, mask);
			break;
		case 4:	 // Loop address
			set_loop_addr(data, mask);
			break;
		case 5:	 // End address
			set_end_addr(data, mask);
			break;
		default: break;
	}
}

u16 c140_core::voice_t::read(const u8 address)
{
	switch (address)
	{
		case 0:	 // Volume
			return lvolume() | (u16(rvolume()) << 8);
		case 1:	 // Frequency
			return freq();
		case 2:	 // Bank/Mode
			return (u16(bank()) << 8) | (busy() ? 0x40 : 0x00) | (loop() ? 0x10 : 0x00) |
				   (compressed() ? 0x08 : 0x00);
		case 3:	 // Start address
			return start_addr();
		case 4:	 // Loop address
			return loop_addr();
		case 5:	 // End address
			return end_addr();
		default: return 0;
	}
}

void c219_core::write(const u16 address, const u16 data, const u16 mask)
{
	m_reg[address] = merge_data(m_reg[address], data, mask);
	if (bitfield(~address, 8))
	{
		const u16 voice = bitfield(address, 3, 5);
		if (voice < max_voices())
		{
			const u8 regs = bitfield(address, 0, 3);
			m_voice[voice].write(regs, data, mask);
		}
		else if ((address & 0xf8) == 0xf8)
		{
			m_bank[bitfield(address, 0, 2)] =
			  merge_data(m_bank[bitfield(address, 0, 2)], data, bitfield(mask, 0, 2));
		}
	}
	// Register 0x100 - 0x1ff unknown
}

u16 c219_core::read(const u16 address)
{
	if (bitfield(~address, 8))
	{
		const u16 voice = bitfield(address, 3, 5);
		if (voice < max_voices())
		{
			const u8 regs = bitfield(address, 0, 3);
			return m_voice[voice].read(regs);
		}
	}
	// Register 0x100 - 0x1ff unknown
	return m_reg[address];
}

void c219_core::voice_t::write(const u8 address, const u16 data, const u16 mask)
{
	switch (address)
	{
		case 0:	 // Volume
			if (bitfield(mask, 0, 8) != 0)
			{
				set_lvolume(bitfield(data, 0, 8));
			}
			if (bitfield(mask, 8, 8) != 0)
			{
				set_rvolume(bitfield(data, 8, 8));
			}
			break;
		case 1:	 // Frequency
			set_freq(data, mask);
			break;
		case 2:	 // Bank/Mode
			if (bitfield(mask, 8, 8) != 0)
			{
				set_bank(bitfield(data, 8, 8));
			}
			if (bitfield(mask, 0, 8) != 0)
			{
				m_voice_mode.set_keyon(bitfield(data, 7));
				m_voice_mode.set_invert(bitfield(data, 6));
				m_voice_mode.set_loop(bitfield(data, 4));
				m_voice_mode.set_left_inv(bitfield(data, 3));
				m_voice_mode.set_noise(bitfield(data, 2));
				// bit 1 unknown writes
				m_voice_mode.set_compressed(bitfield(data, 0));
				if (m_voice_mode.keyon())
				{
					m_addr = m_start_addr;
					m_frac = 0;

					m_voice_mode.set_busy(true);
					m_voice_mode.set_keyon(false);
				}
				else
				{
					m_voice_mode.set_busy(false);
				}
			}
			break;
		case 3:	 // Start address
			set_start_addr(data, mask);
			break;
		case 4:	 // Loop address
			set_loop_addr(data, mask);
			break;
		case 5:	 // End address
			set_end_addr(data, mask);
			break;
		default: break;
	}
}

u16 c219_core::voice_t::read(const u8 address)
{
	switch (address)
	{
		case 0:	 // Volume
			return lvolume() | (u16(rvolume()) << 8);
		case 1:	 // Frequency
			return freq();
		case 2:	 // Bank/Mode
			return (u16(bank()) << 8) | (invert() ? 0x40 : 0x00) | (loop() ? 0x10 : 0x00) |
				   (left_inv() ? 0x08 : 0x00) | (noise() ? 0x04 : 0x00) |
				   (compressed() ? 0x01 : 0x00);
		case 3:	 // Start address
			return start_addr();
		case 4:	 // Loop address
			return loop_addr();
		case 5:	 // End address
			return end_addr();
		default: return 0;
	}
}

// reset chip
void namcopcm_core::reset() { m_reg.fill(0); }

void c140_core::reset()
{
	namcopcm_core::reset();
	for (auto &elem : m_voice)
	{
		elem.reset();
	}
}

void c219_core::reset()
{
	namcopcm_core::reset();
	for (auto &elem : m_voice)
	{
		elem.reset();
	}
	m_bank.fill(0);
	m_lfsr = 0x1234;
}

// reset voice
void namcopcm_core::voice_t::reset()
{
	m_voice_mode.reset();
	m_lvolume	 = 0;
	m_rvolume	 = 0;
	m_freq		 = 0;
	m_bank		 = 0;
	m_mode		 = 0;
	m_start_addr = 0;
	m_loop_addr	 = 0;
	m_end_addr	 = 0;
	m_addr		 = 0;
	m_frac		 = 0;
	m_lout		 = 0;
	m_rout		 = 0;
}
