/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Various core utilities for vgsound_emu
*/

#ifndef _VGSOUND_EMU_SRC_CORE_UTIL_HPP
#define _VGSOUND_EMU_SRC_CORE_UTIL_HPP

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

namespace vgsound_emu
{
	typedef unsigned char u8;
	typedef unsigned short u16;
	typedef unsigned int u32;
	typedef unsigned long long u64;
	typedef signed char s8;
	typedef signed short s16;
	typedef signed int s32;
	typedef signed long long s64;
	typedef float f32;
	typedef double f64;

	class vgsound_emu_core
	{
		public:
			// constructors
			vgsound_emu_core(std::string tag)
				: m_tag(tag)
			{
			}

			// getters
			std::string tag() { return m_tag; }

		protected:
			const f64 PI = 3.1415926535897932384626433832795;

			// std::clamp is only for C++17 or later; I use my own code
			template<typename T>
			T clamp(T in, T min, T max)
			{
#if defined(_HAS_CXX17) && _HAS_CXX17
				// just use std::clamp if C++17 or above
				return std::clamp(in, min, max);
#else
				// otherwise, use my own implementation of std::clamp
				return std::min(std::max(in, min), max);
#endif
			}

			// get bitfield, bitfield(input, position, len)
			template<typename T>
			T bitfield(T in, u8 pos, u8 len = 1)
			{
				return (in >> pos) & (len ? (T(1 << len) - 1) : 1);
			}

			// get sign extended value, sign_ext<type>(input, len)
			template<typename T>
			T sign_ext(T in, u8 len)
			{
				len = std::max<u8>(0, (8 * sizeof(T)) - len);
				return T(T(in) << len) >> len;
			}

			// convert attenuation decibel value to gain
			inline f32 dB_to_gain(f32 attenuation) { return std::pow(10.0f, attenuation / 20.0f); }

		private:
			std::string m_tag = "";	 // core tags
	};

	class vgsound_emu_mem_intf : public vgsound_emu_core
	{
		public:
			// constructor
			vgsound_emu_mem_intf()
				: vgsound_emu_core("mem_intf")
			{
			}

			virtual u8 read_byte(u32 address) { return 0; }

			virtual u16 read_word(u32 address) { return 0; }

			virtual u32 read_dword(u32 address) { return 0; }

			virtual u64 read_qword(u32 address) { return 0; }

			virtual void write_byte(u32 address, u8 data) {}

			virtual void write_word(u32 address, u16 data) {}

			virtual void write_dword(u32 address, u32 data) {}

			virtual void write_qword(u32 address, u64 data) {}
	};

	template<typename T>
	class clock_pulse_t : public vgsound_emu_core
	{
		private:
			const T m_init_width = 1;

			class edge_t : public vgsound_emu_core
			{
				private:
					const u8 m_init_edge = 1;

				public:
					edge_t(u8 init_edge = 0)
						: vgsound_emu_core("clock_pulse_edge")
						, m_init_edge(init_edge)
						, m_current(init_edge ^ 1)
						, m_previous(init_edge)
						, m_rising(0)
						, m_falling(0)
						, m_changed(0)
					{
						set(init_edge);
					}

					// internal states
					void reset()
					{
						m_previous = m_init_edge;
						m_current  = m_init_edge ^ 1;
						set(m_init_edge);
					}

					void tick(bool toggle)
					{
						u8 current = m_current;
						if (toggle)
						{
							current ^= 1;
						}
						set(current);
					}

					void set(u8 edge)
					{
						edge	 &= 1;
						m_rising = m_falling = m_changed = 0;
						if (m_current != edge)
						{
							m_changed = 1;
							if (m_current && (!edge))
							{
								m_falling = 1;
							}
							else if ((!m_current) && edge)
							{
								m_rising = 1;
							}
							m_current = edge;
						}
						m_previous = m_current;
					}

					// getters
					inline bool current() { return m_current; }

					inline bool rising() { return m_rising; }

					inline bool falling() { return m_falling; }

					inline bool changed() { return m_changed; }

				private:
					u8 m_current  : 1;	// current edge
					u8 m_previous : 1;	// previous edge
					u8 m_rising	  : 1;	// rising edge
					u8 m_falling  : 1;	// falling edge
					u8 m_changed  : 1;	// changed flag
			};

		public:
			clock_pulse_t(T init_width, u8 init_edge = 0)
				: vgsound_emu_core("clock_pulse")
				, m_init_width(init_width)
				, m_edge(edge_t(init_edge & 1))
				, m_width(init_width)
				, m_width_latch(init_width)
				, m_counter(init_width)
				, m_cycle(0)
			{
			}

			void reset(T init)
			{
				m_edge.reset();
				m_width = m_width_latch = m_counter = init;
				m_cycle								= 0;
			}

			inline void reset() { reset(m_init_width); }

			bool tick(T width = 0)
			{
				bool carry = ((--m_counter) <= 0);
				if (carry)
				{
					if (!width)
					{
						m_width = m_width_latch;
					}
					else
					{
						m_width = width;  // reset width
					}
					m_counter = m_width;
					m_cycle	  = 0;
				}
				else
				{
					m_cycle++;
				}

				m_edge.tick(carry);
				return carry;
			}

			inline void set_width(T width) { m_width = width; }

			inline void set_width_latch(T width) { m_width_latch = width; }

			// Accessors
			inline bool current_edge() { return m_edge.current(); }

			inline bool rising_edge() { return m_edge.rising(); }

			inline bool falling_edge() { return m_edge.falling(); }

			// getters
			edge_t &edge() { return m_edge; }

			inline T cycle() { return m_cycle; }

		private:
			edge_t m_edge;
			T m_width		= 1;  // clock pulse width
			T m_width_latch = 1;  // clock pulse width latch
			T m_counter		= 1;  // clock counter
			T m_cycle		= 0;  // clock cycle
	};
};	// namespace vgsound_emu

using namespace vgsound_emu;

#endif
