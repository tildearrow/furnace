/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): (Author name)
	Template for sound emulation core
*/

#include "template.hpp"

void template_core::tick()
{
	// tick per each clock
}

void template_core::reset()
{
	// reset this chip
	std::fill(m_array.begin(), m_array.end(), 0);  // std::fill() for fill std::array, std::vector
}

/*
template voice function
void template_core::voice_t::tick()
{
	// tick per each voice
}

void template_core::voice_t::reset()
{
	// reset this voice
}
*/
