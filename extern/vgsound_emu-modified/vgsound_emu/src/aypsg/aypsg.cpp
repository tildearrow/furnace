/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	General Instrument PSG and variants emulation core
*/

#include "aypsg.hpp"

void aypsg_core::tick()
{
	if (get_divider())
	{
		if ((m_clock_divider <= 1) || ((++m_clock_counter) >= m_clock_divider))
		{
			output_exec();
			m_clock_counter = 0;
		}
		m_pulse = 0;
	}
}

void aypsg_core::tick_perf() { output_exec(); }

void aypsg_core::output_exec()
{
	// tick per each cycle
	for (u8 c = 0; c < 16; c++)
	{
		for (square_t &elem : m_square)
		{
			elem.tick();
		}
	}
	for (envelope_t &elem : m_envelope)
	{
		elem.tick();
	}

	for (int i = 0; i < 3; i++)
	{
		const s32 volume = get_volume(i);
		square_t &voice	 = m_square[i];
		m_voice_out[i] =
		  (voice.pulse_out() | voice.tone()) & (noise().noise_out() | voice.noise()) ? volume : 0;
	}
	if (m_single_mix)
	{
		const s32 average = (m_voice_out[0] + m_voice_out[1] + m_voice_out[2]) / 3;
		m_out[0]		  = average;
		m_out[1]		  = 0;
		m_out[2]		  = 0;
	}
	else
	{
		std::copy(m_voice_out.begin(), m_voice_out.end(), m_out.begin());
	}
}

void aypsg_core::reset()
{
	for (square_t &elem : m_square)
	{
		elem.reset();
	}

	for (envelope_t &elem : m_envelope)
	{
		elem.reset();
	}

	m_noise.reset();
	m_voice_out.fill(0);
	m_out.fill(0);
	m_clock_counter = 0;
	m_pulse			= 0;
	m_reg_addr		= 0;
	m_half_div		= 0;
}

void ay8930_core::reset()
{
	aypsg_core::reset();
	m_test			= 0;
	m_expanded_mode = 0;
	m_bank			= 0;
}

bool aypsg_core::alu_t::tick()
{
	// carry handling
	const bool carry = (bitfield(--m_counter, 0, 16) == 0);
	if (carry)
	{
		counter_reset();
	}

	return carry;
}

bool aypsg_core::square_t::tick()
{
	m_update(aypsg_core::alu_t::tick(), m_index);
	return pulse_out();
}

bool aypsg_core::noise_t::tick()
{
	m_update(aypsg_core::alu_t::tick());
	return noise_out();
}

bool aypsg_core::envelope_t::tick()
{
	if (aypsg_core::alu_t::tick())
	{
		/*

		Envelope control
		Bits   Waveform
		3210
		00xx   \_______________
		01xx   /_______________
		1000   \\\\\\\\\\\\\\\\
		1001   \_______________
		1010   \/\/\/\/\/\/\/\/
		1011   \---------------
		1100   ////////////////
		1101   /---------------
		1110   /\/\/\/\/\/\/\/\
		1111   /_______________

		*/

		if (!m_host.is_expanded_mode() || bitfield(m_pulse++, 0))
		{
			// holding?
			if (!m_holding)
			{
				// advance
				m_env_out++;
				const bool carry = m_attack ? bitfield(m_env_out, 5) : bitfield(~m_env_out, 5);
				if (carry)
				{
					if (!m_continue)  // continue off, /_______ or \_______
					{
						m_env_out = 0x3f;
						m_holding = 1;
					}
					else  // continue on
					{
						if (m_hold)
						{
							m_env_out = (m_attack ^ m_alternate) ? 0x1f : 0x3f;
							m_holding = 1;
						}
						else if (!m_alternate)
						{
							m_env_out = (m_attack ? 0x00 : 0x20) | bitfield(m_env_out, 0, 5);
						}
					}
				}
			}
		}
	}
	return true;
}

