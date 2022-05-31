/*
	License: BSD-3-Clause
	see https://github.com/cam900/vgsound_emu/blob/vgsound_emu_v1/LICENSE for more details

	Copyright holder(s): cam900
	Modifiers and Contributors for Furnace: cam900
	Ensoniq ES5504/ES5505/ES5506 Shared Accumulator emulation core

	see es550x.cpp for more info
*/

#include "es550x.hpp"

// Accumulator functions
void es550x_shared_core::es550x_alu_t::reset()
{
	m_cr.reset();
	m_fc = 0;
	m_start = 0;
	m_end = 0;
	m_accum = 0;
	m_sample[0] = m_sample[1] = 0;
}

bool es550x_shared_core::es550x_alu_t::busy()
{
	return ((!m_cr.stop0) && (!m_cr.stop1));
}

bool es550x_shared_core::es550x_alu_t::tick()
{
	if (m_cr.dir)
		m_accum -= m_fc;
	else
		m_accum += m_fc;

	m_accum &= m_accum_mask;
	return ((!m_cr.lei)
	    && ((( m_cr.dir) && (m_accum < m_start))
			||  ((!m_cr.dir) && (m_accum > m_end)))) ? true : false;
}

void es550x_shared_core::es550x_alu_t::loop_exec()
{
	if (m_cr.irqe) // Set IRQ
		m_cr.irq = 1;

	if (m_cr.dir) // Reverse playback
	{
		if (m_cr.lpe) // Loop enable
		{
			if (m_cr.ble) // Bidirectional
			{
				m_cr.dir = 0;
				m_accum = m_start + (m_start - m_accum);
			}
			else// Normal
				m_accum = m_end - (m_start - m_accum);
		}
		else if (m_cr.ble && m_transwave) // m_transwave
		{
			m_cr.lpe = m_cr.ble = 0;
			m_cr.lei = 1; // Loop end ignore
			m_accum = m_end - (m_start - m_accum);
		}
		else // Stop
			m_cr.stop0 = 1;
	}
	else
	{
		if (m_cr.lpe) // Loop enable
		{
			if (m_cr.ble) // Bidirectional
			{
				m_cr.dir = 1;
				m_accum = m_end - (m_end - m_accum);
			}
			else // Normal
				m_accum = (m_accum - m_end) + m_start;
		}
		else if (m_cr.ble && m_transwave) // m_transwave
		{
			m_cr.lpe = m_cr.ble = 0;
			m_cr.lei = 1; // Loop end ignore
			m_accum = (m_accum - m_end) + m_start;
		}
		else // Stop
			m_cr.stop0 = 1;
	}
}

s32 es550x_shared_core::es550x_alu_t::interpolation()
{
	// SF = S1 + ACCfr * (S2 - S1)
	return m_sample[0] + ((bitfield<s32>(m_accum, std::max<s8>(0, m_fraction - 9), 9) * (m_sample[1] - m_sample[0])) >> 9);
}

u32 es550x_shared_core::es550x_alu_t::get_accum_integer()
{
	return bitfield(m_accum, m_fraction, m_integer);
}

void es550x_shared_core::es550x_alu_t::irq_exec(es550x_intf &intf, es550x_irq_t &irqv, u8 index)
{
	const u8 prev = irqv.irqb;
	if (m_cr.irq)
	{
		if (irqv.irqb)
		{
			irqv.set(index);
			m_cr.irq = 0;
		}
	}
	if (prev != irqv.irqb)
		irq_update(intf, irqv);
}
