/*

============================================================================

NDS sound emulator
by cam900

MODIFIED BY TILDEARROW!!!
MODIFIED BY TILDEARROW!!!
MODIFIED BY TILDEARROW!!!
MODIFIED BY TILDEARROW!!!
MODIFIED BY TILDEARROW!!!
MODIFIED BY TILDEARROW!!!

making it SUPER CLEAR to comply with the license
this is NOT the original version! for the original version, git checkout
any commit from January 2025.

This file is licensed under zlib license.

============================================================================

zlib License

(C) 2024-present cam900 and contributors

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

============================================================================
TODO:
- needs to further verifications from real hardware

Tech info: https://problemkaputt.de/gbatek.htm

*/

#include "nds.hpp"
#include <stdlib.h>

namespace nds_sound_emu
{
	void nds_sound_t::reset()
	{
		for (channel_t &elem : m_channel)
			elem.reset();
		for (capture_t &elem : m_capture)
			elem.reset();

		m_control = 0;
		m_bias = 0;
		m_loutput = 0;
		m_routput = 0;
	}

  s32 nds_sound_t::predict() {
    s32 ret=INT32_MAX;
    for (u8 i = 0; i < 16; i++) {
      const s32 next=m_channel[i].predict();
      if (next<ret) ret=next;
    }
    return ret;
  }

	void nds_sound_t::resetTS(u32 what) {
		m_lastts = what;
		for (u8 i = 0; i < 16; i++) {
			m_channel[i].resetTS(m_lastts);
		}
	}

	void nds_sound_t::set_bb(blip_buffer_t* bbLeft, blip_buffer_t* bbRight) {
		for (u8 i = 0; i < 16; i++) {
			m_channel[i].set_bb(bbLeft,bbRight);
		}
	}

	void nds_sound_t::set_oscbuf(DivDispatchOscBuffer** oscBuf) {
		for (u8 i = 0; i < 16; i++) {
			m_channel[i].set_oscbuf(oscBuf[i]);
		}
	}

	void nds_sound_t::tick(s32 cycle)
	{
		m_loutput = m_routput = (m_bias & 0x3ff);
		if (!enable())
			return;

		// mix outputs
		s32 lmix = 0, rmix = 0;
		for (u8 i = 0; i < 16; i++)
		{
			channel_t &channel = m_channel[i];
			channel.update(cycle);
			/*
			// bypass mixer
			if (((i == 1) && (mix_ch1())) || ((i == 3) && (mix_ch3())))
				continue;

			lmix += channel.loutput();
			rmix += channel.routput();
			*/
		}

		return; // don't care about the rest

		// send mixer output to capture
		m_capture[0].update(lmix, cycle);
		m_capture[1].update(rmix, cycle);

		// select left/right output source
		switch (lout_from())
		{
		case 0: // left mixer
			break;
		case 1: // channel 1
			lmix = m_channel[1].loutput();
			break;
		case 2: // channel 3
			lmix = m_channel[3].loutput();
			break;
		case 3: // channel 1 + 3
			lmix = m_channel[1].loutput() + m_channel[3].loutput();
			break;
		}

		switch (rout_from())
		{
		case 0: // right mixer
			break;
		case 1: // channel 1
			rmix = m_channel[1].routput();
			break;
		case 2: // channel 3
			rmix = m_channel[3].routput();
			break;
		case 3: // channel 1 + 3
			rmix = m_channel[1].routput() + m_channel[3].routput();
			break;
		}

		// adjust master volume
		lmix = (lmix * mvol()) >> 13;
		rmix = (rmix * mvol()) >> 13;

		// add bias and clip output
		m_loutput = clamp<s32>((lmix + (m_bias & 0x3ff)), 0, 0x3ff);
		m_routput = clamp<s32>((rmix + (m_bias & 0x3ff)), 0, 0x3ff);
	}

	u8 nds_sound_t::read8(u32 addr)
	{
		return bitfield(read32(addr >> 2), bitfield(addr, 0, 2) << 3, 8);
	}

	u16 nds_sound_t::read16(u32 addr)
	{
		return bitfield(read32(addr >> 1), bitfield(addr, 0) << 4, 16);
	}