void aypsg_core::alu_t::reset()
{
	m_divider = 0;
	m_counter = 0;
	m_pulse	  = 0;
}

void aypsg_core::square_t::reset()
{
	aypsg_core::alu_t::reset();
	m_volume	= 0;
	m_lvolume	= 0;
	m_rvolume	= 0;
	m_tone		= 0;
	m_noise		= 0;
	m_envelope	= 0;
	m_left_en	= 1;
	m_right_en	= 1;
	m_duty		= 4;
	m_pulse_out = 0;
}

void aypsg_core::noise_t::reset()
{
	aypsg_core::alu_t::reset();
	m_and_mask	 = 0;
	m_or_mask	 = 0;
	m_mask_value = 0;
	m_noise_out	 = 1;
	m_lfsr		 = 1;
}

void aypsg_core::envelope_t::reset()
{
	aypsg_core::alu_t::reset();
	m_env_out	= 0;
	m_holding	= 0;
	m_hold		= 0;
	m_alternate = 0;
	m_attack	= 0;
	m_continue	= 0;
}

void aypsg_core::io_t::reset()
{
	m_input	 = 0;
	m_io_sel = false;
}

// Accessors
u8 aypsg_core::muxed_r(const u8 address)
{
	if (bitfield(address, 0))
	{
		return data_r();
	}
	return 0xff;
}

void aypsg_core::muxed_w(const u8 address, const u8 data)
{
	if (bitfield(address, 0))
	{
		data_w(data);
	}
	else
	{
		addr_w(data);
	}
}

u8 aypsg_core::direct_r(const u8 address)
{
	addr_w(address);
	return data_r();
}

void aypsg_core::direct_w(const u8 address, const u8 data)
{
	addr_w(address);
	data_w(data);
}

void aypsg_core::addr_w(const u8 data) { set_reg_addr(bitfield(data, 0, 4)); }

// default register map
void aypsg_core::data_w(const u8 data)
{
	switch (reg_addr())
	{
		case 0:	 // Square 0 Fine tune
		case 1:	 // Square 0 Coarse tune
		case 2:	 // Square 1 Fine tune
		case 3:	 // Square 1 Coarse tune
		case 4:	 // Square 2 Fine tune
		case 5:	 // Square 2 Coarse tune
			{
				const u8 voice = std::min<u8>(2, bitfield(reg_addr(), 1, 2));
				const bool msb = bitfield(reg_addr(), 0);
				square(voice).set_divider(msb ? (u16(data) << 4) : data, msb ? 0x0f00 : 0x00ff);
				break;
			}
		case 6:	 // Noise Frequency
			noise().set_divider(bitfield(data, 0, 5), 0x001f);
			break;
		case 7:	 // Control
			control_enable_w(data);
			break;
		case 8:	  // Square 0 Amplitude
		case 9:	  // Square 1 Amplitude
		case 10:  // Square 2 Amplitude
			{
				const u8 voice = std::min<u8>(2, bitfield(reg_addr(), 0, 2));
				square(voice).set_envelope(bitfield(data, 4));
				square(voice).set_volume(bitfield(data, 0, 4));
				break;
			}
		case 11:  // Envelope Fine tune
		case 12:  // Envelope Coarse tune
			{
				const bool lsb = bitfield(reg_addr(), 0);
				envelope(0).set_divider(lsb ? data : (u16(data) << 4), lsb ? 0x00ff : 0xff00);
				break;
			}
		case 13:  // Envelope control
			envelope(0).ctrl_w(data);
			break;
		case 14:  // IO Port A
		case 15:  // IO Port B
			io_w(reg_addr() - 14, data);
			break;
		default: break;
	}
}

