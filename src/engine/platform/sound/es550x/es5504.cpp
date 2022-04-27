/*
	License: BSD-3-Clause
	see https://github.com/cam900/vgsound_emu/LICENSE for more details

	Copyright holder(s): cam900
	Ensoniq ES5504 emulation core

	see es550x.cpp for more info
*/

#include "es5504.hpp"

// Internal functions
void es5504_core::tick()
{
	// /CAS, E
	if (m_clkin.falling_edge()) // falling edge triggers /CAS, E clock
	{
		// /CAS
		if (m_cas.tick())
		{
			// /CAS high, E low: get sample address
			if (m_cas.falling_edge())
			{
				// /CAS low, E low: fetch sample
				if (!m_e.current_edge())
					m_voice[m_voice_cycle].fetch(m_voice_cycle, m_voice_fetch);
			}
		}
		// E
		if (m_clkin.falling_edge()) // falling edge triggers E clock
		{
			if (m_e.tick())
			{
				m_intf.e_pin(m_e.current_edge());
				if (m_e.rising_edge()) // Host access
				{
					m_host_intf.m_rw = m_host_intf.m_rw_strobe;
					m_host_intf.m_host_access = m_host_intf.m_host_access_strobe;
					voice_tick();
				}
				if (m_e.falling_edge()) // Voice memory
				{
					m_host_intf.m_host_access = false;
					m_voice[m_voice_cycle].fetch(m_voice_cycle, m_voice_fetch);
				}
			}
			if (m_e.current_edge()) // Host interface
			{
				if (m_host_intf.m_host_access)
				{
					if (m_host_intf.m_rw && (m_e.cycle() == 2)) // Read
					{
						m_hd = read(m_ha);
						m_host_intf.m_host_access = false;
					}
					else if ((!m_host_intf.m_rw) && (m_e.cycle() == 2)) // Write
						write(m_ha, m_hd);
				}
			}
			else if (!m_e.current_edge())
			{
				if (m_e.cycle() == 2)
				{
					// reset host access state
					m_hd = 0;
					m_host_intf.m_host_access_strobe = false;
				}
			}
		}
	}
}

// less cycle accurate, but less CPU heavy routine
void es5504_core::tick_perf()
{
	// update
	// falling edge
	m_e.m_edge.set(false);
	m_intf.e_pin(false);
	m_host_intf.m_host_access = m_host_intf.m_host_access_strobe = false;
	m_voice[m_voice_cycle].fetch(m_voice_cycle, m_voice_fetch);
	voice_tick();
	// rising edge
	m_e.m_edge.set(true);
	m_intf.e_pin(true);
	m_host_intf.m_rw = m_host_intf.m_rw_strobe;
	m_host_intf.m_host_access = m_host_intf.m_host_access_strobe;
	// falling edge
	m_e.m_edge.set(false);
	m_intf.e_pin(false);
	m_host_intf.m_host_access = m_host_intf.m_host_access_strobe = false;
	m_voice[m_voice_cycle].fetch(m_voice_cycle, m_voice_fetch);
	voice_tick();
	// rising edge
	m_e.m_edge.set(true);
	m_intf.e_pin(true);
	m_host_intf.m_rw = m_host_intf.m_rw_strobe;
	m_host_intf.m_host_access = m_host_intf.m_host_access_strobe;
}

void es5504_core::voice_tick()
{
	// Voice updates every 2 E clock cycle (= 1 CHSTRB cycle or 4 BCLK clock cycle)
	if (bitfield(m_voice_fetch++, 0))
	{
		// Update voice
		m_voice[m_voice_cycle].tick(m_voice_cycle);

		// Refresh output (Multiplexed analog output)
		m_ch[m_voice[m_voice_cycle].m_cr.ca] = m_voice[m_voice_cycle].m_ch;

		if ((++m_voice_cycle) > std::min<u8>(24, m_active)) // ~ 25 voices
			m_voice_cycle = 0;

		m_voice_fetch = 0;
	}
}