	u32 nds_sound_t::read32(u32 addr)
	{
		addr <<= 2; // word address

		switch (addr & 0x100)
		{
			case 0x000:
				if ((addr & 0xc) == 0)
					return m_channel[bitfield(addr, 4, 4)].control();
				break;
			case 0x100:
				switch (addr & 0xff)
				{
					case 0x00:
						return m_control;
					case 0x04:
						return m_bias;
					case 0x08:
						return m_capture[0].control() | (m_capture[1].control() << 8);
					case 0x10:
					case 0x18:
						return m_capture[bitfield(addr, 3)].dstaddr();
					default:
						break;
				}
				break;
		}
		return 0;
	}

	void nds_sound_t::write8(u32 addr, u8 data)
	{
		const u8 bit = bitfield(addr, 0, 2);
		const u32 in = u32(data) << (bit << 3);
		const u32 in_mask = 0xff << (bit << 3);
		write32(addr >> 2, in, in_mask);
	}

	void nds_sound_t::write16(u32 addr, u16 data, u16 mask)
	{
		const u8 bit = bitfield(addr, 0);
		const u32 in = u32(data) << (bit << 4);
		const u32 in_mask = u32(mask) << (bit << 4);
		write32(addr >> 1, in, in_mask);
	}

	void nds_sound_t::write32(u32 addr, u32 data, u32 mask)
	{
		addr <<= 2; // word address
		
		switch (addr & 0x100)
		{
			case 0x000:
				m_channel[bitfield(addr, 4, 4)].write(bitfield(addr, 2, 2), data, mask);
				break;
			case 0x100:
				switch (addr & 0xff)
				{
					case 0x00:
						m_control = (m_control & ~mask) | (data & mask);
						break;
					case 0x04:
						mask &= 0x3ff;
						m_bias = (m_bias & ~mask) | (data & mask);
						break;
					case 0x08:
						if (bitfield(mask, 0, 8))
							m_capture[0].control_w(data & 0xff);
						if (bitfield(mask, 8, 8))
							m_capture[1].control_w((data >> 8) & 0xff);
						break;
					case 0x10:
					case 0x14:
					case 0x18:
					case 0x1c:
						m_capture[bitfield(addr, 3)].addrlen_w(bitfield(addr, 2), data, mask);
						break;
					default:
						break;
				}
				break;
		}
	}

	// channels
	void nds_sound_t::channel_t::reset()
	{
		m_control = 0;
		m_sourceaddr = 0;
		m_freq = 0;
		m_loopstart = 0;
		m_length = 0;

		m_ctl_volume = 0;
		m_ctl_voldiv = 0;
		m_ctl_hold = 0;
		m_ctl_pan = 0;
		m_ctl_duty = 0;
		m_ctl_repeat = 0;
		m_ctl_format = 0;
		m_ctl_busy = 0;

		m_playing = false;
		m_adpcm_out = 0;
		m_adpcm_index = 0;
		m_prev_adpcm_out = 0;
		m_prev_adpcm_index = 0;
		m_cur_addr = 0;
		m_cur_state = 0;
		m_cur_bitaddr = 0;
		m_delay = 0;
		m_sample = 0;
		m_lfsr = 0x7fff;
		m_lfsr_out = 0x7fff;
		m_counter = 0x10000;
		m_output = 0;
		m_loutput = 0;
		m_routput = 0;
	}

