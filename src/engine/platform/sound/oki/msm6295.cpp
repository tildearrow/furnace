/*
	License: BSD-3-Clause
	see https://github.com/cam900/vgsound_emu/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	OKI MSM6295 emulation core

	It is 4 channel ADPCM playback chip from OKI semiconductor.
	It was becomes de facto standard for ADPCM playback in arcade machine, due to cost performance.

	The chip itself is pretty barebone: there is no "register" in chip.
	It can't control volume and pitch in currently playing channels, only stopable them.
	And volume is must be determined at playback start command.

	Command format:

	Playback command (2 byte):

	Byte Bit      Description
	     76543210
	0    1xxxxxxx Phrase select (Header stored in ROM)
	1    x000---- Play channel 4
	     0x00---- Play channel 3
	     00x0---- Play channel 2
	     000x---- Play channel 1
	     ----xxxx Volume
	     ----0000 0.0dB
	     ----0001 -3.2dB
	     ----0010 -6.0dB
	     ----0011 -9.2dB
	     ----0100 -12.0dB
	     ----0101 -14.5dB
	     ----0110 -18.0dB
	     ----0111 -20.5dB
	     ----1000 -24.0dB

	Suspend command (1 byte):

	Byte Bit      Description
	     76543210
	0    0x------ Suspend channel 4
	     0-x----- Suspend channel 3
	     0--x---- Suspend channel 2
	     0---x--- Suspend channel 1

	Frequency calculation:
	if (SS) then
		Frequency = Input clock / 165
	else then
		Frequency = Input clock / 132

*/

#include "msm6295.hpp"

void msm6295_core::tick()
{
	// command handler
	if (m_command_pending)
	{
		if (bitfield(m_command, 7)) // play voice
		{
			if ((++m_clock) >= ((15 * (m_ss ? 5 : 4))))
			{
				m_clock = 0;
				if (bitfield(m_next_command, 4, 4) != 0)
				{
					for (int i = 0; i < 4; i++)
					{
						if (bitfield(m_next_command, 4 + i))
						{
							if (!m_voice[i].m_busy)
							{
								m_voice[i].m_command = m_command;
								m_voice[i].m_volume = (bitfield(m_next_command, 0, 4) < 9) ? m_volume_table[std::min<u8>(8, bitfield(m_next_command, 0, 4))] : 0;
							}
							break; // voices aren't be playable simultaneously at once
						}
					}
				}
				m_command = 0;
				m_command_pending = false;
			}
		}
		else if (bitfield(m_next_command, 7)) // select phrase
		{
			if ((++m_clock) >= ((15 * (m_ss ? 5 : 4))))
			{
				m_clock = 0;
				m_command = m_next_command;
				m_command_pending = false;
			}
		}
		else
		{
			if (bitfield(m_next_command, 3, 4) != 0) // suspend voices
			{
				for (int i = 0; i < 4; i++)
				{
					if (bitfield(m_next_command, 3 + i))
					{
						if (m_voice[i].m_busy)
							m_voice[i].m_command = m_next_command;
					}
				}
				m_next_command &= ~0x78;
			}
			m_command_pending = false;
		}
	}
	m_out = 0;
	for (int i = 0; i < 4; i++)
	{
		m_voice[i].tick();
		m_out += m_voice[i].m_out;
	}
}

void msm6295_core::reset()
{
	for (auto & elem : m_voice)
		elem.reset();

	m_command = 0;
	m_next_command = 0;
	m_command_pending = false;
	m_clock = 0;
	m_out = 0;
}

void msm6295_core::voice_t::tick()
{
	if (!m_busy)
	{
		if (bitfield(m_command, 7))
		{
			// get phrase header (stored in data memory)
			const u32 phrase = bitfield(m_command, 0, 7) << 3;
			// Start address
			m_addr = (bitfield(m_host.m_intf.read_byte(phrase | 0), 0, 2) << 16)
			       | (m_host.m_intf.read_byte(phrase | 1) << 8)
			       | (m_host.m_intf.read_byte(phrase | 2) << 0);
			// End address
			m_end = (bitfield(m_host.m_intf.read_byte(phrase | 3), 0, 2) << 16)
			      | (m_host.m_intf.read_byte(phrase | 4) << 8)
			      | (m_host.m_intf.read_byte(phrase | 5) << 0);
			m_nibble = 4; // MSB first, LSB second
			m_command = 0;
			m_busy = true;
			vox_decoder_t::reset();
		}
		m_out = 0;
	}
	else
	{
		// playback
		if ((++m_clock) >= ((33 * (m_host.m_ss ? 5 : 4))))
		{
			m_clock = 0;
			bool is_end = (m_command != 0);
			m_curr.decode(bitfield(m_host.m_intf.read_byte(m_addr), m_nibble, 4));
			if (m_nibble <= 0)
			{
				m_nibble = 4;
				if (++m_addr > m_end)
					is_end = true;
			}
			else
				m_nibble -= 4;
			if (is_end)
			{
				m_command = 0;
				m_busy = false;
			}
			m_out = (out() * m_volume) >> 7; // scale out to 12 bit output
		}
	}
}

void msm6295_core::voice_t::reset()
{
	vox_decoder_t::reset();
	m_clock = 0;
	m_busy = false;
	m_command = 0;
	m_addr = 0;
	m_nibble = 0;
	m_end = 0;
	m_volume = 0;
	m_out = 0;
}

// accessors
u8 msm6295_core::busy_r()
{
	return (m_voice[0].m_busy ? 0x01 : 0x00)
	     | (m_voice[1].m_busy ? 0x02 : 0x00)
	     | (m_voice[2].m_busy ? 0x04 : 0x00)
	     | (m_voice[3].m_busy ? 0x08 : 0x00);
}

void msm6295_core::command_w(u8 data)
{
	if (!m_command_pending)
	{
		m_next_command = data;
		m_command_pending = true;
	}
}
