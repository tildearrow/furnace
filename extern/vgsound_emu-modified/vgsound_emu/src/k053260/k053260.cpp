/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Konami K053260 core
*/

#include "k053260.hpp"

void k053260_core::tick(u32 cycle)
{
	m_out[0] = m_out[1] = 0;
	if (m_ctrl.sound_en())
	{
		for (int i = 0; i < 4; i++)
		{
			m_voice[i].tick(cycle);
			m_out[0] += m_voice[i].out(0);
			m_out[1] += m_voice[i].out(1);
		}
	}
	/*
	// dac clock (YM3012 format)
	u8 dac_clock = m_dac.clock();
	if (bitfield(++dac_clock, 0, 4) == 0)
	{
		m_intf.write_int(m_dac.state());
		u8 dac_state = m_dac.state();
		if (bitfield(++dac_state, 0) == 0)
		{
			m_ym3012.tick(bitfield(dac_state, 1), m_out[bitfield(dac_state, 1) ^ 1]);
		}

		m_dac.set_state(bitfield(dac_state, 0, 2));
	}
	m_dac.set_clock(bitfield(dac_clock, 0, 4));
	*/
}

void k053260_core::voice_t::tick(u32 cycle)
{
	if (m_enable && m_busy)
	{
		// update counter
		m_counter += cycle;
		if (m_counter >= 0x1000)
		{
			if (m_bitpos < 8)
			{
				m_bitpos += 8;
				m_addr = m_reverse ? bitfield(m_addr - 1, 0, 21) : bitfield(m_addr + 1, 0, 21);
				m_remain--;
				if (m_remain < 0)  // check end flag
				{
					if (m_loop)
					{
						m_addr		= m_start;
						m_remain	= m_length;
						m_output = 0;
					}
					else
					{
						m_busy = false;
					}
				}
			}
			m_data = m_host.m_intf.read_sample(bitfield(m_addr, 0, 21));  // fetch ROM
			if (m_adpcm)
			{
				m_bitpos		-= 4;
				const u8 nibble = bitfield(m_data, m_reverse ? (~m_bitpos & 4) : (m_bitpos & 4), 4);  // get nibble from ROM
				if (nibble)
				{
					m_output += m_host.adpcm_lut(nibble);
				}
			}
			else
			{
				m_bitpos -= 8;
			}
			m_counter = (m_counter - 0x1000) + bitfield(m_pitch, 0, 12);
		}

		// calculate output
		s32 output = (m_adpcm ? m_output : sign_ext<s32>(m_data, 8)) * s32(m_volume);
		// use math for now; actually fomula unknown
		m_out[0] = (output * m_host.pan_lut(m_pan, 0)) >> 7;
		m_out[1] = (output * m_host.pan_lut(m_pan, 1)) >> 7;
	}
	else
	{
		m_out[0] = m_out[1] = 0;
	}
}

u8 k053260_core::read(u8 address)
{
	address &= 0x3f;  // 6 bit for CPU read

	switch (address)
	{
		case 0x0:
		case 0x1:  // Answer from host
			return m_host2snd[address & 1];
			break;
		case 0x29:	// Voice playing status
			return (m_voice[0].busy() ? 0x1 : 0x0) | (m_voice[1].busy() ? 0x2 : 0x0) |
				   (m_voice[2].busy() ? 0x4 : 0x0) | (m_voice[3].busy() ? 0x8 : 0x0);
		case 0x2e:	// ROM readback
			{
				if (!m_ctrl.rom_read())
				{
					return 0xff;
				}

				const u32 rom_addr = m_voice[0].start() + m_voice[0].length();
				m_voice[0].length_inc();
				return m_intf.read_sample(rom_addr);
			}
	}
	return 0xff;
}

