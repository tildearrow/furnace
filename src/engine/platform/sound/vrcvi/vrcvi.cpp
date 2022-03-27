/*
	License: BSD-3-Clause
	see https://github.com/cam900/vgsound_emu/LICENSE for more details

	Copyright holder(s): cam900
	Konami VRC VI sound emulation core

	It's one of NES mapper with built-in sound chip, and also one of 2 Konami VRCs with this feature. (rest one has OPLL derivatives.)

	It's also DACless like other sound chip and mapper-with-sound manufactured by konami,
	the Chips 6 bit digital sound output is needs converted to analog sound output when you it want to make some sounds, or send to sound mixer.

	Its are used for Akumajou Densetsu (Japan release of Castlevania III), Madara, Esper Dream 2.

	The chip is installed in 351951 PCB and 351949A PCB.

	351951 PCB is used exclusivly for Akumajou Densetsu, Small board has VRC VI, PRG and CHR ROM.
	- It's configuration also calls VRC6a, iNES mapper 024.

	351949A PCB is for Last 2 titles with VRC VI, Bigger board has VRC VI, PRG and CHR ROM, and Battery Backed 8K x 8 bit SRAM.
	- Additionally, It's PRG A0 and A1 bit to VRC VI input is swapped, compare to above.
	- It's configuration also calls VRC6b, iNES mapper 026.

	The chip itself has 053328, 053329, 053330 Revision, but Its difference between revision is unknown.

	Like other mappers for NES, It has internal timer - Its timer can be sync with scanline like other Konami mapper in this era.

	Register layout (Sound and Timer only; 351951 PCB case, 351949A swaps xxx1 and xxx2):

	Address Bits      Description
	        7654 3210

	9000-9002 Pulse 1

	9000    x--- ---- Pulse 1 Duty ignore
	        -xxx ---- Pulse 1 Duty cycle
	        ---- xxxx Pulse 1 Volume
	9001    xxxx xxxx Pulse 1 Pitch bit 0-7
	9002    x--- ---- Pulse 1 Enable
	        ---- xxxx Pulse 1 Pitch bit 8-11

	9003 Sound control

	9003    ---- -x-- 4 bit Frequency mode
	        ---- -0x- 8 bit Frequency mode
	        ---- ---x Halt

	a000-a002 Pulse 2

	a000    x--- ---- Pulse 2 Duty ignore
	        -xxx ---- Pulse 2 Duty cycle
	        ---- xxxx Pulse 2 Volume
	a001    xxxx xxxx Pulse 2 Pitch bit 0-7
	a002    x--- ---- Pulse 2 Enable
	        ---- xxxx Pulse 2 Pitch bit 8-11

	b000-b002 Sawtooth

	b000    --xx xxxx Sawtooth Accumulate Rate
	b001    xxxx xxxx Sawtooth Pitch bit 0-7
	b002    x--- ---- Sawtooth Enable
	        ---- xxxx Sawtooth Pitch bit 8-11

	f000-f002 IRQ Timer

	f000    xxxx xxxx IRQ Timer latch
	f001    ---- -0-- Sync with scanline
	        ---- --x- Enable timer
	        ---- ---x Enable timer after IRQ Acknowledge
	f002    ---- ---- IRQ Acknowledge

	Frequency calculations:

	if 4 bit Frequency Mode then
		Frequency: Input clock / (bit 8 to 11 of Pitch + 1)
	end else if 8 bit Frequency Mode then
		Frequency: Input clock / (bit 4 to 11 of Pitch + 1)
	end else then
		Frequency: Input clock / (Pitch + 1)
	end
*/

#include "vrcvi.hpp"

void vrcvi_core::tick()
{
	m_out = 0;
	if (!m_control.m_halt) // Halt flag
	{
		// tick per each clock
		for (auto & elem : m_pulse)
		{
			if (elem.tick())
				m_out += elem.m_control.m_volume; // add 4 bit pulse output
		}
		if (m_sawtooth.tick())
			m_out += bitfield(m_sawtooth.m_accum, 3, 5); // add 5 bit sawtooth output
	}
	if (m_timer.tick())
		m_timer.counter_tick();
}

void vrcvi_core::reset()
{
	for (auto & elem : m_pulse)
		elem.reset();

	m_sawtooth.reset();
	m_timer.reset();
	m_control.reset();
	m_out = 0;
}

bool vrcvi_core::alu_t::tick()
{
	if (m_divider.m_enable)
	{
		const u16 temp = m_counter;
		// post decrement
		if (bitfield(m_host.m_control.m_shift, 1))
		{
			m_counter = (m_counter & 0x0ff) | (bitfield(bitfield(m_counter, 8, 4) - 1, 0, 4) << 8);
			m_counter = (m_counter & 0xf00) | (bitfield(bitfield(m_counter, 0, 8) - 1, 0, 8) << 0);
		}
		else if (bitfield(m_host.m_control.m_shift, 0))
		{
			m_counter = (m_counter & 0x00f) | (bitfield(bitfield(m_counter, 4, 8) - 1, 0, 8) << 4);
			m_counter = (m_counter & 0xff0) | (bitfield(bitfield(m_counter, 0, 4) - 1, 0, 4) << 0);
		}
		else
			m_counter = bitfield(bitfield(m_counter, 0, 12) - 1, 0, 12);

		// carry handling
		bool carry = bitfield(m_host.m_control.m_shift, 1) ? (bitfield(temp, 8, 4) == 0) : 
		            (bitfield(m_host.m_control.m_shift, 0) ? (bitfield(temp, 4, 8) == 0) :
								(bitfield(temp, 0, 12) == 0));
		if (carry)
			m_counter = m_divider.m_divider;

		return carry;
	}
	return false;
}