	void nds_sound_t::channel_t::write(u32 offset, u32 data, u32 mask)
	{
		const u32 old = m_control;
		switch (offset & 3)
		{
			case 0: // Control/Status
				m_control = (m_control & ~mask) | (data & mask);

				// explode this register
				m_ctl_volume = bitfield(m_control, 0, 7);
				m_ctl_voldiv = m_voldiv_shift[bitfield(m_control, 8, 2)];
				m_ctl_hold = bitfield(m_control, 15);
				m_ctl_pan = bitfield(m_control, 16, 7);
				m_ctl_duty = bitfield(m_control, 24, 3);
				m_ctl_repeat = bitfield(m_control, 27, 2);
				m_ctl_format = bitfield(m_control, 29, 2);
				m_ctl_busy = bitfield(m_control, 31);

				if (bitfield(old ^ m_control, 31))
				{
					if (m_ctl_busy)
						keyon();
					else if (!m_ctl_busy)
						keyoff();
				}
				// reset hold flag
				if (!m_playing && !m_ctl_hold)
				{
					m_sample = m_lfsr_out = 0;
					m_output = m_loutput = m_routput = 0;
				}
				break;
			case 1: // Source address
				mask &= 0x7ffffff;
				m_sourceaddr = (m_sourceaddr & ~mask) | (data & mask);
				break;
			case 2: // Frequency, Loopstart
				if (bitfield(mask, 0, 16))
					m_freq = (m_freq & bitfield(~mask, 0, 16)) | (bitfield(data & mask, 0, 16));
				if (bitfield(mask, 16, 16))
					m_loopstart = (m_loopstart & bitfield(~mask, 16, 16)) | (bitfield(data & mask, 16, 16));
				break;
			case 3: // Length
				mask &= 0x3fffff;
				m_length = (m_length & ~mask) | (data & mask);
				break;
		}
	}

	void nds_sound_t::channel_t::keyon()
	{
		if (!m_playing)
		{
			m_playing = true;
			m_delay = m_ctl_format == 2 ? 11 : 3; // 3 (11 for ADPCM) delay for playing sample
			m_cur_bitaddr = m_cur_addr = 0;
			m_cur_state = (m_ctl_format == 2) ? STATE_ADPCM_LOAD : ((m_loopstart == 0) ? STATE_POST_LOOP : STATE_PRE_LOOP);
			m_counter = 0x10000;
			m_sample = 0;
			m_lfsr_out = 0x7fff;
			m_lfsr = 0x7fff;
		}
	}


	void nds_sound_t::channel_t::keyoff()
	{
		if (m_playing)
		{
			if (m_ctl_busy) {
				m_control &= ~(1 << 31);
				m_ctl_busy = false;
			}
			if (!m_ctl_hold)
			{
				m_sample = m_lfsr_out = 0;
				m_output = m_loutput = m_routput = 0;
			}

			m_playing = false;
		}
	}

  // sorry. I need my spaces back.
  void nds_sound_t::channel_t::update(s32 timestamp)
  {
    if (m_playing)
    {
      for (s32 i=m_lastts; i<timestamp; i++) {
        int cycle = m_counter - m_freq;
        if (cycle>timestamp-i) cycle=timestamp-i;
        if (cycle<1) cycle=1;

	// get output
	m_counter -= cycle;
	while (m_counter <= m_freq)
	{
	  // advance
	  fetch();
	  advance();
	  m_counter += 0x10000 - m_freq;
	}
	m_output = (m_sample * m_ctl_volume) >> (7 + m_ctl_voldiv);
	const s32 loutput = (m_output * lvol()) >> 7;
	const s32 routput = (m_output * rvol()) >> 7;

        i+=cycle-1;

        if (m_loutput!=loutput) {
          blip_add_delta(m_bb[0],i,m_loutput-loutput);
          m_loutput=loutput;
        }
        if (m_routput!=routput) {
          blip_add_delta(m_bb[1],i,m_routput-routput);
          m_routput=routput;
        }
        m_oscBuf->putSample(i,(loutput+routput)>>1);
      }
    }
    m_lastts = timestamp;
  }

  s32 nds_sound_t::channel_t::predict() {
    if (!m_playing) return INT32_MAX;
    if (!(m_ctl_volume)) return INT32_MAX;
    return m_counter-m_freq;
  }

	void nds_sound_t::channel_t::fetch()
	{
		if (m_playing)
		{
			// fetch samples
			switch (m_ctl_format)
			{
			case 0: // PCM8
				m_sample = s16(m_host.m_intf.read_byte(addr()) << 8);
				break;
			case 1: // PCM16
				m_sample = m_host.m_intf.read_word(addr());
				break;
			case 2: // ADPCM
				m_sample = m_cur_state == STATE_ADPCM_LOAD ? 0 : m_adpcm_out;
				break;
			case 3: // PSG or Noise
				m_sample = 0;
				if (m_psg) // psg
					m_sample = (m_ctl_duty == 7) ? -0x7fff : ((m_cur_bitaddr < s32(u32(7) - m_ctl_duty)) ? -0x7fff : 0x7fff);
				else if (m_noise) // noise
					m_sample = m_lfsr_out;
				break;
			}
		}

		// apply delay
		if (m_ctl_format != 3 && m_delay > 0)
			m_sample = 0;
	}

