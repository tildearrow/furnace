/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Ensoniq ES5504/ES5505/ES5506 Shared Filter emulation core
*/

#include "es550x.hpp"


// Yn = K*(Xn - Yn-1) + Yn-1
#define lp_exec(coeff,in,out) \
  m_o[out][1] = m_o[out][0]; \
  m_o[out][0] = ((coeff * (m_o[in][0] - m_o[out][0])) / 4096) + m_o[out][0];

// Yn = Xn - Xn-1 + K*Yn-1
#define hp_exec(coeff,in,out) \
  m_o[out][1] = m_o[out][0]; \
  m_o[out][0] = m_o[in][0] - m_o[in][1] + ((coeff * m_o[out][0]) / 8192) + (m_o[out][0] / 2);

// Filter functions
void es550x_shared_core::es550x_voice_t::es550x_filter_t::reset()
{
	m_lp = 0;
	m_k2 = 0;
	m_k1 = 0;
	memset(m_o,0,2*5*sizeof(s32));
}

void es550x_shared_core::es550x_voice_t::es550x_filter_t::tick(s32 in)
{
	// set sample input
	m_o[0][0]	 = in;

	s32 coeff_k1 = s32(bitfield(m_k1, 4, 12));	// 12 MSB used
	s32 coeff_k2 = s32(bitfield(m_k2, 4, 12));	// 12 MSB used

	// First and second stage: LP/K1, LP/K1 Fixed
	lp_exec(coeff_k1, 0, 1);
	lp_exec(coeff_k1, 1, 2);
	switch (m_lp)
	{
		case 0:	 // LP3 = 0, LP4 = 0: HP/K2, HP/K2
		default:
			hp_exec(coeff_k2, 2, 3);
			hp_exec(coeff_k2, 3, 4);
			break;
		case 1:	 // LP3 = 0, LP4 = 1: HP/K2, LP/K1
			lp_exec(coeff_k1, 2, 3);
			hp_exec(coeff_k2, 3, 4);
			break;
		case 2:	 // LP3 = 1, LP4 = 0: LP/K2, LP/K2
			lp_exec(coeff_k2, 2, 3);
			lp_exec(coeff_k2, 3, 4);
			break;
		case 3:	 // LP3 = 1, LP4 = 1: LP/K2, LP/K1
			lp_exec(coeff_k1, 2, 3);
			lp_exec(coeff_k2, 3, 4);
			break;
	}
}
