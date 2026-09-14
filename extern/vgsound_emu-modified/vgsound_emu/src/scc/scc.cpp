/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Contributor(s): Natt Akuma, James Alan Nguyen, Laurens Holst
	Konami SCC emulation core

        modified by tildearrow...
*/

#include "scc.hpp"
#include "../../../../../src/engine/dispatch.h"

// shared SCC features
void scc_core::tick(const int cycles, blip_buffer_t* bb, DivDispatchOscBuffer** oscBuf)
{
	for (int elem=0; elem<5; elem++)
	{
		m_voice[elem].tick(cycles,bb,oscBuf[elem]);
	}
}

void scc_core::voice_t::updateOut(const int pos) {
  int out=0;
  // get output
  if (m_enable)
  {
    out = (m_wave[m_addr] * m_volume) >> 4;  // scale to 11 bit digital output
  }
  else
  {
    out = 0;
  }

  m_oscBuf->putSample(pos,out<<7);

  if (out!=m_out) {
    blip_add_delta(m_bb,pos,(out-m_out)<<5);
    m_out=out;
  }
}

void scc_core::voice_t::tick(const int amt, blip_buffer_t* bb, DivDispatchOscBuffer* oscBuf)
{
  m_bb=bb;
  m_oscBuf=oscBuf;

  if (m_pitch >= 9)  // or voice is halted
  {
    int rem=amt;
    for (int pos=0; pos<amt; pos++) {
      int cycles=m_host.m_test.freq_4bit()?(m_counter&15):(m_host.m_test.freq_8bit()?(m_counter&0xff):(m_counter&0xfff));
      if (cycles>rem) cycles=rem;
      if (cycles<1) cycles=1;

      // update counter - Post decrement
      const u16 temp = m_counter;
      if (m_host.m_test.freq_4bit())  // 4 bit frequency mode
      {
        m_counter = (m_counter & ~0x0ff) | (bitfield(bitfield(m_counter, 0, 8) - cycles, 0, 8) << 0);
        m_counter = (m_counter & ~0xf00) | (bitfield(bitfield(m_counter, 8, 4) - cycles, 0, 4) << 8);
      }
      else
      {
        m_counter = bitfield(m_counter - cycles, 0, 12);
      }

      // handle counter carry
      const bool carry = (temp<cycles) || (m_host.m_test.freq_8bit()
                         ? (bitfield(temp, 0, 8) == 0)
                         : (m_host.m_test.freq_4bit() ? (bitfield(temp, 8, 4) == 0)
                         : (bitfield(temp, 0, 12) == 0)));
      if (carry)
      {
        m_addr    = bitfield(m_addr + 1, 0, 5);
        m_counter = m_pitch - ((temp<cycles)?(cycles-temp-1):0);
        while (m_counter>m_pitch) {
          m_addr    = bitfield(m_addr + 1, 0, 5);
          m_counter+=m_pitch-1;
        }
      }
      pos+=cycles-1;
      rem-=cycles;
      updateOut(pos);
    }
  } else {
    updateOut(0);
  }
}

void scc_core::reset()
{
	for (int elem=0; elem<5; elem++)
	{
		m_voice[elem].reset();
	}

	m_test.reset();
	m_out = 0;
  memset(m_reg,0,256);
}

void scc_core::voice_t::reset()
{
	memset(m_wave,0,32);
	m_enable  = false;
	m_pitch	  = 0;
	m_volume  = 0;
	m_addr	  = 0;
	m_counter = 0;
	m_out	  = 0;
}

// SCC accessors
u8 scc_core::wave_r(bool is_sccplus, u8 address)
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

void scc_core::wave_w(bool is_sccplus, u8 address, u8 data)
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

void scc_core::freq_vol_enable_w(u8 address, u8 data)
{
	// *0-*f Pitch, Volume, Enable
	address				= bitfield(address, 0, 4);	// mask address to 4 bit
	const u8 voice_freq = bitfield(address, 1, 3);
	switch (address)
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
			m_voice[voice_freq].set_pitch(u16(bitfield(data, 0, 4)) << 8, 0xf00);
			break;
		case 0xa:  // 0x*a Voice 0 Volume
		case 0xb:  // 0x*b Voice 1 Volume
		case 0xc:  // 0x*c Voice 2 Volume
		case 0xd:  // 0x*d Voice 3 Volume
		case 0xe:  // 0x*e Voice 4 Volume
			m_voice[address - 0xa].set_volume(bitfield(data, 0, 4));
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

void k051649_scc_core::scc_w(bool is_sccplus, u8 address, u8 data)
{
	const u8 voice = bitfield(address, 5, 3);
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
		case 0b111:	 // 0xe0-0xff Test register
			m_test.set_freq_4bit(bitfield(data, 0));
			m_test.set_freq_8bit(bitfield(data, 1));
			m_test.set_resetpos(bitfield(data, 5));
			m_test.set_rotate(bitfield(data, 6));
			m_test.set_rotate4(bitfield(data, 7));
			break;
	}
	m_reg[address] = data;
}

void k052539_scc_core::scc_w(bool is_sccplus, u8 address, u8 data)
{
	const u8 voice = bitfield(address, 5, 3);
	if (is_sccplus)
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
			case 0b110:	 // 0xc0-0xdf Test register
				m_test.set_freq_4bit(bitfield(data, 0));
				m_test.set_freq_8bit(bitfield(data, 1));
				m_test.set_resetpos(bitfield(data, 5));
				m_test.set_rotate(bitfield(data, 6));
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
			case 0b110:	 // 0xc0-0xdf Test register
				m_test.set_freq_4bit(bitfield(data, 0));
				m_test.set_freq_8bit(bitfield(data, 1));
				m_test.set_resetpos(bitfield(data, 5));
				m_test.set_rotate(bitfield(data, 6));
				break;
			default: break;
		}
	}
	m_reg[address] = data;
}