	void nds_sound_t::channel_t::advance()
	{
		if (m_playing)
		{
			// advance bit address
			switch (m_ctl_format)
			{
			case 0: // PCM8
				m_cur_bitaddr += 8;
				break;
			case 1: // PCM16
				m_cur_bitaddr += 16;
				break;
			case 2: // ADPCM
				if (m_cur_state == STATE_ADPCM_LOAD) // load ADPCM data
				{
					if (m_cur_bitaddr == 0)
						m_prev_adpcm_out = m_adpcm_out = m_host.m_intf.read_word(addr());
					if (m_cur_bitaddr == 16)
						m_prev_adpcm_index = m_adpcm_index = clamp<s32>(m_host.m_intf.read_byte(addr()) & 0x7f, 0, 88);
				}
				else // decode ADPCM
				{
					const u8 input = bitfield(m_host.m_intf.read_byte(addr()), m_cur_bitaddr & 4, 4);
					s32 diff = ((bitfield(input, 0, 3) * 2 + 1) * m_host.adpcm_diff_table[m_adpcm_index] / 8);
					if (bitfield(input, 3)) diff = -diff;
					m_adpcm_out = clamp<s32>(m_adpcm_out + diff, -0x8000, 0x7fff);
					m_adpcm_index = clamp<s32>(m_adpcm_index + m_host.adpcm_index_table[bitfield(input, 0, 3)], 0, 88);
				}
				m_cur_bitaddr += 4;
				break;
			case 3: // PSG or Noise
				if (m_psg) // psg
					m_cur_bitaddr = (m_cur_bitaddr + 1) & 7;
				else if (m_noise) // noise
				{
					if (bitfield(m_lfsr, 0))
					{
						m_lfsr = (m_lfsr >> 1) ^ 0x6000;
						m_lfsr_out = -0x7fff;
					}
					else
					{
						m_lfsr >>= 1;
						m_lfsr_out = 0x7fff;
					}
				}
				break;
			}

			// address update
			if (m_ctl_format != 3)
			{
				// adjust delay
				m_delay--;

				// update address, loop
				while (m_cur_bitaddr >= 32)
				{
					// already loaded?
					if (m_ctl_format == 2 && m_cur_state == STATE_ADPCM_LOAD)
					{
						m_cur_state = m_loopstart == 0 ? STATE_POST_LOOP : STATE_PRE_LOOP;
					}
					m_cur_addr++;
					if (m_cur_state == STATE_PRE_LOOP && m_cur_addr >= m_loopstart)
					{
						m_cur_state = STATE_POST_LOOP;
						m_cur_addr = 0;
						if (m_ctl_format == 2)
						{
							m_prev_adpcm_out = m_adpcm_out;
							m_prev_adpcm_index = m_adpcm_index;
						}
					}
					else if (m_cur_state == STATE_POST_LOOP && m_cur_addr >= m_length)
					{
						switch (m_ctl_repeat)
						{
						case 0: // manual; not correct?
						case 2: // one-shot
						case 3: // prohibited
							keyoff();
							break;
						case 1: // loop infinitely
							if (m_ctl_format == 2)
							{
								if (m_loopstart == 0) // reload ADPCM
								{
									m_cur_state = STATE_ADPCM_LOAD;
								}
								else // restore
								{
									m_adpcm_out = m_prev_adpcm_out;
									m_adpcm_index = m_prev_adpcm_index;
								}
							}
							m_cur_addr = 0;
							break;
						}
					}
					m_cur_bitaddr -= 32;
				}
			}
		}
	}

