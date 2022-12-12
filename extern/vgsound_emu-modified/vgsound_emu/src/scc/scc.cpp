/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Contributor(s): Natt Akuma, James Alan Nguyen, Laurens Holst
	Konami SCC emulation core
*/

#include "scc.hpp"

// shared SCC features
s32 scc_core::tick()
{
	m_out = 0;
	for (auto &elem : m_voice)
	{
		elem.tick();
		m_out += elem.out();
	}
	return m_out;
}

void scc_core::voice_t::tick()
{
	if (m_pitch >= 9)  // or voice is halted
	{
		// update counter - Post decrement
		const u16 temp = m_counter;
		if (m_host.test().freq_4bit())	// 4 bit frequency mode
		{
			m_counter = (m_counter & ~0x0ff) | (bitfield(bitfield(m_counter, 0, 8) - 1, 0, 8) << 0);
			m_counter = (m_counter & ~0xf00) | (bitfield(bitfield(m_counter, 8, 4) - 1, 0, 4) << 8);
		}
		else
		{
			m_counter = bitfield(m_counter - 1, 0, 12);
		}

		// handle counter carry
		const bool carry = m_host.test().freq_8bit()
						   ? (bitfield(temp, 0, 8) == 0)
						   : (m_host.test().freq_4bit() ? (bitfield(temp, 8, 4) == 0)
														: (bitfield(temp, 0, 12) == 0));
		if (carry)
		{
			m_addr	  = bitfield(m_addr + 1, 0, 5);
			m_counter = m_pitch;
		}
	}
	// get output
	if (m_enable)
	{
		m_out = (m_wave[m_addr] * m_volume) >> 4;  // scale to 11 bit digital output
	}
	else
	{
		m_out = 0;
	}
}

void scc_core::reset()
{
	for (auto &elem : m_voice)
	{
		elem.reset();
	}

	m_test.reset();
	m_out = 0;
	m_reg.fill(0);
}

void scc_core::voice_t::reset()
{
	m_wave.fill(0);
	m_enable  = 0;
	m_addr	  = 0;
	m_pitch	  = 0;
	m_volume  = 0;
	m_counter = 0;
	m_out	  = 0;
}

// SCC accessors
u8 scc_core::wave_r(const bool is_sccplus, const u8 address) const
{
	u8 ret		   = 0xff;
	const u8 voice = bitfield(address, 5, 3);
	if (voice > 4)
	{
		return ret;
	}

	u8 wave_addr = bitfield(address, 0, 5);

	if (m_test.rotate())
	{  // rotate flag
		wave_addr = bitfield(wave_addr + m_voice[voice].addr(), 0, 5);
	}

	if (!is_sccplus)
	{
		if (voice == 3)	 // rotate voice 3~4 flag
		{
			if (m_test.rotate4() || m_test.rotate())
			{  // rotate flag
				wave_addr =
				  bitfield(bitfield(address, 0, 5) + m_voice[3 + m_test.rotate()].addr(), 0, 5);
			}
		}
	}
	ret = m_voice[voice].wave(wave_addr);

	return ret;
}

void scc_core::wave_w(const bool is_sccplus, const u8 address, const u8 data)
{
	if (m_test.rotate())
	{  // write protected
		return;
	}

	const u8 voice = bitfield(address, 5, 3);
	if (voice > 4)
	{
		return;
	}

	const u8 wave_addr = bitfield(address, 0, 5);

	if (!is_sccplus)
	{
		if (((voice >= 3) && m_test.rotate4()) || (voice >= 4))
		{  // Ignore if write protected, or voice 4
			return;
		}
		if (voice >= 3)	 // voice 3, 4 shares waveform
		{
			m_voice[3].set_wave(wave_addr, data);
			m_voice[4].set_wave(wave_addr, data);
		}
		else
		{
			m_voice[voice].set_wave(wave_addr, data);
		}
	}
	else
	{
		m_voice[voice].set_wave(wave_addr, data);
	}
}

