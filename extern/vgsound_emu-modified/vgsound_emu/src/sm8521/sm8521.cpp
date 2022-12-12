/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Sharp SM8521 sound emulation core
*/

#include "sm8521.hpp"

s8 sm8521_core::tick()
{
	m_out = 0;
	if (m_control.enable())	 // Enable flag
	{
		// tick per each clock
		if (m_control.dac_en())
		{
			m_out = m_dac;
		}
		else
		{
			if (m_control.sg0_en())
			{
				m_out += m_wave[0].get_output();
			}
			if (m_control.sg1_en())
			{
				m_out += m_wave[1].get_output();
			}
			if (m_control.sg2_en())
			{
				m_out += m_noise.get_output();
			}
			m_out = clamp<s16>(m_out, -0x80, 0x7f);
		}
	}
	return m_out;
}

void sm8521_core::reset()
{
	for (auto &elem : m_wave)
	{
		elem.reset();
	}

	m_noise.reset();
	m_control.reset();
	m_out = 0;
}

bool sm8521_core::alu_t::tick()
{
	// pre decrement
	m_counter = bitfield(--m_counter, 0, 12);

	// carry handling
	const bool carry = bitfield(m_counter, 0, 12) == 0;
	if (carry)
	{
		m_counter = m_divider.divider();
	}

	return carry;
}

s16 sm8521_core::wave_t::get_output()
{
	// add 4 bit waveform output
	if (tick())
	{
		m_counter = bitfield(++m_counter, 0, 5);
	}
	m_out = (sign_ext<s16>(nibble(m_counter), 4) * m_volume) >> 1;
	return m_out;
}

s16 sm8521_core::noise_t::get_output()
{
	// add 4 bit noise output
	if (tick())
	{
		m_lfsr = rand() & 0xf;	// TODO: unknown algorithm, needs to verification
	}
	m_out = (sign_ext<s16>(m_lfsr, 4) * m_volume) >> 1;
	return m_out;
}

void sm8521_core::alu_t::reset()
{
	m_divider.reset();
	m_out	  = 0;
	m_counter = 0;
	m_cycle	  = 0;
	m_volume  = 0;
}

void sm8521_core::wave_t::reset()
{
	sm8521_core::alu_t::reset();
	m_wave.fill(0);
	m_counter = 0;
}

void sm8521_core::noise_t::reset()
{
	sm8521_core::alu_t::reset();
	m_lfsr = 1;
}

// Accessors

void sm8521_core::alu_t::divider_t::write(const bool msb, const u8 data)
{
	if (msb)
	{
		m_divider = (m_divider & ~0xf00) | (bitfield<u32>(data, 0, 4) << 8);
	}
	else
	{
		m_divider = (m_divider & ~0x0ff) | data;
	}
}

// Global control - 0x0x40
// Volume - 0x42 (SG0), 0x44 (SG1), 0x4A (SG2)
// Pitch MSB - 0x46 (SG0), 0x48 (SG1), 0x4C (SG2)
// Pitch LSB - 0x47 (SG0), 0x49 (SG1), 0x4D (SG2)
// D/A direct output - 0x0x4E (write only)
// Waveform - 0x60-0x6f (SG0), 0x70-0x7f (SG1)
void sm8521_core::wave_w(const u8 voice, const u8 address, const u8 data)
{
	wave_t &v = m_wave[voice];
	switch (address)
	{
		case 0x42: v.set_volume(bitfield(data, 0, 5)); break;
		case 0x46: v.divider().write(true, data); break;
		case 0x47: v.divider().write(false, data); break;
	}
}

u8 sm8521_core::wave_r(const u8 voice, const u8 address)
{
	wave_t &v = m_wave[voice];
	switch (address)
	{
		case 0x42: return v.volume(); break;
		case 0x46: return bitfield(v.divider().divider(), 8, 4); break;
		case 0x47: return bitfield(v.divider().divider(), 0, 8); break;
	}
	return 0;
}

void sm8521_core::waveform_w(const u8 voice, const u8 address, const u8 data)
{
	m_wave[voice].set_byte(bitfield(address, 0, 4), data);
}

u8 sm8521_core::waveform_r(const u8 voice, const u8 address)
{
	return m_wave[voice].byte(bitfield(address, 0, 4));
}

void sm8521_core::noise_w(const u8 address, const u8 data)
{
	switch (address)
	{
		case 0x4a: m_noise.set_volume(bitfield(data, 0, 5)); break;
		case 0x4c: m_noise.divider().write(true, data); break;
		case 0x4d: m_noise.divider().write(false, data); break;
	}
}

u8 sm8521_core::noise_r(const u8 address)
{
	switch (address)
	{
		case 0x4a: return m_noise.volume(); break;
		case 0x4c: return bitfield(m_noise.divider().divider(), 8, 4); break;
		case 0x4d: return bitfield(m_noise.divider().divider(), 0, 8); break;
	}
	return 0;
}

void sm8521_core::control_w(const u8 data)
{
	const bool prev_sg0_en = m_control.sg0_en();
	const bool prev_sg1_en = m_control.sg1_en();
	const bool prev_sg2_en = m_control.sg2_en();
	m_control.write(data);
	if (!prev_sg0_en && m_control.sg0_en())
	{
		m_wave[0].clear_cycle();
		m_wave[0].clear_counter();
	}
	if (!prev_sg1_en && m_control.sg1_en())
	{
		m_wave[1].clear_cycle();
		m_wave[1].clear_counter();
	}
	if (!prev_sg2_en && m_control.sg2_en())
	{
		m_noise.clear_cycle();
		m_noise.clear_lfsr();
	}
}

u8 sm8521_core::control_r() { return m_control.byte(); }

void sm8521_core::dac_w(const u8 data) { m_dac = data; }
