/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): (Author name)
	Template for sound emulation core, also guideline
*/

#ifndef _VGSOUND_EMU_SRC_TEMPLATE_HPP  // _VGSOUND_EMU_ABSOLUTE_PATH_OF_THIS_FILE
#define _VGSOUND_EMU_SRC_TEMPLATE_HPP

#pragma once

#include "../core/core.hpp"
#include "../core/util/mem_intf.hpp"

using namespace vgsound_emu;

class template_core : public vgsound_emu_core
{
		friend class vgsound_emu_mem_intf;	// common memory interface if exists

	private:  // protected: if shares between inheritances
		// place classes and local constants here if exists

		// template voice classes
		class voice_t : public vgsound_emu_core
		{
			public:
				// constructor
				voice_t(template_core &host)
					: vgsound_emu_core("your_voice_tag_here")
					, m_host(host)
					, m_something(0)
				{
				}

				// internal state
				void reset();
				void tick();

				// accessors, getters, setters

				// setters
				void set_something(s32 something) { m_something = something; }

				// getters
				s32 something() { return m_something; }

			private:
				// registers
				template_core &m_host;
				s32 m_something = 0;  // register
		};

	public:
		// place constructor and destructor, getter and setter for local variables,
		// off-chip interfaces, update routine here only if can't be local

		// constructor
		template_core(vgsound_emu_mem_intf &intf)
			: vgsound_emu_core("your_core_tag_here")
			// initialize all variables in constructor, because constructor is also executable
			// anywhere, and it works as initializer.
			, m_voice{*this}
			, m_intf(intf)
		{
			m_array.fill(0);
			m_vector.resize(1,0);
		}

		// accessors, getters, setters

		// internal state
		void reset();
		void tick();

	protected:
		// place local variables and functions here if shares between inheritances

	private:
		// place local variables and functions here

		std::array<voice_t, 1 /*number of voices*/> m_voice;  // voice classes
		vgsound_emu_mem_intf &m_intf;						  // common memory interface
		std::array<u8 /*type*/, 8 /*size of array*/> m_array; // std::array for static size array
		std::vector<u8 /*type*/> m_vector;  // std::vector for variable size array
};

#endif
