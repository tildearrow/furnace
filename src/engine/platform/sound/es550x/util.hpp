/*
	License: BSD-3-Clause
	see https://github.com/cam900/vgsound_emu/blob/vgsound_emu_v1/LICENSE for more details

	Copyright holder(s): cam900
	Various core utilities for vgsound_emu
*/

#ifndef _VGSOUND_EMU_CORE_UTIL_HPP
#define _VGSOUND_EMU_CORE_UTIL_HPP

#pragma once

#include <algorithm>
#include <memory>

typedef unsigned char       u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;
typedef signed char         s8;
typedef signed short       s16;
typedef signed int         s32;

// get bitfield, bitfield(input, position, len)
template<typename T> T bitfield(T in, u8 pos, u8 len = 1)
{
	return (in >> pos) & (len ? (T(1 << len) - 1) : 1);
}

// get sign extended value, sign_ext<type>(input, len)
template<typename T> T sign_ext(T in, u8 len)
{
	len = std::max<u8>(0, (8 * sizeof(T)) - len);
	return T(T(in) << len) >> len;
}

// std::clamp is only for C++17 or later; I use my own code
template<typename T> T clamp(T in, T min, T max)
{
	return std::min(std::max(in, min), max);
}

template<typename T, T InitWidth, u8 InitEdge = 0>
struct clock_pulse_t
{
	void reset(T init = InitWidth)
	{
		m_edge.reset();
		m_width = m_width_latch = m_counter = init;
		m_cycle = 0;
	}

	bool tick(T width = 0)
	{
		bool carry = ((--m_counter) <= 0);
		if (carry)
		{
			if (!width)
				m_width = m_width_latch;
			else
				m_width = width; // reset width
			m_counter = m_width;
			m_cycle = 0;
		}
		else
			m_cycle++;

		m_edge.tick(carry);
		return carry;
	}

	void set_width(T width) { m_width = width; }
	void set_width_latch(T width) { m_width_latch = width; }

	// Accessors
	bool current_edge() { return m_edge.m_current; }
	bool rising_edge() { return m_edge.m_rising; }
	bool falling_edge() { return m_edge.m_rising; }
	T cycle() { return m_cycle; }

	struct edge_t
	{
		edge_t()
			: m_current(InitEdge ^ 1)
			, m_previous(InitEdge)
			, m_rising(0)
			, m_falling(0)
			, m_changed(0)
		{
			set(InitEdge);
		}

		void tick(bool toggle)
		{
			u8 current = m_current;
			if (toggle)
				current ^= 1;
			set(current);
		}

		void set(u8 edge)
		{
			edge &= 1;
			m_rising = m_falling = m_changed = 0;
			if (m_current != edge)
			{
				m_changed = 1;
				if (m_current && (!edge))
					m_falling = 1;
				else if ((!m_current) && edge)
					m_rising = 1;
				m_current = edge;
			}
			m_previous = m_current;
		}

		void reset()
		{
			m_previous = InitEdge;
			m_current = InitEdge ^ 1;
			set(InitEdge);
		}

		u8 m_current  : 1; // current edge
		u8 m_previous : 1; // previous edge
		u8 m_rising   : 1; // rising edge
		u8 m_falling  : 1; // falling edge
		u8 m_changed  : 1; // changed flag
	};

	edge_t m_edge;
	T m_width       = InitWidth; // clock pulse width
	T m_width_latch = InitWidth; // clock pulse width latch
	T m_counter     = InitWidth; // clock counter
	T m_cycle       = 0;         // clock cycle
};

#endif
