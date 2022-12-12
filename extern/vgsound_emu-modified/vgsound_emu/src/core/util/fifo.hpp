/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Common FIFO memory for vgsound_emu
*/

#ifndef _VGSOUND_EMU_SRC_CORE_UTIL_FIFO_HPP
#define _VGSOUND_EMU_SRC_CORE_UTIL_FIFO_HPP

#pragma once

#include "../core.hpp"

namespace vgsound_emu
{
	class fifo_intf : public vgsound_emu_core
	{
		public:
			// constructor
			fifo_intf()
				: vgsound_emu_core("fifo_intf")
			{
			}

			virtual void empty_w(bool empty) {}

			virtual void full_w(bool full) {}

			virtual void half_full_w(bool half_full) {}

			virtual void overflow_w(bool overflow) {}

			virtual void underflow_w(bool underflow) {}
	};

	template<typename T, std::size_t S>
	class fifo_t : public vgsound_emu_core
	{
			friend class fifo_intf;

		private:
			const std::size_t m_size = S;

		public:
			// constructor
			fifo_t(fifo_intf &intf)
				: vgsound_emu_core("fifo")
				, m_intf(intf)
				, m_data{0}
				, m_read_pos(0)
				, m_write_pos(0)
				, m_used(0)
				, m_empty(1)
				, m_full(0)
				, m_half_full(0)
				, m_overflow(0)
				, m_underflow(0)
			{
			}

			inline void reset()
			{
				m_data.fill(0);
				m_read_pos	= 0;
				m_write_pos = 0;
				m_used		= 0;
				set_empty(true);
				set_full(false);
				set_half_full(false);
				set_overflow(false);
				set_underflow(false);
			}

			// accessors
			inline T read()
			{
				if (m_empty)
				{
					set_underflow(true);
					return 0;
				}
				else
				{
					const T out = m_data[m_read_pos];
					if (m_full)
					{
						set_full(false);
						set_overflow(false);
					}
					if ((++m_read_pos) == m_size)
					{
						m_read_pos = 0;
					}
					--m_used;
					if (m_half_full && (m_used < (m_size >> 1)))
					{
						set_half_full(false);
					}
					if ((!m_empty) && (m_used <= 0))
					{
						set_empty(true);
					}
					return out;
				}
			}

			inline void write(T in)
			{
				if (m_full)
				{
					set_overflow(true);
				}
				else
				{
					m_data[m_write_pos] = in;
					if (m_empty)
					{
						set_empty(false);
						set_underflow(false);
					}
					if ((++m_write_pos) == m_size)
					{
						m_write_pos = 0;
					}
					++m_used;
					if ((!m_half_full) && (m_used >= (m_size >> 1)))
					{
						set_half_full(true);
					}
					if ((!m_full) && (m_used >= m_size))
					{
						set_full(true);
					}
				}
			}

			// getters
			inline T head() const { return m_data[m_write_pos]; }

			inline T tail() const { return m_data[m_read_pos]; }

		private:
			void set_empty(bool empty)
			{
				m_empty = boolmask<u8>(empty);
				m_intf.empty_w(m_empty);
			}

			void set_full(bool full)
			{
				m_full = boolmask<u8>(full);
				m_intf.empty_w(m_full);
			}

			void set_half_full(bool half_full)
			{
				m_half_full = boolmask<u8>(half_full);
				m_intf.empty_w(m_half_full);
			}

			void set_overflow(bool overflow)
			{
				m_overflow = boolmask<u8>(overflow);
				m_intf.empty_w(m_overflow);
			}

			void set_underflow(bool underflow)
			{
				m_underflow = boolmask<u8>(underflow);
				m_intf.empty_w(m_underflow);
			}

			fifo_intf &m_intf;
			std::array<T, S> m_data = {0};
			std::size_t m_read_pos	= 0;
			std::size_t m_write_pos = 0;
			std::size_t m_used		= 0;
			u8 m_empty	   : 1;
			u8 m_full	   : 1;
			u8 m_half_full : 1;
			u8 m_overflow  : 1;
			u8 m_underflow : 1;
	};
};	// namespace vgsound_emu
#endif