u8 aypsg_core::data_r()
{
	u8 ret = 0xff;
	switch (reg_addr())
	{
		case 0:	 // Square 0 Fine tune
		case 1:	 // Square 0 Coarse tune
		case 2:	 // Square 1 Fine tune
		case 3:	 // Square 1 Coarse tune
		case 4:	 // Square 2 Fine tune
		case 5:	 // Square 2 Coarse tune
			{
				const u8 voice = std::min<u8>(2, bitfield(reg_addr(), 1, 2));
				const bool msb = bitfield(reg_addr(), 0);
				ret			   = square(voice).divider() >> (msb ? 8 : 0);
				break;
			}
		case 6:	 // Noise Frequency
			ret = noise().divider();
			break;
		case 7:	 // Control
			ret = control_enable_r();
			break;
		case 8:	  // Square 0 Amplitude
		case 9:	  // Square 1 Amplitude
		case 10:  // Square 2 Amplitude
			{
				const u8 voice = std::min<u8>(2, bitfield(reg_addr(), 0, 2));
				ret			   = (square(voice).envelope() ? 0x10 : 0x00) | square(voice).volume();
				break;
			}
		case 11:  // Envelope Fine tune
		case 12:  // Envelope Coarse tune
			{
				const bool lsb = bitfield(reg_addr(), 0);
				ret			   = envelope(0).divider() >> (lsb ? 0 : 8);
				break;
			}
		case 13:  // Envelope control
			ret = envelope(0).ctrl_r();
			break;
		case 14:  // IO Port A
		case 15:  // IO Port B
			ret = io_r(reg_addr() - 14);
			break;
		default: break;
	}
	return ret;
}

// AY-3-8914: different register map and extended envelope volume
void ay_3_8914_core::data_w(const u8 data)
{
	switch (reg_addr())
	{
		case 0:	 // Square 0 Fine tune
		case 1:	 // Square 1 Fine tune
		case 2:	 // Square 2 Fine tune
		case 4:	 // Square 0 Coarse tune
		case 5:	 // Square 1 Coarse tune
		case 6:	 // Square 2 Coarse tune
			{
				const u8 voice = std::min<u8>(2, bitfield(reg_addr(), 0, 2));
				const bool msb = bitfield(reg_addr(), 2);
				square(voice).set_divider(msb ? (u16(data) << 4) : data, msb ? 0x0f00 : 0x00ff);
				break;
			}
		case 3:	 // Envelope Fine tune
		case 7:	 // Envelope Coarse tune
			{
				const bool msb = bitfield(reg_addr(), 2);
				envelope(0).set_divider(msb ? (u16(data) << 4) : data, msb ? 0xff00 : 0x00ff);
				break;
			}
		case 8:	 // Control
			control_enable_w(data);
			break;
		case 9:	 // Noise Frequency
			noise().set_divider(bitfield(data, 0, 5), 0x001f);
			break;
		case 10:  // Envelope control
			envelope(0).ctrl_w(data);
			break;
		case 11:  // Square 0 Amplitude
		case 12:  // Square 1 Amplitude
		case 13:  // Square 2 Amplitude
			{
				const u8 voice = std::min<u8>(2, bitfield(reg_addr() - 11, 0, 2));
				square(voice).set_envelope(bitfield(data, 4, 2));
				square(voice).set_volume(bitfield(data, 0, 4));
				break;
			}
		case 14:  // IO Port A
		case 15:  // IO Port B
			io_w(reg_addr() - 14, data);
			break;
		default: break;
	}
}