void es5504_core::voice_t::fetch(u8 voice, u8 cycle)
{
	m_alu.m_sample[cycle] = m_host.m_intf.read_sample(voice, bitfield(m_cr.ca, 0, 3), bitfield(m_alu.get_accum_integer() + cycle, 0, m_alu.m_integer));
}

void es5504_core::voice_t::tick(u8 voice)
{
	m_ch = 0;

	// Filter execute
	m_filter.tick(m_alu.interpolation());

	if (m_alu.busy())
	{
		// Send to output
		m_ch = ((sign_ext<s32>(m_filter.m_o4_1, 16) >> 3) * m_volume) >> 12; // Analog multiplied in real chip, 13/12 bit ladder DAC

		// ALU execute
		if (m_alu.tick())
		{
			m_alu.loop_exec();
		}

		// ADC check
		adc_exec();
	}

	// Update IRQ
	m_alu.irq_exec(m_host.m_intf, m_host.m_irqv, voice);
}

// ADC; Correct?
void es5504_core::voice_t::adc_exec()
{
	if (m_cr.adc)
		m_host.m_adc = m_host.m_intf.adc_r() & ~0x7;
}

void es5504_core::reset()
{
	es550x_shared_core::reset();
	for (auto & elem : m_voice)
		elem.reset();

	m_adc = 0;
	std::fill(std::begin(m_ch), std::end(m_ch), 0);
}

void es5504_core::voice_t::reset()
{
	es550x_shared_core::es550x_voice_t::reset();
	m_volume = 0;
	m_ch = 0;
}

// Accessors
u16 es5504_core::host_r(u8 address)
{
	if (!m_host_intf.m_host_access)
	{
		m_ha = address;
		if (m_e.rising_edge()) // update directly
			m_hd = read(m_ha, true);
		else
		{
			m_host_intf.m_rw_strobe = true;
			m_host_intf.m_host_access_strobe = true;
		}
	}
	return m_hd;
}

void es5504_core::host_w(u8 address, u16 data)
{
	if (!m_host_intf.m_host_access)
	{
		m_ha = address;
		m_hd = data;
		if (m_e.rising_edge()) // update directly
			write(m_ha, m_hd, true);
		else
		{
			m_host_intf.m_rw_strobe = false;
			m_host_intf.m_host_access_strobe = true;
		}
	}
}

u16 es5504_core::read(u8 address, bool cpu_access)
{
	return regs_r(m_page, address, cpu_access);
}

void es5504_core::write(u8 address, u16 data, bool cpu_access)
{
	regs_w(m_page, address, data, cpu_access);
}

