/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Common serial latch for vgsound_emu
*/

#ifndef _VGSOUND_EMU_SRC_CORE_UTIL_SERIAL_LATCH_HPP
#define _VGSOUND_EMU_SRC_CORE_UTIL_SERIAL_LATCH_HPP

#pragma once

#include "../core.hpp"

namespace vgsound_emu
{
	template<typename T, u8 S>
	class serial_latch_t : public vgsound_emu_core
	{
		private:
			const u8 m_size			 = clamp<u8>(S, 1, sizeof(8 * T));
			const u8 m_pos			 = m_size - 1;
			const std::size_t m_mask = std::size_t(bitmask<u64>(m_size));

		public:
			// constructor
			serial_latch_t()
				: vgsound_emu_core("serial_latch")
				, m_data(0)
			{
			}

			inline void reset() { m_data = 0; }

			// setters
			inline void write_msb(const bool data)
			{
				m_data = (boolmask<T>(data) << m_pos) | ((m_data & m_mask) >> 1);
			}

			inline void write_lsb(const bool data)
			{
				m_data = ((m_data << 1) & m_mask) | boolmask<T>(data);
			}

			inline void write(const T data) { m_data = data & m_mask; }

			inline void write(const T data, const T mask)
			{
				m_data = (m_data & ~(mask & m_mask)) | (data & (mask & m_mask));
			}

			// getters
			inline bool read_msb() const { return bitfield(m_data, m_pos); }

			inline bool read_lsb() const { return bitfield(m_data, 0); }

			inline T read() const { return m_data; }

		private:
			T m_data = 0;
	};
};	// namespace vgsound_emu
#endif
