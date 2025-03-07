/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Konami VRC VI sound emulation core
*/

#include "vrcvi.hpp"

void vrcvi_core::tick()
{
	m_out = 0;
	if (!m_control.m_halt)	// Halt flag
	{
		// tick per each clock
		m_out += m_pulse[0].get_output();	 // add 4 bit pulse output
    m_out += m_pulse[1].get_output();
		m_out += m_sawtooth.get_output();  // add 5 bit sawtooth output
	}
}

void vrcvi_core::reset()
{
	m_pulse[0].reset();
  m_pulse[1].reset();
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
		if (m_host.m_control.m_shift&2)
		{
			m_counter = (m_counter & 0x0ff) | (bitfield(bitfield(m_counter, 8, 4) - 1, 0, 4) << 8);
			m_counter = (m_counter & 0xf00) | (bitfield(bitfield(m_counter, 0, 8) - 1, 0, 8) << 0);

      if (bitfield(temp, 8, 4) == 0)
      {
        m_counter = m_divider.m_divider;
        return true;
      }
		}
		else if (m_host.m_control.m_shift&1)
		{
			m_counter = (m_counter & 0x00f) | (bitfield(bitfield(m_counter, 4, 8) - 1, 0, 8) << 4);
			m_counter = (m_counter & 0xff0) | (bitfield(bitfield(m_counter, 0, 4) - 1, 0, 4) << 0);

      if (bitfield(temp, 4, 8) == 0)
      {
        m_counter = m_divider.m_divider;
        return true;
      }
		}
		else
		{
			m_counter = (m_counter-1)&0xfff; //bitfield(bitfield(m_counter, 0, 12) - 1, 0, 12);
      if (!(temp&0xfff)) {
        m_counter = m_divider.m_divider;
        return true;
      }
		}

		return false;
	}
	return false;
}

bool vrcvi_core::pulse_t::tick()
{
	if (!m_divider.m_enable)
	{
		return false;
	}

	if (vrcvi_core::alu_t::tick())
	{
		m_cycle = (m_cycle+1)&15;
	}

	return m_control.m_mode ? true : ((m_cycle > m_control.m_duty) ? true : false);
}

bool vrcvi_core::sawtooth_t::tick()
{
	if (!m_divider.m_enable)
	{
		return false;
	}

	if (vrcvi_core::alu_t::tick())
	{
		if ((m_cycle++)&1)
		{  // Even step only
			m_accum += m_rate;
		}
		if (m_cycle >= 14)	// Reset accumulator at every 14 cycles
		{
			m_accum = 0;
			m_cycle = 0;
		}
	}
	return (m_accum != 0);
}

s8 vrcvi_core::pulse_t::get_output()
{
	// add 4 bit pulse output
	m_out = tick() ? m_control.m_volume : 0;
	return m_out;
}

s8 vrcvi_core::sawtooth_t::get_output()
{
	// add 5 bit sawtooth output
	m_out = tick() ? ((m_accum>>3)&31) : 0;
	return m_out;
}

void vrcvi_core::alu_t::reset()
{
	m_divider.reset();
	m_counter = 0;
	m_cycle	  = 0;
	m_out	  = 0;
}

void vrcvi_core::pulse_t::reset()
{
	vrcvi_core::alu_t::reset();
	m_control.reset();
}

void vrcvi_core::sawtooth_t::reset()
{
	vrcvi_core::alu_t::reset();
	m_rate	= 0;
	m_accum = 0;
}

bool vrcvi_core::timer_t::tick()
{
	if (m_timer_control.enable())
	{
		if (!m_timer_control.sync())  // scanline sync mode
		{
			m_prescaler -= 3;
			if (m_prescaler <= 0)
			{
				m_prescaler += 341;
				return true;
			}
		}
	}
	return (m_timer_control.enable() && m_timer_control.sync()) ? true : false;
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
		m_enable  = bitfield(data, 7);
	}
	else
	{
		m_divider = (m_divider & ~0x0ff) | data;
	}
}

void vrcvi_core::pulse_w(u8 voice, u8 address, u8 data)
{
	pulse_t &v = m_pulse[voice];
	switch (address)
	{
		case 0x00:	// Control - 0x9000 (Pulse 1), 0xa000 (Pulse 2)
			v.control().write(data);
			break;
		case 0x01:	// Pitch LSB - 0x9001/0x9002 (Pulse 1), 0xa001/0xa002 (Pulse 2)
			v.divider().write(false, data);
			break;
		case 0x02:	// Pitch MSB, Enable/Disable - 0x9002/0x9001 (Pulse 1), 0xa002/0xa001 (Pulse 2)
			v.divider().write(true, data);
			if (!v.divider().m_enable)
			{  // Reset duty cycle
				v.clear_cycle();
			}
			break;
	}
}

void vrcvi_core::saw_w(u8 address, u8 data)
{
	switch (address)
	{
		case 0x00:	// Sawtooth Accumulate - 0xb000
			m_sawtooth.set_rate(bitfield(data, 0, 6));
			break;
		case 0x01:	// Pitch LSB - 0xb001/0xb002 (Sawtooth)
			m_sawtooth.divider().write(false, data);
			break;
		case 0x02:	// Pitch MSB, Enable/Disable - 0xb002/0xb001 (Sawtooth)
			m_sawtooth.divider().write(true, data);
			if (!m_sawtooth.divider().m_enable)
			{  // Reset accumulator
				m_sawtooth.clear_accum();
			}
			break;
	}
}

void vrcvi_core::timer_w(u8 address, u8 data)
{
	switch (address)
	{
		case 0x00:	// Timer latch - 0xf000
			m_timer.set_counter_latch(data);
			break;
		case 0x01:	// Timer control - 0xf001/0xf002
			m_timer.timer_control_w(data);
			break;
		case 0x02:	// IRQ Acknowledge - 0xf002/0xf001
			m_timer.irq_ack();
			break;
	}
}

void vrcvi_core::control_w(u8 data)
{
	// Global control - 0x9003
	m_control.write(data);
}