u16 es5504_core::regs_r(u8 page, u8 address, bool cpu_access)
{
	u16 ret = 0xffff;
	address = bitfield(address, 0, 4); // 4 bit address for CPU access

	if (address >= 12) // Global registers
	{
		switch (address)
		{
			case 12: // A/D (A to D Convert/Test)
				ret = (ret & ~0xfffb) | (m_adc & 0xfffb);
				break;
			case 13: // ACT (Number of voices)
				ret = (ret & ~0x1f) | bitfield(m_active, 0, 5);
				break;
			case 14: // IRQV (Interrupting voice vector)
				ret = (ret & ~0x9f) | (m_irqv.irqb ? 0x80 : 0) | bitfield(m_irqv.voice, 0, 5);
				if (cpu_access)
				{
					m_irqv.clear();
					if (bitfield(ret, 7) != m_irqv.irqb)
						m_voice[m_irqv.voice].m_alu.irq_update(m_intf, m_irqv);
				}
				break;
			case 15: // PAGE (Page select register)
				ret = (ret & ~0x3f) | bitfield(m_page, 0, 6);
				break;
		}
	}
	else // Voice specific registers
	{
		const u8 voice = bitfield(page, 0, 5); // Voice select
		if (voice < 25)
		{
			voice_t &v = m_voice[voice];
			if (bitfield(page, 5)) // Page 32 - 56
			{
				switch (address)
				{
					case 1: // O4(n-1) (Filter 4 Temp Register)
						ret = v.m_filter.m_o4_1;
						break;
					case 2: // O3(n-2) (Filter 3 Temp Register #2)
						ret = v.m_filter.m_o3_2;
						break;
					case 3: // O3(n-1) (Filter 3 Temp Register #1)
						ret = v.m_filter.m_o3_1;
						break;
					case 4: // O2(n-2) (Filter 2 Temp Register #2)
						ret = v.m_filter.m_o2_2;
						break;
					case 5: // O2(n-1) (Filter 2 Temp Register #1)
						ret = v.m_filter.m_o2_1;
						break;
					case 6: // O1(n-1) (Filter 1 Temp Register)
						ret = v.m_filter.m_o1_1;
						break;
				}
			}
			else // Page 0 - 24
			{
				switch (address)
				{
					case 0: // CR (Control Register)
						ret = (ret & ~0xff) | 
						      (v.m_alu.m_cr.stop0 ? 0x01 : 0x00)
						    | (v.m_alu.m_cr.stop1 ? 0x02 : 0x00)
						    | (v.m_cr.adc         ? 0x04 : 0x00)
						    | (v.m_alu.m_cr.lpe   ? 0x08 : 0x00)
						    | (v.m_alu.m_cr.ble   ? 0x10 : 0x00)
						    | (v.m_alu.m_cr.irqe  ? 0x20 : 0x00)
						    | (v.m_alu.m_cr.dir   ? 0x40 : 0x00)
						    | (v.m_alu.m_cr.irq   ? 0x80 : 0x00);
						break;
					case 1: // FC (Frequency Control)
						ret = (ret & ~0xfffe) | (v.m_alu.m_fc << 1);
						break;
					case 2: // STRT-H (Loop Start Register High)
						ret = (ret & ~0x1fff) | bitfield(v.m_alu.m_start, 16, 13);
						break;
					case 3: // STRT-L (Loop Start Register Low)
						ret = (ret & ~0xffe0) | (v.m_alu.m_start & 0xffe0);
						break;
					case 4: // END-H (Loop End Register High)
						ret = (ret & ~0x1fff) | bitfield(v.m_alu.m_end, 16, 13);
						break;
					case 5: // END-L (Loop End Register Low)
						ret = (ret & ~0xffe0) | (v.m_alu.m_end & 0xffe0);
						break;
					case 6: // K2 (Filter Cutoff Coefficient #2)
						ret = (ret & ~0xfff0) | (v.m_filter.m_k2 & 0xfff0);
						break;
					case 7: // K1 (Filter Cutoff Coefficient #1)
						ret = (ret & ~0xfff0) | (v.m_filter.m_k1 & 0xfff0);
						break;
					case 8: // Volume
						ret = (ret & ~0xfff0) | ((v.m_volume << 4) & 0xfff0);
						break;
					case 9: // CA (Filter Config, Channel Assign)
						ret = (ret & ~0x3f) | 
						       bitfield(v.m_cr.ca, 0, 4)
						    | (bitfield(v.m_filter.m_lp, 0, 2) << 4);
						break;
					case 10: // ACCH (Accumulator High)
						ret = (ret & ~0x1fff) | bitfield(v.m_alu.m_accum, 16, 13);
						break;
					case 11: // ACCL (Accumulator Low)
						ret = bitfield(v.m_alu.m_accum, 0, 16);
						break;
				}
			}
		}
	}

	return ret;
}

