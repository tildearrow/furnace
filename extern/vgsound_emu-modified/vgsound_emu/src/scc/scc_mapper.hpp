/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Contributor(s): Natt Akuma, James Alan Nguyen, Laurens Holst
	Konami SCC Mapper emulation core
*/

#ifndef _VGSOUND_EMU_SRC_SCC_MAPPER_HPP
#define _VGSOUND_EMU_SRC_SCC_MAPPER_HPP

#pragma once

#include "../core/util/mem_intf.hpp"
#include "scc.hpp"

// MegaROM Mapper with SCC
class k051649_core : public k051649_scc_core
{
		friend class vgsound_emu_mem_intf;	// for megaROM mapper

	private:
		// mapper classes
		class k051649_mapper_t : public vgsound_emu_core
		{
			public:
				k051649_mapper_t()
					: vgsound_emu_core("k051649_mapper")
					, m_bank{0, 1, 2, 3}
				{
				}

				// internal state
				void reset();

				// setters
				inline void set_bank(const u8 slot, const u8 bank) { m_bank[slot & 3] = bank; }

				// getters
				inline u8 bank(const u8 slot) const { return m_bank[slot & 3]; }

			private:
				// registers
				u8 m_bank[4] = {0, 1, 2, 3};
		};

	public:
		// constructor
		k051649_core(vgsound_emu_mem_intf &intf)
			: k051649_scc_core("k051649")
			, m_intf(intf)
			, m_mapper(k051649_mapper_t())
			, m_scc_enable(false)
		{
		}

		// accessors
		u8 read(const u16 address);
		void write(const u16 address, const u8 data);

		virtual void reset() override;

	private:
		vgsound_emu_mem_intf m_intf;
		k051649_mapper_t m_mapper;
		bool m_scc_enable = false;
};

// MegaRAM Mapper with SCC
class k052539_core : public k052539_scc_core
{
		friend class vgsound_emu_mem_intf;	// for megaRAM mapper

	private:
		// mapper classes
		class k052539_mapper_t : public vgsound_emu_core
		{
			public:
				k052539_mapper_t()
					: vgsound_emu_core("k052539_mapper")
					, m_bank{0, 1, 2, 3}
					, m_ram_enable{false}
				{
				}

				// internal state
				void reset();

				// setters
				inline void set_bank(const u8 slot, const u8 bank) { m_bank[slot & 3] = bank; }

				inline void set_ram_enable(const u8 slot, const bool ram_enable)
				{
					m_ram_enable[slot & 3] = ram_enable;
				}

				// getters
				inline u8 bank(const u8 slot) const { return m_bank[slot & 3]; }

				inline bool ram_enable(const u8 slot) const { return m_ram_enable[slot & 3]; }

			private:
				// registers
				std::array<u8, 4> m_bank		 = {0, 1, 2, 3};
				std::array<bool, 4> m_ram_enable = {false};
		};

	public:
		// constructor
		k052539_core(vgsound_emu_mem_intf &intf)
			: k052539_scc_core("k052539")
			, m_intf(intf)
			, m_mapper(k052539_mapper_t())
			, m_scc_enable(false)
			, m_is_sccplus(false)
		{
		}

		// accessors
		u8 read(const u16 address);
		void write(const u16 address, const u8 data);

		virtual void reset() override;

	private:
		vgsound_emu_mem_intf m_intf;
		k052539_mapper_t m_mapper;
		bool m_scc_enable = false;
		bool m_is_sccplus = false;
};

#endif