	// capture
	void nds_sound_t::capture_t::reset()
	{
		m_control = 0;
		m_dstaddr = 0;
		m_length = 0;

		m_counter = 0x10000;
		m_cur_addr = 0;
		m_cur_waddr = 0;
		m_cur_bitaddr = 0;
		m_enable = false;
	}

	void nds_sound_t::capture_t::control_w(u8 data)
	{
		const u8 old = m_control;
		m_control = data;
		if (bitfield(old ^ m_control, 7))
		{
			if (busy())
				capture_on();
			else if (!busy())
				capture_off();
		}
	}

	void nds_sound_t::capture_t::addrlen_w(u32 offset, u32 data, u32 mask)
	{
		switch (offset & 1)
		{
			case 0: // Destination Address
				mask &= 0x7ffffff;
				m_dstaddr = (m_dstaddr & ~mask) | (data & mask);
				break;
			case 1: // Buffer Length
				mask &= 0xffff;
				m_length = (m_length & ~mask) | (data & mask);
				break;
		}
	}

	void nds_sound_t::capture_t::update(s32 mix, s32 cycle)
	{
		if (m_enable)
		{
			s32 inval = 0;
			// get inputs
			// TODO: hardware bugs aren't emulated, add mode behavior not verified
			if (addmode())
				inval = get_source() ? m_input.output() + m_output.output() : mix;
			else
				inval = get_source() ? m_input.output() : mix;

			// clip output
			inval = clamp<s32>(inval, -0x8000, 0x7fff);

			// update counter
			m_counter -= cycle;
			while (m_counter <= m_output.freq())
			{
				// write to memory; TODO: verify write behavior
				if (format()) // 8 bit output
				{
					m_fifo[m_fifo_head & 7].write_byte(m_cur_bitaddr & 0x18, (inval >> 8) & 0xff);
					m_cur_bitaddr += 8;
				}
				else
				{
					m_fifo[m_fifo_head & 7].write_word(m_cur_bitaddr & 0x10, inval & 0xffff);
					m_cur_bitaddr += 16;
				}

				// update address
				while (m_cur_bitaddr >= 32)
				{
					// clear FIFO empty flag
					m_fifo_empty = false;

					// advance FIFO head position
					m_fifo_head = (m_fifo_head + 1) & 7;
					if ((m_fifo_head & fifo_mask()) == (m_fifo_tail & fifo_mask()))
						m_fifo_full = true;

					// update loop
					if (++m_cur_addr >= m_length)
					{
						if (repeat())
							m_cur_addr = 0;
						else
							capture_off();
					}

					if (m_fifo_full)
					{
						// execute FIFO
						fifo_write();

						// check repeat
						if (m_cur_waddr >= m_length && repeat())
							m_cur_waddr = 0;
					}

					m_cur_bitaddr -= 32;
				}
				m_counter += 0x10000 - m_output.freq();
			}
		}
	}

	bool nds_sound_t::capture_t::fifo_write()
	{
		if (m_fifo_empty)
			return true;

		// clear FIFO full flag
		m_fifo_full = false;

		// write FIFO data to memory
		m_host.m_intf.write_dword(waddr(), m_fifo[m_fifo_tail].data());
		m_cur_waddr++;

		// advance FIFO tail position
		m_fifo_tail = (m_fifo_tail + 1) & 7;
		if ((m_fifo_head & fifo_mask()) == (m_fifo_tail & fifo_mask()))
			m_fifo_empty = true;

		return m_fifo_empty;
	}

	void nds_sound_t::capture_t::capture_on()
	{
		if (!m_enable)
		{
			m_enable = true;

			// reset address
			m_cur_bitaddr = 0;
			m_cur_addr = m_cur_waddr = 0;
			m_counter = 0x10000;

			// reset FIFO
			m_fifo_head = m_fifo_tail = 0;
			m_fifo_empty = true;
			m_fifo_full = false;
		}
	}

	void nds_sound_t::capture_t::capture_off()
	{
		if (m_enable)
		{
			// flush FIFO
			while (m_cur_waddr < m_length)
			{
				if (fifo_write())
					break;
			}

			m_enable = false;
			if (busy())
				m_control &= ~(1 << 7);
		}
	}

}; // namespace nds_sound_emu
