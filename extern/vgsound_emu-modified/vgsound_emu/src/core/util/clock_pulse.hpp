/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Common clock pulse emulation for vgsound_emu
*/

#ifndef _VGSOUND_EMU_SRC_CORE_UTIL_CLOCK_PULSE_HPP
#define _VGSOUND_EMU_SRC_CORE_UTIL_CLOCK_PULSE_HPP

#pragma once

#include "../core.hpp"

namespace vgsound_emu
{
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
#endif
