/*
	License: BSD-3-Clause
	see https://github.com/cam900/vgsound_emu/blob/vgsound_emu_v1/LICENSE for more details

	Copyright holder(s): cam900
	Modifiers and Contributors for Furnace: tildearrow
	Dialogic ADPCM core
*/

#include "util.hpp"
#include <algorithm>
#include <memory>

#ifndef _VGSOUND_EMU_CORE_VOX_HPP
#define _VGSOUND_EMU_CORE_VOX_HPP

#pragma once

#define MODIFIED_CLAMP(x,xMin,xMax) (std::min(std::max((x),(xMin)),(xMax)))

class vox_core
{
protected:
	struct vox_decoder_t
	{
		vox_decoder_t(vox_core &vox)
			: m_curr(vox)
			, m_loop(vox)
		{ };

		virtual void reset()
		{
			m_curr.reset();
			m_loop.reset();
			m_loop_saved = false;
		}

		void save()
		{
			if (!m_loop_saved)
			{
				m_loop.copy(m_curr);
				m_loop_saved = true;
			}
		}

		void restore()
		{
			if (m_loop_saved)
				m_curr.copy(m_loop);
		}

		s32 out() { return m_curr.m_step; }

		struct decoder_state_t
		{
			decoder_state_t(vox_core &vox)
				: m_vox(vox)
			{ };

			void reset()
			{
				m_index = 0;
				m_step = 16;
			}

			void copy(decoder_state_t src)
			{
				m_index = src.m_index;
				m_step = src.m_step;
			}

			void decode(u8 nibble)
			{
				const u8 delta = bitfield(nibble, 0, 3); 
				s16 ss = m_vox.m_step_table[m_index]; // ss(n)

				// d(n) = (ss(n) * B2) + ((ss(n) / 2) * B1) + ((ss(n) / 4) * B0) + (ss(n) / 8)
				s16 d = ss >> 3;
				if (bitfield(delta, 2))
					d += ss;
				if (bitfield(delta, 1))
					d += (ss >> 1);
				if (bitfield(delta, 0))
					d += (ss >> 2);

				// if (B3 = 1) then d(n) = d(n) * (-1) X(n) = X(n-1) * d(n)
				if (bitfield(nibble, 3))
					m_step = std::max(m_step - d, -2048);
				else
					m_step = std::min(m_step + d, 2047);

				// adjust step index
				m_index = MODIFIED_CLAMP(m_index + m_vox.m_index_table[delta], 0, 48);
			}

			vox_core &m_vox;
			s8 m_index = 0;
			s32 m_step = 16;
		};

		decoder_state_t m_curr;
		decoder_state_t m_loop;
		bool m_loop_saved = false;
	};

	s8 m_index_table[8] = {-1, -1, -1, -1, 2, 4, 6, 8};
	s32 m_step_table[49] = {
		16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 
		50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143,
		157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 
		494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552 
	};
};

#endif
