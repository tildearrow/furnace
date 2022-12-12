/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Texas instruments digital complex sound generators and variants emulation core
*/

#include "dcsg.hpp"

void dcsg_core::tick()
{
	if ((m_clock_divider <= 1) || ((++m_clock_counter) >= m_clock_divider))
	{
		output_exec();
		if (!m_ready)
		{
			if (m_write_pending)
			{
				write(m_addr_latch, m_reg_latch);
				m_write_pending = 0;
			}
			else
			{
				m_ready = 1;
				m_intf.ready_w(m_ready);
			}
		}
		m_clock_counter = 0;
	}
}

void dcsg_core::tick_perf()
{
	output_exec();
	if (!m_ready)
	{
		if (m_write_pending)
		{
			write(m_addr_latch, m_reg_latch);
			m_write_pending = 0;
		}
		else
		{
			m_ready = 1;
			m_intf.ready_w(m_ready);
		}
	}
}

void dcsg_core::output_exec()
{
	// tick per each clock
	for (auto &elem : m_square)
	{
		// add square output
		elem.get_output();
	}
	// add noise output
	m_noise.get_output();

	m_out = m_in;
	if (m_is_ngp)  // neo geo pocket
	{
		for (auto &elem : m_square)
		{
			// add square output
			m_lout += elem.lout();
			m_rout += elem.rout();
		}
		// add noise output
		m_lout += m_noise.lout();
		m_rout += m_noise.rout();
	}
	else if (m_is_gg)  // game gear
	{
		for (auto &elem : m_square)
		{
			// add square output
			if (elem.left_en())
			{
				m_lout += elem.out();
			}
			if (elem.right_en())
			{
				m_rout += elem.out();
			}
		}
		// add noise output
		if (m_noise.left_en())
		{
			m_lout += m_noise.out();
		}
		if (m_noise.right_en())
		{
			m_rout += m_noise.out();
		}
	}
	else
	{
		for (auto &elem : m_square)
		{
			m_out += elem.out();  // add square output
		}
		m_out += m_noise.out();	 // add noise output
	}
	if (m_negative_out)
	{
		m_out  = -m_out;
		m_lout = -m_lout;
		m_rout = -m_rout;
	}
}

void dcsg_core::reset()
{
	for (auto &elem : m_square)
	{
		elem.reset();
	}

	m_noise.reset();
	m_in			= 0;
	m_out			= 0;
	m_lout			= 0;
	m_rout			= 0;
	m_clock_counter = 0;
	m_addr_latch	= 0;
	m_reg_latch		= 0;
	m_write_pending = 0;
	m_ready			= 1;
	m_reg_voice.fill(m_is_sega ? 0 : 3);
}

bool dcsg_core::alu_t::tick()
{
	// carry handling
	const bool carry = (bitfield(--m_counter, 0, 10) == 0);
	if (carry)
	{
		counter_reset();
	}

	return carry;
}

bool dcsg_core::square_t::tick()
{
	if (dcsg_core::alu_t::tick())
	{
		m_pulse = bitfield(m_pulse++, 0);
	}
	return m_pulse;
}

bool dcsg_core::noise_t::tick()
{
	if (((m_shift != 3) && (bitfield(--m_fixed_count, 0, 4) == 0)) ||
		((m_shift == 3) && dcsg_core::alu_t::tick()))
	{
		if (bitfield(++m_pulse, (m_shift == 3) ? 0 : m_shift) == 0)
		{
			m_lfsr = (m_lfsr >> 1) | (m_host.calculate_noise(m_lfsr, m_mode) ? m_lfsr_feedback : 0);
		}
		m_fixed_count = 1 << 4;
	}
	return bitfield(m_lfsr, 0);
}

void dcsg_core::alu_t::reset()
{
	m_divider  = 0;
	m_counter  = 0;
	m_volume   = 0;
	m_lvolume  = 0;
	m_rvolume  = 0;
	m_out	   = 0;
	m_lout	   = 0;
	m_rout	   = 0;
	m_left_en  = 0;
	m_right_en = 0;
	m_pulse	   = 0;
}

void dcsg_core::noise_t::reset()
{
	dcsg_core::alu_t::reset();
	m_lfsr		  = 0;
	m_shift		  = 0;
	m_mode		  = 0;
	m_fixed_count = 1 << 4;
}