bool vrcvi_core::pulse_t::tick()
{
	if (!m_divider.m_enable)
		return false;

	if (vrcvi_core::alu_t::tick())
		m_cycle = bitfield(m_cycle + 1, 0, 4);

	return m_control.m_mode ? true : ((m_cycle > m_control.m_duty) ? true : false);
}

bool vrcvi_core::sawtooth_t::tick()
{
	if (!m_divider.m_enable)
		return false;

	if (vrcvi_core::alu_t::tick())
	{
		if (bitfield(m_cycle++, 0)) // Even step only
			m_accum += m_rate;
		if (m_cycle >= 14) // Reset accumulator at every 14 cycles
		{
			m_accum = 0;
			m_cycle = 0;
		}
	}
	return (m_accum == 0) ? false : true;
}

void vrcvi_core::alu_t::reset()
{
	m_divider.reset();
	m_counter = 0;
	m_cycle = 0;
}

void vrcvi_core::pulse_t::reset()
{
	vrcvi_core::alu_t::reset();
	m_control.reset();
}

void vrcvi_core::sawtooth_t::reset()
{
	vrcvi_core::alu_t::reset();
	m_rate = 0;
	m_accum = 0;
}

bool vrcvi_core::timer_t::tick()
{
	if (m_timer_control.m_enable)
	{
		if (!m_timer_control.m_sync) // scanline sync mode
		{
			m_prescaler -= 3;
			if (m_prescaler <= 0)
			{
				m_prescaler += 341;
				return true;
			}
		}
	}
	return (m_timer_control.m_enable && m_timer_control.m_sync) ? true : false;
}

void vrcvi_core::timer_t::counter_tick()
{
	if (bitfield(++m_counter, 0, 8) == 0)
	{
		m_counter = m_counter_latch;
		irq_set();
	}
}

void vrcvi_core::timer_t::reset()
{
	m_timer_control.reset();
	m_prescaler = 341;
	m_counter = m_counter_latch = 0;
	irq_clear();
}

// Accessors

void vrcvi_core::alu_t::divider_t::write(bool msb, u8 data)
{
	if (msb)
	{
		m_divider = (m_divider & ~0xf00) | (bitfield<u32>(data, 0, 4) << 8);
		m_enable = bitfield(data, 7);
	}
	else
		m_divider = (m_divider & ~0x0ff) | data;
}


void vrcvi_core::pulse_w(u8 voice, u8 address, u8 data)
{
	pulse_t &v = m_pulse[voice];
	switch (address)
	{
		case 0x00: // Control - 0x9000 (Pulse 1), 0xa000 (Pulse 2)
			v.m_control.m_mode = bitfield(data, 7);
			v.m_control.m_duty = bitfield(data, 4, 3);
			v.m_control.m_volume = bitfield(data, 0, 4);
			break;
		case 0x01: // Pitch LSB - 0x9001/0x9002 (Pulse 1), 0xa001/0xa002 (Pulse 2)
			v.m_divider.write(false, data);
			break;
		case 0x02: // Pitch MSB, Enable/Disable - 0x9002/0x9001 (Pulse 1), 0xa002/0xa001 (Pulse 2)
			v.m_divider.write(true, data);
			break;
	}
}

void vrcvi_core::saw_w(u8 address, u8 data)
{
	switch (address)
	{
		case 0x00: // Sawtooth Accumulate - 0xb000
			m_sawtooth.m_rate = bitfield(data, 0, 6);
			break;
		case 0x01: // Pitch LSB - 0xb001/0xb002 (Sawtooth)
			m_sawtooth.m_divider.write(false, data);
			break;
		case 0x02: // Pitch MSB, Enable/Disable - 0xb002/0xb001 (Sawtooth)
			m_sawtooth.m_divider.write(true, data);
			break;
	}
}

void vrcvi_core::timer_w(u8 address, u8 data)
{
	switch (address)
	{
		case 0x00: // Timer latch - 0xf000
			m_timer.m_counter_latch = data;
			break;
		case 0x01: // Timer control - 0xf001/0xf002
			m_timer.m_timer_control.m_sync = bitfield(data, 2);
			m_timer.m_timer_control.m_enable = bitfield(data, 1);
			m_timer.m_timer_control.m_enable_ack = bitfield(data, 0);
			if (m_timer.m_timer_control.m_enable)
			{
				m_timer.m_counter = m_timer.m_counter_latch;
				m_timer.m_prescaler = 341;
			}
			m_timer.irq_clear();
			break;
		case 0x02: // IRQ Acknowledge - 0xf002/0xf001
			m_timer.irq_clear();
			m_timer.m_timer_control.m_enable = m_timer.m_timer_control.m_enable_ack;
			break;
	}
}

void vrcvi_core::control_w(u8 data)
{
	// Global control - 0x9003
	m_control.m_halt = bitfield(data, 0);
	m_control.m_shift = bitfield(data, 1, 2);
}
