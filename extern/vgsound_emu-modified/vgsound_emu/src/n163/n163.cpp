/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Namco 163 Sound emulation core
*/

#include "n163.hpp"

void n163_core::tick()
{
	if (m_multiplex)
	{
		m_out = 0;
	}
	// 0xe000-0xe7ff Disable sound bits (bit 6, bit 0 to 5 are CPU ROM Bank
	// 0x8000-0x9fff select.)
	if (m_disable)
	{
		if (!m_multiplex)
		{
			m_out = 0;
		}
		return;
	}

	// tick per each clock
	const u32 freq = m_ram[m_voice_cycle + 0] | (u32(m_ram[m_voice_cycle + 2]) << 8) |
					 (bitfield<u32>(m_ram[m_voice_cycle + 4], 0, 2) << 16);	 // 18 bit frequency

	u32 accum = m_ram[m_voice_cycle + 1] | (u32(m_ram[m_voice_cycle + 3]) << 8) |
				(u32(m_ram[m_voice_cycle + 5]) << 16);	// 24 bit accumulator

	const u16 length = 256 - (m_ram[m_voice_cycle + 4] & 0xfc);
	const u8 addr	 = m_ram[m_voice_cycle + 6] + bitfield(accum, 16, 8);
	const s16 wave	 = (bitfield(m_ram[bitfield(addr, 1, 7)], bitfield(addr, 0) << 2, 4) - 8);
	const s16 volume = bitfield(m_ram[m_voice_cycle + 7], 0, 4);

	// get per-voice output
	const s16 voice_out					  = (wave * volume);
	m_voice_out[(m_voice_cycle >> 3) & 7] = voice_out;

	// accumulate address
	accum = bitfield(accum + freq, 0, 24);
	if (bitfield(accum, 16, 8) >= length)
	{
		accum = bitfield(accum, 0, 18);
	}

	// writeback to register
	m_ram[m_voice_cycle + 1] = bitfield(accum, 0, 8);
	m_ram[m_voice_cycle + 3] = bitfield(accum, 8, 8);
	m_ram[m_voice_cycle + 5] = bitfield(accum, 16, 8);

	// update voice cycle
	bool flush	  = m_multiplex ? true : false;
	m_voice_cycle -= 0x8;
	if (m_voice_cycle < (0x78 - (bitfield(m_ram[0x7f], 4, 3) << 3)))
	{
		if (!m_multiplex)
		{
			flush = true;
		}
		m_voice_cycle = 0x78;
	}

	// output 4 bit waveform and volume, multiplexed
	m_acc += voice_out;
	if (flush)
	{
		m_out = m_acc / (m_multiplex ? 1 : (bitfield(m_ram[0x7f], 4, 3) + 1));
		m_acc = 0;
	}
}

void n163_core::reset()
{
	// reset this chip
	m_disable	= false;
	m_multiplex = true;
	std::fill(m_ram.begin(), m_ram.end(), 0);
	m_voice_cycle = 0x78;
	m_addr_latch.reset();
	m_out = 0;
	m_acc = 0;
}

// accessor
void n163_core::addr_w(u8 data)
{
	// 0xf800-0xffff Sound address, increment
	m_addr_latch.write(data);
}

void n163_core::data_w(u8 data, bool cpu_access)
{
	// 0x4800-0x4fff Sound data write
	m_ram[m_addr_latch.addr()] = data;

	// address latch increment
	if (cpu_access && m_addr_latch.incr())
	{
		m_addr_latch.addr_inc();
	}
}

u8 n163_core::data_r(bool cpu_access)
{
	// 0x4800-0x4fff Sound data read
	const u8 ret = m_ram[m_addr_latch.addr()];

	// address latch increment
	if (cpu_access && m_addr_latch.incr())
	{
		m_addr_latch.addr_inc();
	}

	return ret;
}
