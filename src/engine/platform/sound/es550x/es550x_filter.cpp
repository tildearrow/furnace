/*
	License: BSD-3-Clause
	see https://github.com/cam900/vgsound_emu/blob/vgsound_emu_v1/LICENSE for more details

	Copyright holder(s): cam900
	Ensoniq ES5504/ES5505/ES5506 Shared Filter emulation core

	see es550x.cpp for more info
*/

#include "es550x.hpp"

// Filter functions
void es550x_shared_core::es550x_filter_t::reset()
{
	m_lp = 0;
	m_k2 = 0;
	m_k1 = 0;
	m_o1_1 = 0;
	m_o2_1 = 0;
	m_o2_2 = 0;
	m_o3_1 = 0;
	m_o3_2 = 0;
	m_o4_1 = 0;
}

void es550x_shared_core::es550x_filter_t::tick(s32 in)
{
	s32 coeff_k1 = s32(bitfield(m_k1, 4, 12)); // 12 MSB used
	s32 coeff_k2 = s32(bitfield(m_k2, 4, 12)); // 12 MSB used
	// Store previous filter data
	m_o2_2 = m_o2_1;
	m_o3_2 = m_o3_1;

	// First and second stage: LP/K1, LP/K1 Fixed
	m_o1_1 = lp_exec(coeff_k1, in, m_o1_1);
	m_o2_1 = lp_exec(coeff_k1, m_o1_1, m_o2_1);
	switch (m_lp)
	{
		case 0: // LP3 = 0, LP4 = 0: HP/K2, HP/K2
		default:
			m_o3_1 = hp_exec(coeff_k2, m_o2_1, m_o3_1, m_o2_2);
			m_o4_1 = hp_exec(coeff_k2, m_o3_1, m_o4_1, m_o3_2);
		break;
		case 1: // LP3 = 0, LP4 = 1: HP/K2, LP/K1
			m_o3_1 = lp_exec(coeff_k1, m_o2_1, m_o3_1);
			m_o4_1 = hp_exec(coeff_k2, m_o3_1, m_o4_1, m_o3_2);
		break;
		case 2: // LP3 = 1, LP4 = 0: LP/K2, LP/K2
			m_o3_1 = lp_exec(coeff_k2, m_o2_1, m_o3_1);
			m_o4_1 = lp_exec(coeff_k2, m_o3_1, m_o4_1);
		break;
		case 3: // LP3 = 1, LP4 = 1: LP/K2, LP/K1
			m_o3_1 = lp_exec(coeff_k1, m_o2_1, m_o3_1);
			m_o4_1 = lp_exec(coeff_k2, m_o3_1, m_o4_1);
		break;
	}
}

s32 es550x_shared_core::es550x_filter_t::lp_exec(s32 coeff, s32 in, s32 prev_out)
{
	// Yn = K*(Xn - Yn-1) + Yn-1
	return ((coeff * (in - prev_out)) / 4096) + prev_out;
}
		
s32 es550x_shared_core::es550x_filter_t::hp_exec(s32 coeff, s32 in, s32 prev_out, s32 prev_in)
{
	// Yn = Xn - Xn-1 + K*Yn-1
	return in - prev_in + ((coeff * prev_out) / 8192) + (prev_out / 2);
}
