/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Nanao GA20 core
*/

#include "ga20.hpp"

void ga20_core::tick()
{
	for (int i = 0; i < 4; i++)
	{
		m_voice[i].tick();
	}
}

s32 ga20_core::voice_t::tick()
{
	if (m_busy)
	{
		// update counter
		m_data = m_host.m_intf.read_byte(bitfield(m_addr, 0, 20));	// fetch ROM
		if ((m_data == 0) || (m_addr > (m_end | 0xf)))	// check end marker, address limits?
		{
			// loop?
			m_busy = 0;
			return 0;
		}
		else if ((bitfield(++m_counter, 0, 8) == 0))
		{
			m_counter = bitfield(m_pitch, 0, 8);
			m_addr++;
		}

		m_out = s32(s8(m_data) - 0x80) * m_volume;	// send to output pin
	}
	else
	{
		m_out = 0;
	}
	return m_out;
}

u8 ga20_core::read(const u8 address) const
{
	const u8 reg = bitfield(address, 0, 5);	 // 5 bit for CPU address
	if (bitfield(reg, 0, 3) == 7)			 // busy
	{
		return m_voice[bitfield(reg, 3, 2)].busy();
	}
	// unknown reads?
	return 0;
}

void ga20_core::write(const u8 address, const u8 data)
{
	const u8 reg = bitfield(address, 0, 5);	 // 5 bit for CPU address

	m_voice[bitfield(reg, 3, 2)].write(bitfield(reg, 0, 3), data);

	m_reg[reg] = data;
}

// write registers on each voices
void ga20_core::voice_t::write(const u8 address, const u8 data)
{
	switch (bitfield(address, 0, 3))
	{
		case 0:	 // start address bit 4-11
			m_start = (m_start & ~0x00fff) | (u32(data) << 4);
			break;
		case 1:	 // start address bit 12-19
			m_start = (m_start & ~0xff00f) | (u32(data) << 12);
			break;
		case 2:	 // end address bit 4-11
			m_end = (m_end & ~0x00ff0) | (u32(data) << 4);
			break;
		case 3:	 // end address bit 12-19
			m_end = (m_end & ~0xff000) | (u32(data) << 12);
			break;
		case 4:	 // pitch
			m_pitch = data;
			break;
		case 5:	 // volume
			m_volume = (data * 256) / (data + 10);
			break;
		case 6:	 // keyon trigger
			if (bitfield(data, 1))
			{
				keyon();
			}
			else
			{
				m_busy = false;
			}
			break;
		default:  // unknown registers
			break;
	}
}

// key on trigger
void ga20_core::voice_t::keyon()
{
	m_busy	  = 1;
	m_counter = bitfield(m_pitch, 0, 8);
	m_addr	  = m_start;
}

// reset chip
void ga20_core::reset()
{
	for (auto &elem : m_voice)
	{
		elem.reset();
	}

	m_reg.fill(0);
}

// reset voice
void ga20_core::voice_t::reset()
{
	m_counter = 0;
	m_pitch	  = 0;
	m_busy	  = 0;
	m_loop	  = 0;
	m_addr	  = 0;
	m_start	  = 0;
	m_end	  = 0xf;
	m_data	  = 0;
	m_out	  = 0;
	m_volume  = 0;
}