void scc_core::freq_vol_enable_w(const u8 address, const u8 data)
{
	// *0-*f Pitch, Volume, Enable
	const u8 reg		= bitfield(address, 0, 4);	// mask address to 4 bit
	const u8 voice_freq = bitfield(address, 1, 3);
	switch (reg)
	{
		case 0x0:  // 0x*0 Voice 0 Pitch LSB
		case 0x2:  // 0x*2 Voice 1 Pitch LSB
		case 0x4:  // 0x*4 Voice 2 Pitch LSB
		case 0x6:  // 0x*6 Voice 3 Pitch LSB
		case 0x8:  // 0x*8 Voice 4 Pitch LSB
			if (m_test.resetpos())
			{  // Reset address
				m_voice[voice_freq].reset_addr();
			}
			m_voice[voice_freq].set_pitch(data, 0x0ff);
			break;
		case 0x1:  // 0x*1 Voice 0 Pitch MSB
		case 0x3:  // 0x*3 Voice 1 Pitch MSB
		case 0x5:  // 0x*5 Voice 2 Pitch MSB
		case 0x7:  // 0x*7 Voice 3 Pitch MSB
		case 0x9:  // 0x*9 Voice 4 Pitch MSB
			if (m_test.resetpos())
			{  // Reset address
				m_voice[voice_freq].reset_addr();
			}
			m_voice[voice_freq].set_pitch(bitfield<u16>(data, 0, 4) << 8, 0xf00);
			break;
		case 0xa:  // 0x*a Voice 0 Volume
		case 0xb:  // 0x*b Voice 1 Volume
		case 0xc:  // 0x*c Voice 2 Volume
		case 0xd:  // 0x*d Voice 3 Volume
		case 0xe:  // 0x*e Voice 4 Volume
			m_voice[reg - 0xa].set_volume(bitfield(data, 0, 4));
			break;
		case 0xf:  // 0x*f Enable/Disable flag
			m_voice[0].set_enable(bitfield(data, 0));
			m_voice[1].set_enable(bitfield(data, 1));
			m_voice[2].set_enable(bitfield(data, 2));
			m_voice[3].set_enable(bitfield(data, 3));
			m_voice[4].set_enable(bitfield(data, 4));
			break;
	}
}

void scc_core::scc_w(const bool is_sccplus, const u8 address, const u8 data)
{
	const u8 voice = bitfield(address, 5, 3);
	if (!m_test.write(address, data))  // 0xc0-0xdf/0xe0-0xff Test register
	{
		if (is_sccplus && has_sccplus())
		{
			switch (voice)
			{
				case 0b000:	 // 0x00-0x1f Voice 0 Waveform
				case 0b001:	 // 0x20-0x3f Voice 1 Waveform
				case 0b010:	 // 0x40-0x5f Voice 2 Waveform
				case 0b011:	 // 0x60-0x7f Voice 3 Waveform
				case 0b100:	 // 0x80-0x9f Voice 4 Waveform
					wave_w(true, address, data);
					break;
				case 0b101:	 // 0xa0-0xbf Pitch, Volume, Enable
					freq_vol_enable_w(address, data);
					break;
				default: break;
			}
		}
		else
		{
			switch (voice)
			{
				case 0b000:	 // 0x00-0x1f Voice 0 Waveform
				case 0b001:	 // 0x20-0x3f Voice 1 Waveform
				case 0b010:	 // 0x40-0x5f Voice 2 Waveform
				case 0b011:	 // 0x60-0x7f Voice 3/4 Waveform
					wave_w(false, address, data);
					break;
				case 0b100:	 // 0x80-0x9f Pitch, Volume, Enable
					freq_vol_enable_w(address, data);
					break;
				default: break;
			}
		}
	}
	m_reg[address] = data;
}

u8 scc_core::scc_r(const bool is_sccplus, const u8 address) const
{
	const u8 voice = bitfield(address, 5, 3);
	const u8 wave  = bitfield(address, 0, 5);
	u8 ret		   = 0xff;
	if (is_sccplus && has_sccplus())
	{
		switch (voice)
		{
			case 0b000:	 // 0x00-0x1f Voice 0 Waveform
			case 0b001:	 // 0x20-0x3f Voice 1 Waveform
			case 0b010:	 // 0x40-0x5f Voice 2 Waveform
			case 0b011:	 // 0x60-0x7f Voice 3 Waveform
			case 0b100:	 // 0x80-0x9f Voice 4 Waveform
				ret = wave_r(true, address);
				break;
		}
	}
	else
	{
		switch (voice)
		{
			case 0b000:	 // 0x00-0x1f Voice 0 Waveform
			case 0b001:	 // 0x20-0x3f Voice 1 Waveform
			case 0b010:	 // 0x40-0x5f Voice 2 Waveform
			case 0b011:	 // 0x60-0x7f Voice 3 Waveform
			case 0b101:	 // 0xa0-0xbf Voice 4 Waveform
				ret = wave_r(false, (std::min<u8>(4, voice) << 5) | wave);
				break;
		}
	}
	return ret;
}