u8 ay_3_8914_core::data_r()
{
	u8 ret = 0xff;
	switch (reg_addr())
	{
		case 0:	 // Square 0 Fine tune
		case 1:	 // Square 1 Fine tune
		case 2:	 // Square 2 Fine tune
		case 4:	 // Square 0 Coarse tune
		case 5:	 // Square 1 Coarse tune
		case 6:	 // Square 2 Coarse tune
			{
				const u8 voice = std::min<u8>(2, bitfield(reg_addr(), 0, 2));
				const bool msb = bitfield(reg_addr(), 2);
				ret			   = square(voice).divider() >> (msb ? 8 : 0);
				break;
			}
		case 3:	 // Envelope Fine tune
		case 7:	 // Envelope Coarse tune
			{
				const bool msb = bitfield(reg_addr(), 2);
				ret			   = envelope(0).divider() >> (msb ? 8 : 0);
				break;
			}
		case 8:	 // Control
			ret = control_enable_r();
			break;
		case 9:	 // Noise Frequency
			ret = noise().divider();
			break;
		case 10:  // Envelope control
			ret = envelope(0).ctrl_r();
			break;
		case 11:  // Square 0 Amplitude
		case 12:  // Square 1 Amplitude
		case 13:  // Square 2 Amplitude
			{
				const u8 voice = std::min<u8>(2, bitfield(reg_addr() - 11, 0, 2));
				ret			   = (square(voice).envelope() << 4) | square(voice).volume();
				break;
			}
		case 14:  // IO Port A
		case 15:  // IO Port B
			ret = io_r(reg_addr() - 14);
			break;
		default: break;
	}
	return ret;
}

// AY8930
void ay8930_core::data_w(const u8 data)
{
	if (is_expanded_mode())
	{
		if (m_bank)
		{
			switch (reg_addr())
			{
				case 0:	 // Envelope 1 Fine tune
				case 1:	 // Envelope 1 Coarse tune
				case 2:	 // Envelope 2 Fine tune
				case 3:	 // Envelope 2 Coarse tune
					{
						const u8 voice = std::min<u8>(2, bitfield(reg_addr() + 1, 1, 2));
						const bool msb = bitfield(reg_addr(), 0);
						envelope(voice).set_divider(msb ? (u16(data) << 4) : data,
													msb ? 0xff00 : 0x00ff);
						break;
					}
				case 4:	 // Envelope 1 control
				case 5:	 // Envelope 2 control
					envelope((reg_addr() - 4) + 1).ctrl_w(data);
					break;
				case 6:	 // Square 0 Duty
				case 7:	 // Square 1 Duty
				case 8:	 // Square 2 Duty
					{
						const u8 voice = std::min<u8>(2, bitfield(reg_addr() - 6, 0, 2));
						square(voice).set_duty(bitfield(data, 0, 4));
						break;
					}
				case 9:	 // Noise AND mask
					noise().set_and_mask(data);
					break;
				case 10:  // Noise OR mask
					noise().set_or_mask(data);
					break;
				case 13:  // Envelope control/Mode, bank
					write_env_mode(data);
					break;
				case 15:  // TEST register
					m_test = data;
					break;
				default: break;
			}
		}
		else
		{
			switch (reg_addr())
			{
				case 0:	 // Square 0 Fine tune
				case 1:	 // Square 0 Coarse tune
				case 2:	 // Square 1 Fine tune
				case 3:	 // Square 1 Coarse tune
				case 4:	 // Square 2 Fine tune
				case 5:	 // Square 2 Coarse tune
					{
						const u8 voice = std::min<u8>(2, bitfield(reg_addr(), 1, 2));
						const bool msb = bitfield(reg_addr(), 0);
						square(voice).set_divider(msb ? (u16(data) << 4) : data,
												  msb ? 0xff00 : 0x00ff);
						break;
					}
				case 6:	 // Noise Frequency
					noise().set_divider(data, 0x00ff);
					break;
				case 7:	 // Control
					control_enable_w(data);
					break;
				case 8:	  // Square 0 Amplitude
				case 9:	  // Square 1 Amplitude
				case 10:  // Square 2 Amplitude
					{
						const u8 voice = std::min<u8>(2, bitfield(reg_addr(), 0, 2));
						square(voice).set_envelope(bitfield(data, 5));
						square(voice).set_volume(bitfield(data, 0, 5));
						break;
					}
				case 11:  // Envelope Fine tune
				case 12:  // Envelope Coarse tune
					{
						const bool lsb = bitfield(reg_addr(), 0);
						envelope(0).set_divider(lsb ? data : (u16(data) << 4),
												lsb ? 0x00ff : 0xff00);
						break;
					}
				case 13:  // Envelope control/Mode, bank
					write_env_mode(data);
					break;
				case 14:  // IO Port A
				case 15:  // IO Port B
					io_w(reg_addr() - 14, data);
					break;
				default: break;
			}
		}
	}
	else
	{
		switch (reg_addr())
		{
			case 13:  // Envelope control/Mode, bank
				write_env_mode(data);
				break;
			default: aypsg_core::data_w(data); break;
		}
	}
}