void es5504_core::regs_w(u8 page, u8 address, u16 data, bool cpu_access)
{
	address = bitfield(address, 0, 4); // 4 bit address for CPU access

	if (address >= 12) // Global registers
	{
		switch (address)
		{
			case 12: // A/D (A to D Convert/Test)
				if (bitfield(m_adc, 0)) // Writable ADC
				{
					m_adc = (m_adc & 7) | (data & ~7);
					m_intf.adc_w(m_adc & ~7);
				}
				m_adc = (m_adc & ~3) | (data & 3);
				break;
			case 13: // ACT (Number of voices)
				m_active = std::min<u8>(24, bitfield(data, 0, 5));
				break;
			case 14: // IRQV (Interrupting voice vector)
				// Read only
				break;
			case 15: // PAGE (Page select register)
				m_page = bitfield(data, 0, 6);
				break;
		}
	}
	else // Voice specific registers
	{
		const u8 voice = bitfield(page, 0, 5); // Voice select
		if (voice < 25)
		{
			voice_t &v = m_voice[voice];
			if (bitfield(page, 5)) // Page 32 - 56
			{
				switch (address)
				{
					case 1: // O4(n-1) (Filter 4 Temp Register)
						v.m_filter.m_o4_1 = sign_ext<s32>(data, 16);
						break;
					case 2: // O3(n-2) (Filter 3 Temp Register #2)
						v.m_filter.m_o3_2 = sign_ext<s32>(data, 16);
						break;
					case 3: // O3(n-1) (Filter 3 Temp Register #1)
						v.m_filter.m_o3_1 = sign_ext<s32>(data, 16);
						break;
					case 4: // O2(n-2) (Filter 2 Temp Register #2)
						v.m_filter.m_o2_2 = sign_ext<s32>(data, 16);
						break;
					case 5: // O2(n-1) (Filter 2 Temp Register #1)
						v.m_filter.m_o2_1 = sign_ext<s32>(data, 16);
						break;
					case 6: // O1(n-1) (Filter 1 Temp Register)
						v.m_filter.m_o1_1 = sign_ext<s32>(data, 16);
						break;
				}
			}
			else // Page 0 - 24
			{
				switch (address)
				{
					case 0: // CR (Control Register)
						v.m_alu.m_cr.stop0 = bitfield(data, 0);
						v.m_alu.m_cr.stop1 = bitfield(data, 1);
						v.m_cr.adc         = bitfield(data, 2);
						v.m_alu.m_cr.lpe   = bitfield(data, 3);
						v.m_alu.m_cr.ble   = bitfield(data, 4);
						v.m_alu.m_cr.irqe  = bitfield(data, 5);
						v.m_alu.m_cr.dir   = bitfield(data, 6);
						v.m_alu.m_cr.irq   = bitfield(data, 7);
						break;
					case 1: // FC (Frequency Control)
						v.m_alu.m_fc = bitfield(data, 1, 15);
						break;
					case 2: // STRT-H (Loop Start Register High)
						v.m_alu.m_start = (v.m_alu.m_start & ~0x1fff0000) | (bitfield<u32>(data, 0, 13) << 16);
						break;
					case 3: // STRT-L (Loop Start Register Low)
						v.m_alu.m_start = (v.m_alu.m_start & ~0xffe0) | (data & 0xffe0);
						break;
					case 4: // END-H (Loop End Register High)
						v.m_alu.m_end = (v.m_alu.m_end & ~0x1fff0000) | (bitfield<u32>(data, 0, 13) << 16);
						break;
					case 5: // END-L (Loop End Register Low)
						v.m_alu.m_end = (v.m_alu.m_end & ~0xffe0) | (data & 0xffe0);
						break;
					case 6: // K2 (Filter Cutoff Coefficient #2)
						v.m_filter.m_k2 = data & 0xfff0;
						break;
					case 7: // K1 (Filter Cutoff Coefficient #1)
						v.m_filter.m_k1 = data & 0xfff0;
						break;
					case 8: // Volume
						v.m_volume = bitfield(data, 4, 12);
						break;
					case 9: // CA (Filter Config, Channel Assign)
						v.m_cr.ca       = bitfield(data, 0, 4);
						v.m_filter.m_lp = bitfield(data, 4, 2);
						break;
					case 10: // ACCH (Accumulator High)
						v.m_alu.m_accum = (v.m_alu.m_accum & ~0x1fff0000) | (bitfield<u32>(data, 0, 13) << 16);
						break;
					case 11: // ACCL (Accumulator Low)
						v.m_alu.m_accum = (v.m_alu.m_accum & ~0xffff) | data;
						break;
				}
			}
		}
	}
}
