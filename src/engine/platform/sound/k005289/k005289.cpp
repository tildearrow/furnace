/*
	License: BSD-3-Clause
	see https://github.com/cam900/vgsound_emu/blob/vgsound_emu_v1/LICENSE for more details

	Copyright holder(s): cam900
	Modifiers and Contributors for Furnace: cam900
	Konami K005289 emulation core

	This chip is used at infamous Konami Bubble System, for part of Wavetable sound generator.
	But seriously, It is just to 2 internal 12 bit timer and address generators, rather than sound generator.

	Everything except for internal counter and address are done by external logic, the chip is only has external address, frequency registers and its update pins.

	Frequency calculation: Input clock / (4096 - Pitch input)
*/

#include "k005289.hpp"

void k005289_core::tick()
{
	for (auto & elem : m_voice)
		elem.tick();
}

void k005289_core::reset()
{
	for (auto & elem : m_voice)
		elem.reset();
}

void k005289_core::voice_t::tick()
{
	if (bitfield(++counter, 0, 12) == 0)
	{
		addr = bitfield(addr + 1, 0, 5);
		counter = freq;
	}
}

void k005289_core::voice_t::reset()
{
		addr = 0;
		pitch = 0;
		freq = 0;
		counter = 0;
}