u8 ay8930_core::data_r()
{
	u8 ret = 0xff;
	if (is_expanded_mode())
	{
		if (m_bank)
		{
			switch (reg_addr())
			{
				case 0:	 // Envelope 1 Fine tune
				case 1:	 // Envelope 1 Coarse tune
				case 2:	 // Envelope 2 Fine tune
				case 3:	 // Envelope 2 Coarse tune
					{
						const u8 voice = std::min<u8>(2, bitfield(reg_addr() + 1, 1, 2));
						const bool msb = bitfield(reg_addr(), 0);
						ret			   = envelope(voice).divider() >> (msb ? 8 : 0);
						break;
					}
				case 4:	 // Envelope 1 control
				case 5:	 // Envelope 2 control
					ret = envelope((reg_addr() - 4) + 1).ctrl_r();
					break;
				case 6:	 // Square 0 Duty
				case 7:	 // Square 1 Duty
				case 8:	 // Square 2 Duty
					{
						const u8 voice = std::min<u8>(2, bitfield(reg_addr() - 6, 0, 2));
						ret			   = square(voice).duty();
						break;
					}
				case 9:	 // Noise AND mask
					ret = noise().and_mask();
					break;
				case 10:  // Noise OR mask
					ret = noise().or_mask();
					break;
				case 13:  // Envelope control/Mode, bank
					ret = read_env_mode();
					break;
				case 15:  // TEST register
					ret = m_test;
					break;
				default: break;
			}
		}
		else
		{
			switch (reg_addr())
			{
				case 0:	 // Square 0 Fine tune
				case 1:	 // Square 0 Coarse tune
				case 2:	 // Square 1 Fine tune
				case 3:	 // Square 1 Coarse tune
				case 4:	 // Square 2 Fine tune
				case 5:	 // Square 2 Coarse tune
					{
						const u8 voice = std::min<u8>(2, bitfield(reg_addr(), 1, 2));
						const bool msb = bitfield(reg_addr(), 0);
						ret			   = square(voice).divider() >> (msb ? 8 : 0);
						break;
					}
				case 6:	 // Noise Frequency
					ret = noise().divider();
					break;
				case 7:	 // Control
					ret = control_enable_r();
					break;
				case 8:	  // Square 0 Amplitude
				case 9:	  // Square 1 Amplitude
				case 10:  // Square 2 Amplitude
					{
						const u8 voice = std::min<u8>(2, bitfield(reg_addr(), 0, 2));
						ret = (square(voice).envelope() ? 0x20 : 0x00) | square(voice).volume();
						break;
					}
				case 11:  // Envelope Fine tune
				case 12:  // Envelope Coarse tune
					{
						const bool lsb = bitfield(reg_addr(), 0);
						ret			   = envelope(0).divider() >> (lsb ? 0 : 8);
						break;
					}
				case 13:  // Envelope control/Mode, bank
					ret = read_env_mode();
					break;
				case 14:  // IO Port A
				case 15:  // IO Port B
					ret = io_r(reg_addr() - 14);
					break;
				default: break;
			}
		}
	}
	else
	{
		switch (reg_addr())
		{
			case 13:  // Envelope control/Mode, bank
				ret = read_env_mode();
				break;
			default: ret = aypsg_core::data_r(); break;
		}
	}
	return ret;
}