void k053260_core::write(u8 address, u8 data)
{
	address &= 0x3f;  // 6 bit for CPU write

	switch (address)
	{
		case 0x2:
		case 0x3:  // Reply to host
			m_snd2host[address & 1] = data;
			break;
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:	// voice 0
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:	// voice 1
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:	// voice 2
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:	// voice 3
			m_voice[bitfield(address - 0x8, 3, 2)].write(bitfield(address, 0, 3), data);
			break;
		case 0x28:	// keyon/off toggle
			for (int i = 0; i < 4; i++)
			{
				m_voice[i].set_reverse(bitfield(data, 4 + i));
				if (bitfield(data, i) && (!m_voice[i].enable()))
				{  // rising edge (keyon)
					m_voice[i].keyon();
				}
				else if ((!bitfield(data, i)) && m_voice[i].enable())
				{  // falling edge (keyoff)
					m_voice[i].keyoff();
				}
			}
			break;
		case 0x2a:	// loop/adpcm flag
			for (int i = 0; i < 4; i++)
			{
				m_voice[i].set_loop(bitfield(data, i));
				m_voice[i].set_adpcm(bitfield(data, i + 4));
			}
			break;
		case 0x2c:
			m_voice[0].set_pan(bitfield(data, 0, 3));
			m_voice[1].set_pan(bitfield(data, 3, 3));
			break;
		case 0x2d:
			m_voice[2].set_pan(bitfield(data, 0, 3));
			m_voice[3].set_pan(bitfield(data, 3, 3));
			break;
		case 0x2f: m_ctrl.write(data); break;
		default: break;
	}

	m_reg[address] = data;
}

// write registers on each voices
void k053260_core::voice_t::write(u8 address, u8 data)
{
	switch (address)
	{
		case 0:	 // pitch LSB
			m_pitch = (m_pitch & ~0x00ff) | data;
			break;
		case 1:	 // pitch MSB
			m_pitch = (m_pitch & ~0x0f00) | (u16(bitfield(data, 0, 4)) << 8);
			break;
		case 2:	 // source length LSB
			m_length = (m_length & ~0x000ff) | data;
			break;
		case 3:	 // source length MSB
			m_length = (m_length & ~0x0ff00) | (u16(data) << 8);
			break;
		case 4:	 // start address bit 0-7
			m_start = (m_start & ~0x0000ff) | data;
			break;
		case 5:	 // start address bit 8-15
			m_start = (m_start & ~0x00ff00) | (u32(data) << 8);
			break;
		case 6:	 // start address bit 16-20
			m_start = (m_start & ~0x1f0000) | (u32(bitfield(data, 16, 5)) << 16);
			break;
		case 7:	 // volume
			m_volume = bitfield(data, 0, 7);
			break;
	}
}

// key on trigger
void k053260_core::voice_t::keyon()
{
	m_enable = m_busy = 1;
	m_counter		  = bitfield(m_pitch, 0, 12);
	m_addr			  = m_start;
	m_remain		  = m_length;
	m_bitpos		  = 4;
	m_data			  = 0;
	m_output		  = 0;
	std::fill_n(m_out, 2, 0);
}

// key off trigger
void k053260_core::voice_t::keyoff() { m_enable = m_busy = 0; }

// reset chip
void k053260_core::reset()
{
	for (auto &elem : m_voice)
	{
		elem.reset();
	}

	//m_intf.write_int(0);

	std::fill_n(m_host2snd, 2, 0);
	std::fill_n(m_snd2host, 2, 0);
	m_ctrl.reset();
	//m_dac.reset();

	std::fill_n(m_reg, 64, 0);
	std::fill_n(m_out, 2, 0);
}

// reset voice
void k053260_core::voice_t::reset()
{
	m_enable  = 0;
	m_busy	  = 0;
	m_loop	  = 0;
	m_adpcm	  = 0;
	m_pitch	  = 0;
	m_reverse = 0;
	m_start	  = 0;
	m_length  = 0;
	m_volume  = 0;
	m_pan	  = 4;
	m_counter = 0;
	m_addr	  = 0;
	m_remain  = 0;
	m_bitpos  = 4;
	m_data	  = 0;
	m_output  = 0;
	m_out[0] = m_out[1] = 0;
}
