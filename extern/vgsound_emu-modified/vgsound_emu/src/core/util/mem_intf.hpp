/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Common memory interface for vgsound_emu
*/

#ifndef _VGSOUND_EMU_SRC_CORE_UTIL_MEM_INTF_HPP
#define _VGSOUND_EMU_SRC_CORE_UTIL_MEM_INTF_HPP

#pragma once

#include "../core.hpp"

namespace vgsound_emu
{
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
};	// namespace vgsound_emu
#endif
