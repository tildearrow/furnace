/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Ensoniq ES5504/ES5505/ES5506 emulation core
*/

#include "es550x.hpp"

// Shared functions
void es550x_shared_core::reset()
{
	m_host_intf.reset();
	m_ha   = 0;
	m_hd   = 0;
	m_page = 0;
	m_irqv.reset();
	m_active	   = max_voices() - 1;
	m_voice_cycle  = 0;
	m_voice_fetch  = 0;
	m_voice_update = false;
	m_voice_end	   = false;
	m_clkin.reset();
	m_cas.reset();
	m_e.reset();
}

void es550x_shared_core::es550x_voice_t::reset()
{
	m_cr.reset();
	m_alu.reset();
	m_filter.reset();
}
