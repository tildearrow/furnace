/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Contributor(s): Natt Akuma, James Alan Nguyen, Laurens Holst
	Konami SCC Mapper emulation core
*/

#include "scc_mapper.hpp"

// Mapper
void k051649_core::reset()
{
	k051649_scc_core::reset();
	m_mapper.reset();
	m_scc_enable = false;
}

void k052539_core::reset()
{
	k052539_scc_core::reset();
	m_mapper.reset();
	m_scc_enable = false;
	m_is_sccplus = false;
}

void k051649_core::k051649_mapper_t::reset()
{
	m_bank[0] = 0;
	m_bank[1] = 1;
	m_bank[2] = 2;
	m_bank[3] = 3;
}

void k052539_core::k052539_mapper_t::reset()
{
	m_bank[0] = 0;
	m_bank[1] = 1;
	m_bank[2] = 2;
	m_bank[3] = 3;
	m_ram_enable.fill(false);
}

// Mapper accessors
u8 k051649_core::read(const u16 address)
{
	if ((bitfield(address, 11, 5) == 0b10011) && m_scc_enable)
	{
		return scc_r(false, u8(address));
	}

	return m_intf.read_byte((u32(m_mapper.bank(bitfield(address, 13, 2) ^ 2)) << 13) |
							bitfield(address, 0, 13));
}

u8 k052539_core::read(const u16 address)
{
	// 0x9800-0x9fff SCC
	if ((bitfield(address, 11, 5) == 0b10011) && m_scc_enable && (!m_is_sccplus))
	{
		return scc_r(false, u8(address));
	}

	// 0xb800-0xbfff SCC+
	if ((bitfield(address, 11, 5) == 0b10111) && m_scc_enable && m_is_sccplus)
	{
		return scc_r(true, u8(address));
	}

	return m_intf.read_byte((u32(m_mapper.bank(bitfield(address, 13, 2) ^ 2)) << 13) |
							bitfield(address, 0, 13));
}

void k051649_core::write(const u16 address, const u8 data)
{
	const u16 bank = bitfield(address, 13, 2) ^ 2;
	switch (bitfield(address, 11, 5))
	{
		case 0b01010:  // 0x5000-0x57ff Bank 0
		case 0b01110:  // 0x7000-0x77ff Bank 1
		case 0b10010:  // 0x9000-0x97ff Bank 2
		case 0b10110:  // 0xb000-0xb7ff Bank 3
			m_mapper.set_bank(bank, data);
			m_scc_enable = (bitfield(m_mapper.bank(2), 0, 6) == 0x3f);
			break;
		case 0b10011:  // 0x9800-9fff SCC
			if (m_scc_enable)
			{
				scc_w(false, u8(address), data);
			}
			break;
	}
}

void k052539_core::write(const u16 address, const u8 data)
{
	u8 prev				  = 0;
	bool update			  = false;
	const u16 bank		  = bitfield(address, 13, 2) ^ 2;
	const bool ram_enable = m_mapper.ram_enable(bank);
	if (ram_enable)
	{
		m_intf.write_byte((u32(m_mapper.bank(bank)) << 13) | bitfield(address, 0, 13), data);
	}
	switch (bitfield(address, 11, 5))
	{
		case 0b01010:  // 0x5000-0x57ff Bank 0
		case 0b01110:  // 0x7000-0x77ff Bank 1
		case 0b10010:  // 0x9000-0x97ff Bank 2
		case 0b10110:  // 0xb000-0xb7ff Bank 3
			if (!ram_enable)
			{
				prev = m_mapper.bank(bank);
				m_mapper.set_bank(bank, data);
				update = prev ^ m_mapper.bank(bank);
			}
			break;
		case 0b10011:  // 0x9800-0x9fff SCC
			if ((!ram_enable) && m_scc_enable && (!m_is_sccplus))
			{
				scc_w(false, u8(address), data);
			}
			break;
		case 0b10111:  // 0xb800-0xbfff SCC+, Mapper configuration
			if (bitfield(address, 1, 10) == 0x3ff)
			{
				m_mapper.set_ram_enable(0, bitfield(data, 4) || bitfield(data, 0));
				m_mapper.set_ram_enable(1, bitfield(data, 4) || bitfield(data, 1));
				m_mapper.set_ram_enable(2, bitfield(data, 4) || bitfield(data, 2));
				m_mapper.set_ram_enable(3, bitfield(data, 4));
				prev		 = (m_is_sccplus ? 1 : 0);
				m_is_sccplus = bitfield(data, 5);
				update		 = prev ^ (m_is_sccplus ? 1 : 0);
			}
			else if ((!ram_enable) && m_scc_enable && m_is_sccplus)
			{
				scc_w(true, u8(address), data);
			}
			break;
	}
	if (update)
	{
		m_scc_enable =
		  m_is_sccplus ? bitfield(m_mapper.bank(3), 7) : (bitfield(m_mapper.bank(2), 0, 6) == 0x3f);
	}
}
