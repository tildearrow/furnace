/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Ensoniq ES5504/ES5505/ES5506 Shared Accumulator emulation core
*/

#include "es550x.hpp"

// Accumulator functions
void es550x_shared_core::es550x_voice_t::es550x_alu_t::reset()
{
	m_cr.reset();
	m_fc		= 0;
	m_start		= 0;
	m_end		= 0;
	m_accum		= 0;
	m_sample[0] = m_sample[1] = 0;
}

bool es550x_shared_core::es550x_voice_t::es550x_alu_t::busy() { return m_cr.stop() == 0; }

bool es550x_shared_core::es550x_voice_t::es550x_alu_t::tick()
{
	if (m_cr.dir())
	{
		m_accum -= m_fc;
	}
	else
	{
		m_accum += m_fc;
	}

	m_accum &= m_accum_mask;
	return ((!m_cr.lei()) &&
			(((m_cr.dir()) && (m_accum < m_start)) || ((!m_cr.dir()) && (m_accum > m_end))))
		   ? true
		   : false;
}

void es550x_shared_core::es550x_voice_t::es550x_alu_t::loop_exec()
{
	if (m_cr.irqe())
	{  // Set IRQ
		m_cr.set_irq(true);
	}

	if (m_cr.dir())	 // Reverse playback
	{
		if (m_cr.lpe())	 // Loop enable
		{
			if (m_cr.ble())	 // Bidirectional
			{
				m_cr.set_dir(false);
				m_accum = m_start + (m_start - m_accum);
			}
			else
			{  // Normal
				m_accum = m_end - (m_start - m_accum);
			}
		}
		else if (m_cr.ble() && m_transwave)	 // m_transwave
		{
			m_cr.set_loop(0);
			m_cr.set_lei(true);	 // Loop end ignore
			m_accum = m_end - (m_start - m_accum);
		}
		else
		{  // Stop
			m_cr.set_stop0(true);
		}
	}
	else
	{
		if (m_cr.lpe())	 // Loop enable
		{
			if (m_cr.ble())	 // Bidirectional
			{
				m_cr.set_dir(true);
				m_accum = m_end - (m_end - m_accum);
			}
			else
			{  // Normal
				m_accum = (m_accum - m_end) + m_start;
			}
		}
		else if (m_cr.ble() && m_transwave)	 // m_transwave
		{
			m_cr.set_loop(0);
			m_cr.set_lei(true);	 // Loop end ignore
			m_accum = (m_accum - m_end) + m_start;
		}
		else
		{  // Stop
			m_cr.set_stop0(true);
		}
	}
}

s32 es550x_shared_core::es550x_voice_t::es550x_alu_t::interpolation()
{
	// SF = S1 + ACCfr * (S2 - S1)
	return m_sample[0] + ((bitfield<s32>(m_accum, std::max<s8>(0, m_fraction - 9), 9) *
						   (m_sample[1] - m_sample[0])) >>
						  9);
}

u32 es550x_shared_core::es550x_voice_t::es550x_alu_t::get_accum_integer()
{
	return bitfield(m_accum, m_fraction, m_integer);
}

void es550x_shared_core::es550x_voice_t::es550x_alu_t::irq_exec(es550x_intf &intf,
																es550x_irq_t &irqv,
																u8 index)
{
	const bool prev = irqv.irqb();
	if (m_cr.irq())
	{
		if (irqv.irqb())
		{
			irqv.set(index);
			m_cr.set_irq(false);
		}
	}
	if (prev != irqv.irqb())
	{
		irq_update(intf, irqv);
	}
}
