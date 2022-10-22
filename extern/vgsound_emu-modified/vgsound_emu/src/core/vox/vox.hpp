/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	OKI/Dialogic ADPCM core
*/

#ifndef _VGSOUND_EMU_SRC_CORE_VOX_VOX_HPP
#define _VGSOUND_EMU_SRC_CORE_VOX_VOX_HPP

#pragma once

#include "../core.hpp"

using namespace vgsound_emu;

class vox_core : public vgsound_emu_core
{
	protected:
		class vox_decoder_t : public vgsound_emu_core
		{
			private:
				class decoder_state_t : public vgsound_emu_core
				{
					public:
						decoder_state_t(vox_core &vox, bool wraparound)
							: vgsound_emu_core("vox_decoder_state")
							, m_wraparound(wraparound)
							, m_vox(vox)
							, m_index(0)
							, m_step(16)
						{
						}

						// internal states
						void reset();
						void decode(u8 nibble);

						// getters
						s8 index() { return m_index; }

						s32 step() { return m_step; }

						void copy_state(decoder_state_t &src);

					private:
						const bool m_wraparound = false;  // wraparound or clamp?

						vox_core &m_vox;
						s8 m_index = 0;
						s32 m_step = 16;
				};

			public:
				vox_decoder_t(vox_core &vox, bool wraparound)
					: vgsound_emu_core("vox_decoder")
					, m_curr(vox, wraparound)
					, m_loop(vox, wraparound)
					, m_loop_saved(false)
				{
				}

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
						m_loop.copy_state(m_curr);
						m_loop_saved = true;
					}
				}

				void restore()
				{
					if (m_loop_saved)
					{
						m_curr.copy_state(m_loop);
					}
				}

				void decode(u8 nibble) { m_curr.decode(nibble); }

				s32 step() { return m_curr.step(); }

			private:
				decoder_state_t m_curr;
				decoder_state_t m_loop;
				bool m_loop_saved = false;
		};

		const s8 m_index_table[8]  = {-1, -1, -1, -1, 2, 4, 6, 8};
		const s32 m_step_table[49] = {
		  16,  17,	19,	 21,  23,  25,	28,	 31,  34,  37,	41,	  45,	50,	  55,	60,	 66,  73,
		  80,  88,	97,	 107, 118, 130, 143, 157, 173, 190, 209,  230,	253,  279,	307, 337, 371,
		  408, 449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552};

	public:
		vox_core(std::string tag)
			: vgsound_emu_core(tag)
		{
		}
};

#endif
