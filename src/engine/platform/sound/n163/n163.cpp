/*
	License: BSD-3-Clause
	see https://github.com/cam900/vgsound_emu/LICENSE for more details

	Copyright holder(s): cam900
	Namco 163 Sound emulation core

	This chip is one of NES mapper with sound expansion, This one is by Namco.

	It has 1 to 8 wavetable channels, All channel registers and waveforms are stored to internal RAM.
	4 bit Waveforms are freely allocatable, and its length is variables; its can be stores many short waveforms or few long waveforms in RAM.

	But waveforms are needs to squash, reallocate to avoid conflict with channel register area, each channel register size is 8 bytes per channels.

	Sound output is time division multiplexed, it's can be captured only single channels output at once. in reason, More activated channels are less sound quality.

	Sound register layout

	Address Bit      Description
	        7654 3210

	78-7f Channel 0
	78      xxxx xxxx Channel 0 Pitch input bit 0-7
	79      xxxx xxxx Channel 0 Accumulator bit 0-7*
	7a      xxxx xxxx Channel 0 Pitch input bit 8-15
	7b      xxxx xxxx Channel 0 Accumulator bit 8-15*
	7c      xxxx xx-- Channel 0 Waveform length, 256 - (x * 4)
	        ---- --xx Channel 0 Pitch input bit 16-17
	7d      xxxx xxxx Channel 0 Accumulator bit 16-23*
	7e      xxxx xxxx Channel 0 Waveform base offset
	        xxxx xxx- RAM byte (0 to 127)
	        ---- ---x RAM nibble
					---- ---0 Low nibble
					---- ---1 High nibble
	7f      ---- xxxx Channel 0 Volume

	7f Number of active channels
	7f      -xxx ---- Number of active channels
	        -000 ---- Channel 0 activated
	        -001 ---- Channel 1 activated
	        -010 ---- Channel 2 activated
	        ...
	        -110 ---- Channel 6 activated
	        -111 ---- Channel 7 activated

	70-77 Channel 1 (Optional if activated)
	68-6f Channel 2 (Optional if activated)
	...
	48-4f Channel 6 (Optional if activated)
	40-47 Channel 7 (Optional if activated)

	Rest of RAM area are for 4 bit Waveform and/or scratchpad.
	Each waveform byte has 2 nibbles packed, fetches LSB first, MSB next.
	        ---- xxxx 4 bit waveform, LSB
	        xxxx ---- Same as above, MSB

	Waveform address: Waveform base offset + Bit 16 to 23 of Accumulator, 1 LSB of result is nibble select, 7 MSB of result is Byte address in RAM.

	Frequency formula:
	Frequency: Pitch input * ((Input clock * 15 * Number of activated voices) / 65536)
*/

#include "n163.hpp"

void n163_core::tick()
{
	m_out = 0;
	// 0xe000-0xe7ff Disable sound bits (bit 6, bit 0 to 5 are CPU ROM Bank 0x8000-0x9fff select.)
	if (m_disable)
		return;

	// tick per each clock
	const u32 freq = m_ram[m_voice_cycle + 0] | (u32(m_ram[m_voice_cycle + 2]) << 8) | (bitfield<u32>(m_ram[m_voice_cycle + 4], 0, 2) << 16); // 18 bit frequency
	u32 accum      = m_ram[m_voice_cycle + 1] | (u32(m_ram[m_voice_cycle + 3]) << 8) | (          u32(m_ram[m_voice_cycle + 5])       << 16); // 24 bit accumulator
	const u16 length = 256 - (m_ram[m_voice_cycle + 4] & 0xfc);
	const u8 addr    = m_ram[m_voice_cycle + 6] + bitfield(accum, 16, 8);
	const s16 wave   = (bitfield(m_ram[bitfield(addr, 1, 7)], bitfield(addr, 0) << 2, 4) - 8);
	const s16 volume = bitfield(m_ram[m_voice_cycle + 7], 0, 4);

	// accumulate address
	accum = bitfield(accum + freq, 0, 24);
	if (bitfield(accum, 16, 8) >= length)
		accum = bitfield(accum, 0, 18);

	// writeback to register
	m_ram[m_voice_cycle + 1] = bitfield(accum,  0, 8);
	m_ram[m_voice_cycle + 3] = bitfield(accum,  8, 8);
	m_ram[m_voice_cycle + 5] = bitfield(accum, 16, 8);

	// update voice cycle
	m_voice_cycle -= 0x8;
	if (m_voice_cycle < (0x78 - (bitfield(m_ram[0x7f], 4, 3) << 3)))
		m_voice_cycle = 0x78;

	// output 4 bit waveform and volume, multiplexed
	m_out = wave * volume;
}

void n163_core::reset()
{
	// reset this chip
	m_disable = false;
	std::fill(std::begin(m_ram), std::end(m_ram), 0);
	m_voice_cycle = 0x78;
	m_addr_latch.reset();
	m_out = 0;
}

// accessor
void n163_core::addr_w(u8 data)
{
	// 0xf800-0xffff Sound address, increment
	m_addr_latch.addr = bitfield(data, 0, 7);
	m_addr_latch.incr = bitfield(data, 7);
}

void n163_core::data_w(u8 data, bool cpu_access)
{
	// 0x4800-0x4fff Sound data write
	m_ram[m_addr_latch.addr] = data;

	// address latch increment
	if (cpu_access && m_addr_latch.incr)
		m_addr_latch.addr = bitfield(m_addr_latch.addr + 1, 0, 7);
}

u8 n163_core::data_r(bool cpu_access)
{
	// 0x4800-0x4fff Sound data read
	const u8 ret = m_ram[m_addr_latch.addr];

	// address latch increment
	if (cpu_access && m_addr_latch.incr)
		m_addr_latch.addr = bitfield(m_addr_latch.addr + 1, 0, 7);

	return ret;
}
