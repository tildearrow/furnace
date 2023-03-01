/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Konami K005289 emulation core
*/

#include "k005289.hpp"

void k005289_core::tick(const unsigned int cycles)
{
	for (timer_t &elem : m_timer)
	{
		elem.tick(cycles);
	}
}

void k005289_core::reset()
{
	for (timer_t &elem : m_timer)
	{
		elem.reset();
	}
}

void k005289_core::timer_t::tick(const unsigned int cycles) {
        m_counter-=cycles;
	while (m_counter < 0)
	{
		m_addr	  = bitfield(m_addr + 1, 0, 5);
		m_counter += 0x1000-(m_freq&0xfff);
	}
}

void k005289_core::timer_t::reset()
{
	m_addr	  = 0;
	m_pitch	  = 0;
	m_freq	  = 0;
	m_counter = 0;
}
