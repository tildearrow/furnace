/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Dialogic ADPCM core
*/

#include "vox.hpp"

// reset decoder
void vox_core::vox_decoder_t::decoder_state_t::reset()
{
	m_index = 0;
	m_step	= 16;
}

// copy from source
void vox_core::vox_decoder_t::decoder_state_t::copy(decoder_state_t src)
{
	m_index = src.index();
	m_step	= src.step();
}

// decode single nibble
void vox_core::vox_decoder_t::decoder_state_t::decode(u8 nibble)
{
	const u8 delta = bitfield(nibble, 0, 3);
	const s16 ss   = m_vox.m_step_table[m_index];  // ss(n)

	// d(n) = (ss(n) * B2) + ((ss(n) / 2) * B1) + ((ss(n) / 4) * B0)
	// + (ss(n) / 8)
	s16 d = ss >> 3;
	if (bitfield(delta, 2))
	{
		d += ss;
	}
	if (bitfield(delta, 1))
	{
		d += (ss >> 1);
	}
	if (bitfield(delta, 0))
	{
		d += (ss >> 2);
	}

	// if (B3 = 1) then d(n) = d(n) * (-1) X(n) = X(n-1) * d(n)
	if (bitfield(nibble, 3))
	{
		m_step -= d;
	}
	else
	{
		m_step += d;
	}

	if (m_wraparound)  // wraparound (MSM5205)
	{
		if (m_step < -2048)
		{
			m_step &= 0x7ff;
		}
		else if (m_step > 2047)
		{
			m_step |= ~0x7ff;
		}
	}
	else
	{
		m_step = clamp<s32>(m_step, -2048, 2047);
	}

	// adjust step index
	m_index = clamp(m_index + m_vox.m_index_table[delta], 0, 48);
}
