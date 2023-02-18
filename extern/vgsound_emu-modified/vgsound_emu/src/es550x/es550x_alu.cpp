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
  m_last_accum=0;
	m_sample[0] = m_sample[1] = 0;
}

bool es550x_shared_core::es550x_voice_t::es550x_alu_t::busy() { return !(m_cr.m_stop0 || m_cr.m_stop1); }

bool es550x_shared_core::es550x_voice_t::es550x_alu_t::tick()
{
  m_last_accum = m_accum;
	if (m_cr.dir())
	{
		m_accum -= m_fc;
	}
	else
	{
		m_accum += m_fc;
	}

	m_accum &= m_accum_mask;
	return ((!m_cr.m_lei) &&
			(((m_cr.m_dir) && (m_accum < m_start)) || ((!m_cr.m_dir) && (m_accum > m_end))))
		   ? true
		   : false;
}

void es550x_shared_core::es550x_voice_t::es550x_alu_t::loop_exec()
{
	if (m_cr.m_dir)	 // Reverse playback
	{
		if (m_cr.m_lpe)	 // Loop enable
		{
			if (m_cr.m_ble)	 // Bidirectional
			{
				m_cr.set_dir(false);
				m_accum = m_start + (m_start - m_accum);
			}
			else
			{  // Normal
				m_accum = m_end - (m_start - m_accum);
			}
		}
		else if (m_cr.m_ble && m_transwave)	 // m_transwave
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
		if (m_cr.m_lpe)	 // Loop enable
		{
			if (m_cr.m_ble)	 // Bidirectional
			{
				m_cr.set_dir(true);
				m_accum = m_end - (m_end - m_accum);
			}
			else
			{  // Normal
				m_accum = (m_accum - m_end) + m_start;
			}
		}
		else if (m_cr.m_ble && m_transwave)	 // m_transwave
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
	return m_sample[0] + (((((int)m_accum>>(int)2)&(int)511) *
						   (m_sample[1] - m_sample[0])) >>
						  9);
}

u32 es550x_shared_core::es550x_voice_t::es550x_alu_t::get_accum_integer()
{
	return (m_accum>>m_fraction)&((1<<m_integer)-1);
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
