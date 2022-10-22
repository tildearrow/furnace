/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Core framework for vgsound_emu
*/

#ifndef _VGSOUND_EMU_SRC_CORE_CORE_HPP
#define _VGSOUND_EMU_SRC_CORE_CORE_HPP

#pragma once

#include "util.hpp"

namespace vgsound_emu
{
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

		private:
			std::string m_tag = "";	 // core tags
	};
};	// namespace vgsound_emu
#endif