u8 k051649_scc_core::scc_r(bool is_sccplus, u8 address)
{
	const u8 voice = bitfield(address, 5, 3);
	const u8 wave  = bitfield(address, 0, 5);
	u8 ret		   = 0xff;
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
	return ret;
}

u8 k052539_scc_core::scc_r(bool is_sccplus, u8 address)
{
	const u8 voice = bitfield(address, 5, 3);
	const u8 wave  = bitfield(address, 0, 5);
	u8 ret		   = 0xff;
	if (is_sccplus)
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

// Mapper
void k051649_core::reset()
{
	k051649_scc_core::reset();
	m_mapper.reset();
	m_scc_enable = false;
}

void k052539_core::reset()
{
	k052539_scc_core::reset();
	m_mapper.reset();
	m_scc_enable = false;
	m_is_sccplus = false;
}

void k051649_core::k051649_mapper_t::reset()
{
	m_bank[0] = 0;
	m_bank[1] = 1;
	m_bank[2] = 2;
	m_bank[3] = 3;
}

void k052539_core::k052539_mapper_t::reset()
{
	m_bank[0] = 0;
	m_bank[1] = 1;
	m_bank[2] = 2;
	m_bank[3] = 3;
	std::fill(m_ram_enable.begin(), m_ram_enable.end(), false);
}

// Mapper accessors
u8 k051649_core::read(u16 address)
{
	if ((bitfield(address, 11, 5) == 0b10011) && m_scc_enable)
	{
		return scc_r(false, u8(address));
	}

	return m_intf.read_byte((u32(m_mapper.bank(bitfield(address, 13, 2) ^ 2)) << 13) |
							bitfield(address, 0, 13));
}

u8 k052539_core::read(u16 address)
{
	if ((bitfield(address, 11, 5) == 0b10011) && m_scc_enable && (!m_is_sccplus))
	{
		return scc_r(false, u8(address));
	}

	if ((bitfield(address, 11, 5) == 0b10111) && m_scc_enable && m_is_sccplus)
	{
		return scc_r(true, u8(address));
	}

	return m_intf.read_byte((u32(m_mapper.bank(bitfield(address, 13, 2) ^ 2)) << 13) |
							bitfield(address, 0, 13));
}

void k051649_core::write(u16 address, u8 data)
{
	const u16 bank = bitfield(address, 13, 2) ^ 2;
	switch (bitfield(address, 11, 5))
	{
		case 0b01010:  // 0x5000-0x57ff Bank 0
		case 0b01110:  // 0x7000-0x77ff Bank 1
		case 0b10010:  // 0x9000-0x97ff Bank 2
		case 0b10110:  // 0xb000-0xb7ff Bank 3
			m_mapper.set_bank(bank, data);
			m_scc_enable = (bitfield(m_mapper.bank(2), 0, 6) == 0x3f);
			break;
		case 0b10011:  // 0x9800-9fff SCC
			if (m_scc_enable)
			{
				scc_w(false, u8(address), data);
			}
			break;
	}
}

void k052539_core::write(u16 address, u8 data)
{
	u8 prev				  = 0;
	bool update			  = false;
	const u16 bank		  = bitfield(address, 13, 2) ^ 2;
	const bool ram_enable = m_mapper.ram_enable(bank);
	if (ram_enable)
	{
		m_intf.write_byte((u32(m_mapper.bank(bank)) << 13) | bitfield(address, 0, 13), data);
	}
	switch (bitfield(address, 11, 5))
	{
		case 0b01010:  // 0x5000-0x57ff Bank 0
		case 0b01110:  // 0x7000-0x77ff Bank 1
		case 0b10010:  // 0x9000-0x97ff Bank 2
		case 0b10110:  // 0xb000-0xb7ff Bank 3
			if (!ram_enable)
			{
				prev = m_mapper.bank(bank);
				m_mapper.set_bank(bank, data);
				update = prev ^ m_mapper.bank(bank);
			}
			break;
		case 0b10011:  // 0x9800-0x9fff SCC
			if ((!ram_enable) && m_scc_enable && (!m_is_sccplus))
			{
				scc_w(false, u8(address), data);
			}
			break;
		case 0b10111:  // 0xb800-0xbfff SCC+, Mapper configuration
			if (bitfield(address, 1, 10) == 0x3ff)
			{
				m_mapper.set_ram_enable(0, bitfield(data, 4) || bitfield(data, 0));
				m_mapper.set_ram_enable(1, bitfield(data, 4) || bitfield(data, 1));
				m_mapper.set_ram_enable(2, bitfield(data, 4) || bitfield(data, 2));
				m_mapper.set_ram_enable(3, bitfield(data, 4));
				prev		 = (m_is_sccplus ? 1 : 0);
				m_is_sccplus = bitfield(data, 5);
				update		 = prev ^ (m_is_sccplus ? 1 : 0);
			}
			else if ((!ram_enable) && m_scc_enable && m_is_sccplus)
			{
				scc_w(true, u8(address), data);
			}
			break;
	}
	if (update)
	{
		m_scc_enable =
		  m_is_sccplus ? bitfield(m_mapper.bank(3), 7) : (bitfield(m_mapper.bank(2), 0, 6) == 0x3f);
	}
}
