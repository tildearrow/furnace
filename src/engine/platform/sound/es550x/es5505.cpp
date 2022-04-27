/*
	License: BSD-3-Clause
	see https://github.com/cam900/vgsound_emu/LICENSE for more details

	Copyright holder(s): cam900
	Ensoniq ES5505 emulation core

	see es550x.cpp for more info
*/

#include "es5505.hpp"

// Internal functions
void es5505_core::tick()
{
	// CLKIN
	if (m_clkin.tick())
	{
		// SERBCLK
		if (m_clkin.m_edge.m_changed) // BCLK is freely running clock
		{
			if (m_bclk.tick())
			{
				m_intf.bclk(m_bclk.current_edge());
				// Serial output
				if (m_bclk.falling_edge())
				{
					// SERLRCLK
					if (m_lrclk.tick())
						m_intf.lrclk(m_lrclk.current_edge());
				}
				// SERWCLK
				if (m_lrclk.m_edge.m_changed)
					m_wclk = 0;
				if (m_bclk.falling_edge())
				{
					if (m_wclk == ((m_sermode.sony_bb) ? 1 : 0))
					{
						if (m_lrclk.current_edge())
						{
							for (int i = 0; i < 4; i++)
							{
								// copy output
								m_output[i] = m_output_temp[i];
								m_output_latch[i] = m_ch[i];
								m_output_temp[i].reset();
								// clamp to 16 bit (upper 5 bits are overflow guard bits)
								m_output_latch[i].m_left = clamp<s32>(m_output_latch[i].m_left, -0x8000, 0x7fff);
								m_output_latch[i].m_right = clamp<s32>(m_output_latch[i].m_right, -0x8000, 0x7fff);
								// set signed
								if (m_output_latch[i].m_left < 0)
									m_output_temp[i].m_left = -1;
								if (m_output_latch[i].m_right < 0)
									m_output_temp[i].m_right = -1;
							}
						}
						m_wclk_lr = m_lrclk.current_edge();
						m_output_bit = 16;
					}
					s8 output_bit = --m_output_bit;
					if (m_output_bit >= 0)
					{
						for (int i = 0; i < 4; i++)
						{
							if (m_wclk_lr) // Right output
								m_output_temp[i].m_right = (m_output_temp[i].m_right << 1) | bitfield(m_output_latch[i].m_right, output_bit);
							else // Left output
								m_output_temp[i].m_left = (m_output_temp[i].m_left << 1) | bitfield(m_output_latch[i].m_left, output_bit);
						}
					}
					m_wclk++;
				}
			}
		}
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
			if (m_e.tick())
			{
				m_intf.e_pin(m_e.current_edge());
				if (m_e.rising_edge()) // Host access
				{
					m_host_intf.m_rw = m_host_intf.m_rw_strobe;
					m_host_intf.m_host_access = m_host_intf.m_host_access_strobe;
					voice_tick();
				}
				else if (m_e.falling_edge()) // Voice memory
				{
					m_host_intf.m_host_access = false;
					m_voice[m_voice_cycle].fetch(m_voice_cycle, m_voice_fetch);
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
}

// less cycle accurate, but less CPU heavy routine
void es5505_core::tick_perf()
{
	// output
	for (int c = 0; c < 4; c++)
	{
		m_output[c].m_left  = clamp<s32>(m_ch[c].m_left, -0x8000, 0x7fff);
		m_output[c].m_right = clamp<s32>(m_ch[c].m_right, -0x8000, 0x7fff);
	}

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

void es5505_core::voice_tick()
{
	// Voice updates every 2 E clock cycle (or 4 BCLK clock cycle)
	if (bitfield(m_voice_fetch++, 0))
	{
		// Update voice
		m_voice[m_voice_cycle].tick(m_voice_cycle);

		// Refresh output
		if ((++m_voice_cycle) > clamp<u8>(m_active, 7, 31)) // 8 ~ 32 voices
		{
			m_voice_cycle = 0;
			for (auto & elem : m_ch)
				elem.reset();

			for (auto & elem : m_voice)
			{
				m_ch[bitfield(elem.m_cr.ca, 0, 2)].m_left += elem.m_ch.m_left;
				m_ch[bitfield(elem.m_cr.ca, 0, 2)].m_right += elem.m_ch.m_right;
				elem.m_ch.reset();
			}
		}
		m_voice_fetch = 0;
	}
}

void es5505_core::voice_t::fetch(u8 voice, u8 cycle)
{
	m_alu.m_sample[cycle] = m_host.m_intf.read_sample(voice, bitfield(m_cr.bs, 0), bitfield(m_alu.get_accum_integer() + cycle, 0, m_alu.m_integer));
}

void es5505_core::voice_t::tick(u8 voice)
{
	m_ch.reset();

	// Filter execute
	m_filter.tick(m_alu.interpolation());

	if (m_alu.busy())
	{
		// Send to output
		m_ch.m_left = volume_calc(m_lvol, sign_ext<s32>(m_filter.m_o4_1, 16));
		m_ch.m_right = volume_calc(m_rvol, sign_ext<s32>(m_filter.m_o4_1, 16));

		// ALU execute
		if (m_alu.tick())
			m_alu.loop_exec();
	}

	// Update IRQ
	m_alu.irq_exec(m_host.m_intf, m_host.m_irqv, voice);
}

// volume calculation
s32 es5505_core::voice_t::volume_calc(u8 volume, s32 in)
{
	u8 exponent = bitfield(volume, 4, 4);
	u8 mantissa = bitfield(volume, 0, 4);
	return exponent ? (in * s32(0x10 | mantissa)) >> (20 - exponent) : 0;
}

void es5505_core::reset()
{
	es550x_shared_core::reset();
	for (auto & elem : m_voice)
		elem.reset();

	m_sermode.reset();
	m_bclk.reset();
	m_lrclk.reset();
	m_wclk = 0;
	m_wclk_lr = false;
	m_output_bit = 0;
	for (auto & elem : m_ch)
		elem.reset();
	for (auto & elem : m_output)
		elem.reset();
	for (auto & elem : m_output_temp)
		elem.reset();
	for (auto & elem : m_output_latch)
		elem.reset();
}

void es5505_core::voice_t::reset()
{
	es550x_shared_core::es550x_voice_t::reset();
	m_lvol = 0;
	m_rvol = 0;
	m_ch.reset();
}

// Accessors
u16 es5505_core::host_r(u8 address)
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

void es5505_core::host_w(u8 address, u16 data)
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

u16 es5505_core::read(u8 address, bool cpu_access)
{
	return regs_r(m_page, address, cpu_access);
}

void es5505_core::write(u8 address, u16 data, bool cpu_access)
{
	regs_w(m_page, address, data, cpu_access);
}

u16 es5505_core::regs_r(u8 page, u8 address, bool cpu_access)
{
	u16 ret = 0xffff;
	address = bitfield(address, 0, 4); // 4 bit address for CPU access

	if (address >= 13) // Global registers
	{
		switch (address)
		{
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
				ret = (ret & ~0x7f) | bitfield(m_page, 0, 7);
				break;
		}
	}
	else
	{
		if (bitfield(page, 6)) // Channel registers
		{
			switch (address)
			{
				case 0: // CH0L (Channel 0 Left)
				case 2: // CH1L (Channel 1 Left)
				case 4: // CH2L (Channel 2 Left)
					if (!cpu_access) // CPU can't read here
						ret = m_ch[bitfield(address, 0, 2)].m_left;
					break;
				case 1: // CH0R (Channel 0 Right)
				case 3: // CH1R (Channel 1 Right)
				case 5: // CH2R (Channel 2 Right)
					if (!cpu_access) // CPU can't read here
						ret = m_ch[bitfield(address, 0, 2)].m_right;
					break;
				case 6: // CH3L (Channel 3 Left)
					if ((!cpu_access) || m_sermode.adc)
						ret = m_ch[3].m_left;
					break;
				case 7: // CH3R (Channel 3 Right)
					if ((!cpu_access) || m_sermode.adc)
						ret = m_ch[3].m_right;
					break;
				case 8: // SERMODE (Serial Mode)
					ret = (ret & ~0xf807) |
					      (m_sermode.adc ? 0x01 : 0x00)
					    | (m_sermode.test ? 0x02 : 0x00)
					    | (m_sermode.sony_bb ? 0x04 : 0x00)
					    | (bitfield(m_sermode.msb, 0, 5) << 11);
					break;
				case 9: // PAR (Port A/D Register)
					ret = (ret & ~0x3f) | (m_intf.adc_r() & ~0x3f);
					break;
			}
		}
		else // Voice specific registers
		{
			const u8 voice = bitfield(page, 0, 5); // Voice select
			voice_t &v = m_voice[voice];
			if (bitfield(page, 5)) // Page 32 - 63
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
			else // Page 0 - 31
			{
				switch (address)
				{
					case 0: // CR (Control Register)
						ret = (ret & ~0xfff) | 
						      (v.m_alu.m_cr.stop0     ? 0x01 : 0x00)
						    | (v.m_alu.m_cr.stop1     ? 0x02 : 0x00)
						    | (bitfield(v.m_cr.bs, 0) ? 0x04 : 0x00)
						    | (v.m_alu.m_cr.lpe       ? 0x08 : 0x00)
						    | (v.m_alu.m_cr.ble       ? 0x10 : 0x00)
						    | (v.m_alu.m_cr.irqe      ? 0x20 : 0x00)
						    | (v.m_alu.m_cr.dir       ? 0x40 : 0x00)
						    | (v.m_alu.m_cr.irq       ? 0x80 : 0x00)
						    | (bitfield(v.m_cr.ca, 0, 2) << 8)
						    | (bitfield(v.m_filter.m_lp, 0, 2) << 10);
						break;
					case 1: // FC (Frequency Control)
						ret = (ret & ~0xfffe) | (bitfield(v.m_alu.m_fc, 0, 15) << 1);
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
					case 8: // LVOL (Left Volume)
						ret = (ret & ~0xff00) | ((v.m_lvol << 8) & 0xff00);
						break;
					case 9: // RVOL (Right Volume)
						ret = (ret & ~0xff00) | ((v.m_rvol << 8) & 0xff00);
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

void es5505_core::regs_w(u8 page, u8 address, u16 data, bool cpu_access)
{
	address = bitfield(address, 0, 4); // 4 bit address for CPU access

	if (address >= 12) // Global registers
	{
		switch (address)
		{
			case 13: // ACT (Number of voices)
				m_active = clamp<u8>(bitfield(data, 0, 5), 7, 31);
				break;
			case 14: // IRQV (Interrupting voice vector)
				// Read only
				break;
			case 15: // PAGE (Page select register)
				m_page = bitfield(data, 0, 7);
				break;
		}
	}
	else // Voice specific registers
	{
		if (bitfield(page, 6)) // Channel registers
		{
			switch (address)
			{
				case 0: // CH0L (Channel 0 Left)
					if (m_sermode.test)
						m_ch[0].m_left = data;
					break;
				case 1: // CH0R (Channel 0 Right)
					if (m_sermode.test)
						m_ch[0].m_right = data;
					break;
				case 2: // CH1L (Channel 1 Left)
					if (m_sermode.test)
						m_ch[1].m_left = data;
					break;
				case 3: // CH1R (Channel 1 Right)
					if (m_sermode.test)
						m_ch[1].m_right = data;
					break;
				case 4: // CH2L (Channel 2 Left)
					if (m_sermode.test)
						m_ch[2].m_left = data;
					break;
				case 5: // CH2R (Channel 2 Right)
					if (m_sermode.test)
						m_ch[2].m_right = data;
					break;
				case 6: // CH3L (Channel 3 Left)
					if (m_sermode.test)
						m_ch[3].m_left = data;
					break;
				case 7: // CH3R (Channel 3 Right)
					if (m_sermode.test)
						m_ch[3].m_right = data;
					break;
				case 8: // SERMODE (Serial Mode)
					m_sermode.adc = bitfield(data, 0);
					m_sermode.test = bitfield(data, 1);
					m_sermode.sony_bb = bitfield(data, 2);
					m_sermode.msb = bitfield(data, 11, 5);
					break;
				case 9: // PAR (Port A/D Register)
					// Read only
					break;
			}
		}
		else // Voice specific registers
		{
			const u8 voice = bitfield(page, 0, 5); // Voice select
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
						v.m_cr.bs          = bitfield(data, 2);
						v.m_alu.m_cr.lpe   = bitfield(data, 3);
						v.m_alu.m_cr.ble   = bitfield(data, 4);
						v.m_alu.m_cr.irqe  = bitfield(data, 5);
						v.m_alu.m_cr.dir   = bitfield(data, 6);
						v.m_alu.m_cr.irq   = bitfield(data, 7);
						v.m_cr.ca          = bitfield(data, 8, 2);
						v.m_filter.m_lp    = bitfield(data, 10, 2);
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
					case 8: // LVOL (Left Volume)
						v.m_lvol = bitfield(data, 8, 8);
						break;
					case 9: // RVOL (Right Volume)
						v.m_rvol = bitfield(data, 8, 8);
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
