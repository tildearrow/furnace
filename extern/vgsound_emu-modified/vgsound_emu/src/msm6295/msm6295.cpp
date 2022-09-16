/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	OKI MSM6295 emulation core
*/

#include "msm6295.hpp"

void msm6295_core::tick()
{
	if (m_counter < 4)
	{
		m_voice[m_counter].tick();
		m_out_temp += m_voice[m_counter].out();
	}
	if ((++m_counter) >= (m_ss ? 5 : 4))
	{
		// command handler
		if (m_command_pending)
		{
			if (bitfield(m_command, 7))	 // play voice
			{
				if ((++m_clock) >= 15)
				{
					if (bitfield(m_next_command, 4, 4) != 0)
					{
						for (int i = 0; i < 4; i++)
						{
							if (bitfield(m_next_command, 4 + i))
							{
								if (!m_voice[i].busy())
								{
									m_voice[i].set_command(m_command);
									m_voice[i].set_volume(bitfield(m_next_command, 0, 4));
								}
								// voices aren't be playable simultaneously at once
								break;
							}
						}
					}
					m_command		  = 0;
					m_command_pending = false;
					m_clock			  = 0;
				}
			}
			else if (bitfield(m_next_command, 7))  // select phrase
			{
				if ((++m_clock) >= 15)
				{
					m_command		  = m_next_command;
					m_command_pending = false;
					m_clock			  = 0;
				}
			}
			else
			{
				if (bitfield(m_next_command, 3, 4) != 0)  // suspend voices
				{
					for (int i = 0; i < 4; i++)
					{
						if (bitfield(m_next_command, 3 + i))
						{
							if (m_voice[i].busy())
							{
								m_voice[i].set_command(m_next_command);
							}
						}
					}
					m_next_command &= ~0x78;
				}
				m_command_pending = false;
			}
		}
		m_out	   = m_out_temp;
		m_out_temp = 0;
		m_counter  = 0;
	}
}

void msm6295_core::reset()
{
	for (auto &elem : m_voice)
	{
		elem.reset();
	}

	m_command		  = 0;
	m_next_command	  = 0;
	m_command_pending = false;
	m_clock			  = 0;
	m_counter		  = 0;
	m_out			  = 0;
	m_out_temp		  = 0;
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
			m_addr = (bitfield(m_host.m_intf.read_byte(phrase | 0), 0, 2) << 16) |
					 (m_host.m_intf.read_byte(phrase | 1) << 8) |
					 (m_host.m_intf.read_byte(phrase | 2) << 0);
			// End address
			m_end = (bitfield(m_host.m_intf.read_byte(phrase | 3), 0, 2) << 16) |
					(m_host.m_intf.read_byte(phrase | 4) << 8) |
					(m_host.m_intf.read_byte(phrase | 5) << 0);
			m_nibble  = 4;	// MSB first, LSB second
			m_command = 0;
			m_busy	  = true;
			vox_decoder_t::reset();
		}
		m_out = 0;
	}
	else
	{
		// playback
		if ((++m_clock) >= 33)
		{
			bool is_end = (m_command != 0);	 // suspend
			decode(bitfield(m_host.m_intf.read_byte(m_addr), m_nibble, 4));
			if (m_nibble <= 0)
			{
				m_nibble = 4;
				if (++m_addr > m_end)
				{
					is_end = true;
				}
			}
			else
			{
				m_nibble -= 4;
			}
			if (is_end)
			{
				m_command = 0;
				m_busy	  = false;
			}
			m_out	= (step() * m_volume) >> 7;	 // scale out to 12 bit output
			m_clock = 0;
		}
	}
}

void msm6295_core::voice_t::reset()
{
	vox_decoder_t::reset();
	m_clock	  = 0;
	m_busy	  = false;
	m_command = 0;
	m_addr	  = 0;
	m_nibble  = 0;
	m_end	  = 0;
	m_volume  = 0;
	m_out	  = 0;
	m_mute	  = false;
}

// accessors
u8 msm6295_core::busy_r()
{
	return (m_voice[0].busy() ? 0x01 : 0x00) | (m_voice[1].busy() ? 0x02 : 0x00) |
		   (m_voice[2].busy() ? 0x04 : 0x00) | (m_voice[3].busy() ? 0x08 : 0x00);
}

void msm6295_core::command_w(u8 data)
{
	if (!m_command_pending)
	{
		m_next_command	  = data;
		m_command_pending = true;
	}
}