// Accessors
void dcsg_core::register_w(const u8 address, const u8 data)
{
	if (m_ready)
	{
		m_addr_latch	= address;
		m_reg_latch		= data;
		m_write_pending = 1;
		m_ready			= 0;
		m_intf.ready_w(m_ready);
	}
}

void dcsg_core::write(const u8 address, const u8 data)
{
	const u8 a = (!m_is_gg && !m_is_ngp) ? 0 : bitfield(address, 0);

	if (m_is_gg && a)
	{
		m_square[0].set_left_en(bitfield(data, 0));
		m_square[1].set_left_en(bitfield(data, 1));
		m_square[2].set_left_en(bitfield(data, 2));
		m_noise.set_left_en(bitfield(data, 3));
		m_square[0].set_right_en(bitfield(data, 4));
		m_square[1].set_right_en(bitfield(data, 5));
		m_square[2].set_right_en(bitfield(data, 6));
		m_noise.set_right_en(bitfield(data, 7));
	}
	else
	{
		if (bitfield(data, 7))
		{
			m_reg_voice[a] = bitfield(data, 5, 2);
			switch (bitfield(data, 4, 3))
			{
				case 0b000:	 // Square 0 Frequency
				case 0b001:	 // Square 0 Attenuation
				case 0b010:	 // Square 1 Frequency
				case 0b011:	 // Square 1 Attenuation
				case 0b100:	 // Square 2 Frequency
				case 0b101:	 // Square 2 Attenuation
					square_w(m_reg_voice[a], a, data);
					break;
				case 0b110:	 // Noise Control
				case 0b111:	 // Noise Attenuation
					noise_w(a, data);
					break;
			}
		}
		else
		{
			switch (m_reg_voice[a])
			{
				case 0b00:	// Square 0 Frequency MSB
				case 0b01:	// Square 1 Frequency MSB
				case 0b10:	// Square 2 Frequency MSB
					square_w(m_reg_voice[a], a, data);
					break;
				default: break;
			}
		}
	}
}

void dcsg_core::square_w(const u8 voice, const u8 address, const u8 data)
{
	square_t &v = m_square[voice];
	switch (data & 0x90)
	{
		case 0x80:	// Pitch LSB
			if (m_is_gg || (!m_is_ngp) || (bitfield(address, 0)))
			{
				v.set_divider(data, 0x00f);
			}
			if (voice == 2)
			{
				if (m_is_gg || (!m_is_ngp) || bitfield(~address, 0))
				{
					m_noise.set_divider(data, 0x00f);
				}
			}
			break;
		case 0x90:	// Volume
			if (m_is_gg || (!m_is_ngp))
			{
				v.set_volume(data);
			}
			else
			{
				if (bitfield(address, 0))
				{
					v.set_lvolume(data);
				}
				else
				{
					v.set_rvolume(data);
				}
			}
			break;
		case 0x00:
		case 0x10:	// Pitch MSB
			if (m_is_gg || (!m_is_ngp) || (bitfield(address, 0)))
			{
				v.set_divider(u16(data) << 4, 0x3f0);
			}
			if (voice == 2)
			{
				if (m_is_gg || (!m_is_ngp) || bitfield(~address, 0))
				{
					m_noise.set_divider(u16(data) << 4, 0x3f0);
				}
			}
			break;
	}
}

void dcsg_core::noise_w(const u8 address, const u8 data)
{
	switch (data & 0x90)
	{
		case 0x80:	// Noise control
			if ((!m_is_ngp) || bitfield(~address, 0))
			{
				m_noise.set_shift(bitfield(data, 0, 2));
				m_noise.set_mode(bitfield(data, 2));
				if (!m_is_ncr)
				{
					m_noise.clear_lfsr();
				}
			}
			break;
		case 0x90:	// Volume
			if (m_is_gg || (!m_is_ngp))
			{
				m_noise.set_volume(data);
			}
			else
			{
				if (bitfield(address, 0))
				{
					m_noise.set_lvolume(data);
				}
				else
				{
					m_noise.set_rvolume(data);
				}
			}
			break;
		default: break;
	}
}
