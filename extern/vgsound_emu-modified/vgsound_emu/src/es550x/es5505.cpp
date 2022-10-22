/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Ensoniq ES5505 emulation core
*/

#include "es5505.hpp"

// Internal functions
void es5505_core::tick()
{
	// CLKIN
	if (m_clkin.tick())
	{
		// SERBCLK
		if (m_clkin.edge().changed())  // BCLK is freely running clock
		{
			if (m_bclk.tick())
			{
				m_intf.bclk(m_bclk.current_edge());
				// Serial output
				if (m_bclk.falling_edge())
				{
					// SERLRCLK
					if (m_lrclk.tick())
					{
						m_intf.lrclk(m_lrclk.current_edge());
					}
				}
				// SERWCLK
				if (m_lrclk.edge().changed())
				{
					m_wclk = 0;
				}
				if (m_bclk.falling_edge())
				{
					if (m_wclk == ((m_sermode.sony_bb()) ? 1 : 0))
					{
						if (m_lrclk.current_edge())
						{
							for (int i = 0; i < 4; i++)
							{
								// copy output
								m_output[i].copy_output(m_output_temp[i]);
								// clamp to 16 bit (upper 5 bits are overflow
								// guard bits)
								m_output_latch[i].clamp16(m_ch[i]);
								m_output_temp[i].reset();
								// set signed
								if (m_output_latch[i].left() < 0)
								{
									m_output_temp[i].set_left(-1);
								}
								if (m_output_latch[i].right() < 0)
								{
									m_output_temp[i].set_right(-1);
								}
							}
						}
						m_wclk_lr	 = m_lrclk.current_edge();
						m_output_bit = 16;
					}
					s8 output_bit = --m_output_bit;
					if (m_output_bit >= 0)
					{
						for (int i = 0; i < 4; i++)
						{
							if (m_wclk_lr)
							{  // Right output
								m_output_temp[i].serial_in(
								  m_wclk_lr,
								  bitfield(m_output_latch[i].right(), output_bit));
							}
							else
							{  // Left output
								m_output_temp[i].serial_in(
								  m_wclk_lr,
								  bitfield(m_output_latch[i].left(), output_bit));
							}
						}
					}
					m_wclk++;
				}
			}
		}
		// /CAS, E
		if (m_clkin.falling_edge())	 // falling edge triggers /CAS, E clock
		{
			// /CAS
			if (m_cas.tick())
			{
				// /CAS high, E low: get sample address
				if (m_cas.falling_edge())
				{
					// /CAS low, E low: fetch sample
					if (!m_e.current_edge())
					{
						m_voice[m_voice_cycle].fetch(m_voice_cycle, m_voice_fetch);
					}
				}
			}
			// E
			if (m_e.tick())
			{
				m_intf.e_pin(m_e.current_edge());
				if (m_e.rising_edge())	// Host access
				{
					m_host_intf.update_strobe();
					voice_tick();
				}
				else if (m_e.falling_edge())  // Voice memory
				{
					m_host_intf.clear_host_access();
					m_voice[m_voice_cycle].fetch(m_voice_cycle, m_voice_fetch);
				}
				if (m_e.current_edge())	 // Host interface
				{
					if (m_host_intf.host_access())
					{
						if (m_host_intf.rw() && (m_e.cycle() == 2))	 // Read
						{
							m_hd = read(m_ha);
							m_host_intf.clear_host_access();
						}
						else if ((!m_host_intf.rw()) && (m_e.cycle() == 2))
						{  // Write
							write(m_ha, m_hd);
						}
					}
				}
				else if (!m_e.current_edge())
				{
					if (m_e.cycle() == 2)
					{
						// reset host access state
						m_hd = 0;
						m_host_intf.clear_strobe();
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
		m_output[c].clamp16(m_ch[c]);
	}

	// update
	// falling edge
	m_e.edge().set(false);
	m_intf.e_pin(false);
	m_host_intf.clear_host_access();
	m_host_intf.clear_strobe();
	m_voice[m_voice_cycle].fetch(m_voice_cycle, m_voice_fetch);
	voice_tick();
	// rising edge
	m_e.edge().set(true);
	m_intf.e_pin(true);
	m_host_intf.update_strobe();
	// falling edge
	m_e.edge().set(false);
	m_intf.e_pin(false);
	m_host_intf.clear_host_access();
	m_host_intf.clear_strobe();
	m_voice[m_voice_cycle].fetch(m_voice_cycle, m_voice_fetch);
	voice_tick();
	// rising edge
	m_e.edge().set(true);
	m_intf.e_pin(true);
	m_host_intf.update_strobe();
}

void es5505_core::voice_tick()
{
	// Voice updates every 2 E clock cycle (or 4 BCLK clock cycle)
	m_voice_update = bitfield(m_voice_fetch++, 0);
	if (m_voice_update)
	{
		// Update voice
		m_voice[m_voice_cycle].tick(m_voice_cycle);

		// Refresh output
		if ((++m_voice_cycle) > clamp<u8>(m_active, 7, 31))	 // 8 ~ 32 voices
		{
			m_voice_end	  = true;
			m_voice_cycle = 0;
			for (auto &elem : m_ch)
			{
				elem.reset();
			}

			for (auto &elem : m_voice)
			{
				m_ch[bitfield(elem.cr().ca(), 0, 2)] += elem.ch();
				elem.ch().reset();
			}
		}
		else
		{
			m_voice_end = false;
		}
		m_voice_fetch = 0;
	}
}

void es5505_core::voice_t::fetch(u8 voice, u8 cycle)
{
	m_alu.set_sample(
	  cycle,
	  m_host.m_intf.read_sample(voice,
								bitfield(m_cr.bs(), 0),
								bitfield(m_alu.get_accum_integer() + cycle, 0, m_alu.m_integer)));
}

void es5505_core::voice_t::tick(u8 voice)
{
	m_output[0] = m_output[1] = 0;
	m_ch.reset();

	// Filter execute
	m_filter.tick(m_alu.interpolation());

	if (m_alu.busy())
	{
		// Send to output
		m_output[0] = volume_calc(m_lvol, sign_ext<s32>(m_filter.o4_1(), 16));
		m_output[1] = volume_calc(m_rvol, sign_ext<s32>(m_filter.o4_1(), 16));

		m_ch.set_left(m_output[0]);
		m_ch.set_right(m_output[1]);

		// ALU execute
		if (m_alu.tick())
		{
			m_alu.loop_exec();
		}
	}

	// Update IRQ
	m_alu.irq_exec(m_host.m_intf, m_host.m_irqv, voice);
}

// volume calculation
s32 es5505_core::voice_t::volume_calc(u8 volume, s32 in)
{
	u8 exponent = bitfield(volume, 4, 4);
	u8 mantissa = bitfield(volume, 0, 4);
	return exponent ? (in * s32(0x10 | mantissa)) >> (19 - exponent) : 0;
}

void es5505_core::reset()
{
	es550x_shared_core::reset();
	for (auto &elem : m_voice)
	{
		elem.reset();
	}

	m_sermode.reset();
	m_bclk.reset();
	m_lrclk.reset();
	m_wclk		 = 0;
	m_wclk_lr	 = false;
	m_output_bit = 0;
	for (auto &elem : m_ch)
	{
		elem.reset();
	}
	for (auto &elem : m_output)
	{
		elem.reset();
	}
	for (auto &elem : m_output_temp)
	{
		elem.reset();
	}
	for (auto &elem : m_output_latch)
	{
		elem.reset();
	}
}

void es5505_core::voice_t::reset()
{
	es550x_shared_core::es550x_voice_t::reset();
	m_lvol = 0;
	m_rvol = 0;
	m_ch.reset();
	m_mute		= false;
	m_output[0] = m_output[1] = 0;
}

// Accessors
u16 es5505_core::host_r(u8 address)
{
	if (!m_host_intf.host_access())
	{
		m_ha = address;
		if (m_e.rising_edge())
		{  // update directly
			m_hd = read(m_ha, true);
		}
		else
		{
			m_host_intf.set_strobe(true);
		}
	}
	return m_hd;
}

void es5505_core::host_w(u8 address, u16 data)
{
	if (!m_host_intf.host_access())
	{
		m_ha = address;
		m_hd = data;
		if (m_e.rising_edge())
		{  // update directly
			write(m_ha, m_hd);
		}
		else
		{
			m_host_intf.set_strobe(false);
		}
	}
}

u16 es5505_core::read(u8 address, bool cpu_access) { return regs_r(m_page, address, cpu_access); }

void es5505_core::write(u8 address, u16 data)
{
	regs_w(m_page, address, data);
}

u16 es5505_core::regs_r(u8 page, u8 address, bool cpu_access)
{
	u16 ret = 0xffff;
	address = bitfield(address, 0, 4);	// 4 bit address for CPU access

	if (address >= 13)	// Global registers
	{
		switch (address)
		{
			case 13:  // ACT (Number of voices)
				ret = (ret & ~0x1f) | bitfield(m_active, 0, 5);
				break;
			case 14:  // IRQV (Interrupting voice vector)
				ret = (ret & ~0x9f) | m_irqv.get();
				if (cpu_access)
				{
					m_irqv.clear();
					if (bool(bitfield(ret, 7)) != m_irqv.irqb())
					{
						m_voice[m_irqv.voice()].alu().irq_update(m_intf, m_irqv);
					}
				}
				break;
			case 15:  // PAGE (Page select register)
				ret = (ret & ~0x7f) | bitfield(m_page, 0, 7);
				break;
		}
	}
	else
	{
		if (bitfield(page, 6))	// Channel registers
		{
			switch (address)
			{
				case 0:	 // CH0L (Channel 0 Left)
				case 2:	 // CH1L (Channel 1 Left)
				case 4:	 // CH2L (Channel 2 Left)
					if (!cpu_access)
					{  // CPU can't read here
						ret = m_ch[bitfield(address, 0, 2)].left();
					}
					break;
				case 1:	 // CH0R (Channel 0 Right)
				case 3:	 // CH1R (Channel 1 Right)
				case 5:	 // CH2R (Channel 2 Right)
					if (!cpu_access)
					{  // CPU can't read here
						ret = m_ch[bitfield(address, 0, 2)].right();
					}
					break;
				case 6:	 // CH3L (Channel 3 Left)
					if ((!cpu_access) || m_sermode.adc())
					{
						ret = m_ch[3].left();
					}
					break;
				case 7:	 // CH3R (Channel 3 Right)
					if ((!cpu_access) || m_sermode.adc())
					{
						ret = m_ch[3].right();
					}
					break;
				case 8:	 // SERMODE (Serial Mode)
					ret = (ret & ~0xf807) | (m_sermode.adc() ? 0x01 : 0x00) |
						  (m_sermode.test() ? 0x02 : 0x00) | (m_sermode.sony_bb() ? 0x04 : 0x00) |
						  (bitfield(m_sermode.msb(), 0, 5) << 11);
					break;
				case 9:	 // PAR (Port A/D Register)
					ret = (ret & ~0x3f) | (m_intf.adc_r() & ~0x3f);
					break;
			}
		}
		else  // Voice specific registers
		{
			const u8 voice = bitfield(page, 0, 5);	// Voice select
			voice_t &v	   = m_voice[voice];
			if (bitfield(page, 5))	// Page 32 - 63
			{
				switch (address)
				{
					case 1:	 // O4(n-1) (Filter 4 Temp Register)
						ret = v.filter().o4_1();
						break;
					case 2:	 // O3(n-2) (Filter 3 Temp Register #2)
						ret = v.filter().o3_2();
						break;
					case 3:	 // O3(n-1) (Filter 3 Temp Register #1)
						ret = v.filter().o3_1();
						break;
					case 4:	 // O2(n-2) (Filter 2 Temp Register #2)
						ret = v.filter().o2_2();
						break;
					case 5:	 // O2(n-1) (Filter 2 Temp Register #1)
						ret = v.filter().o2_1();
						break;
					case 6:	 // O1(n-1) (Filter 1 Temp Register)
						ret = v.filter().o1_1();
						break;
				}
			}
			else  // Page 0 - 31
			{
				switch (address)
				{
					case 0:	 // CR (Control Register)
						ret = (ret & ~0xfff) | (v.alu().stop() << 0) |
							  (bitfield(v.cr().bs(), 0) ? 0x04 : 0x00) |
							  (v.alu().lpe() ? 0x08 : 0x00) | (v.alu().ble() ? 0x10 : 0x00) |
							  (v.alu().irqe() ? 0x20 : 0x00) | (v.alu().dir() ? 0x40 : 0x00) |
							  (v.alu().irq() ? 0x80 : 0x00) | (bitfield(v.cr().ca(), 0, 2) << 8) |
							  (bitfield(v.filter().lp(), 0, 2) << 10);
						break;
					case 1:	 // FC (Frequency Control)
						ret = (ret & ~0xfffe) | (bitfield(v.alu().fc(), 0, 15) << 1);
						break;
					case 2:	 // STRT-H (Loop Start Register High)
						ret = (ret & ~0x1fff) | bitfield(v.alu().start(), 16, 13);
						break;
					case 3:	 // STRT-L (Loop Start Register Low)
						ret = (ret & ~0xffe0) | (v.alu().start() & 0xffe0);
						break;
					case 4:	 // END-H (Loop End Register High)
						ret = (ret & ~0x1fff) | bitfield(v.alu().end(), 16, 13);
						break;
					case 5:	 // END-L (Loop End Register Low)
						ret = (ret & ~0xffe0) | (v.alu().end() & 0xffe0);
						break;
					case 6:	 // K2 (Filter Cutoff Coefficient #2)
						ret = (ret & ~0xfff0) | (v.filter().k2() & 0xfff0);
						break;
					case 7:	 // K1 (Filter Cutoff Coefficient #1)
						ret = (ret & ~0xfff0) | (v.filter().k1() & 0xfff0);
						break;
					case 8:	 // LVOL (Left Volume)
						ret = (ret & ~0xff00) | ((v.lvol() << 8) & 0xff00);
						break;
					case 9:	 // RVOL (Right Volume)
						ret = (ret & ~0xff00) | ((v.rvol() << 8) & 0xff00);
						break;
					case 10:  // ACCH (Accumulator High)
						ret = (ret & ~0x1fff) | bitfield(v.alu().accum(), 16, 13);
						break;
					case 11:  // ACCL (Accumulator Low)
						ret = bitfield(v.alu().accum(), 0, 16);
						break;
				}
			}
		}
	}

	return ret;
}

void es5505_core::regs_w(u8 page, u8 address, u16 data)
{
	address = bitfield(address, 0, 4);	// 4 bit address for CPU access

	if (address >= 12)	// Global registers
	{
		switch (address)
		{
			case 13:  // ACT (Number of voices)
				m_active = std::max<u8>(7, bitfield(data, 0, 5));
				break;
			case 14:  // IRQV (Interrupting voice vector)
				// Read only
				break;
			case 15:  // PAGE (Page select register)
				m_page = bitfield(data, 0, 7);
				break;
		}
	}
	else  // Voice specific registers
	{
		if (bitfield(page, 6))	// Channel registers
		{
			switch (address)
			{
				case 0:	 // CH0L (Channel 0 Left)
					if (m_sermode.test())
					{
						m_ch[0].set_left(data);
					}
					break;
				case 1:	 // CH0R (Channel 0 Right)
					if (m_sermode.test())
					{
						m_ch[0].set_right(data);
					}
					break;
				case 2:	 // CH1L (Channel 1 Left)
					if (m_sermode.test())
					{
						m_ch[1].set_left(data);
					}
					break;
				case 3:	 // CH1R (Channel 1 Right)
					if (m_sermode.test())
					{
						m_ch[1].set_right(data);
					}
					break;
				case 4:	 // CH2L (Channel 2 Left)
					if (m_sermode.test())
					{
						m_ch[2].set_left(data);
					}
					break;
				case 5:	 // CH2R (Channel 2 Right)
					if (m_sermode.test())
					{
						m_ch[2].set_right(data);
					}
					break;
				case 6:	 // CH3L (Channel 3 Left)
					if (m_sermode.test())
					{
						m_ch[3].set_left(data);
					}
					break;
				case 7:	 // CH3R (Channel 3 Right)
					if (m_sermode.test())
					{
						m_ch[3].set_right(data);
					}
					break;
				case 8:	 // SERMODE (Serial Mode)
					m_sermode.write(data);
					break;
				case 9:	 // PAR (Port A/D Register)
					// Read only
					break;
			}
		}
		else  // Voice specific registers
		{
			const u8 voice = bitfield(page, 0, 5);	// Voice select
			voice_t &v	   = m_voice[voice];
			if (bitfield(page, 5))	// Page 32 - 56
			{
				switch (address)
				{
					case 1:	 // O4(n-1) (Filter 4 Temp Register)
						v.filter().set_o4_1(sign_ext<s32>(data, 16));
						break;
					case 2:	 // O3(n-2) (Filter 3 Temp Register #2)
						v.filter().set_o3_2(sign_ext<s32>(data, 16));
						break;
					case 3:	 // O3(n-1) (Filter 3 Temp Register #1)
						v.filter().set_o3_1(sign_ext<s32>(data, 16));
						break;
					case 4:	 // O2(n-2) (Filter 2 Temp Register #2)
						v.filter().set_o2_2(sign_ext<s32>(data, 16));
						break;
					case 5:	 // O2(n-1) (Filter 2 Temp Register #1)
						v.filter().set_o2_1(sign_ext<s32>(data, 16));
						break;
					case 6:	 // O1(n-1) (Filter 1 Temp Register)
						v.filter().set_o1_1(sign_ext<s32>(data, 16));
						break;
				}
			}
			else  // Page 0 - 24
			{
				switch (address)
				{
					case 0:	 // CR (Control Register)
						v.alu().set_stop(bitfield(data, 0, 2));
						v.cr().set_bs(bitfield(data, 2));
						v.alu().set_lpe(bitfield(data, 3));
						v.alu().set_ble(bitfield(data, 4));
						v.alu().set_irqe(bitfield(data, 5));
						v.alu().set_dir(bitfield(data, 6));
						v.alu().set_irq(bitfield(data, 7));
						v.cr().set_ca(bitfield(data, 8, 2));
						v.filter().set_lp(bitfield(data, 10, 2));
						break;
					case 1:	 // FC (Frequency Control)
						v.alu().set_fc(bitfield(data, 1, 15));
						break;
					case 2:	 // STRT-H (Loop Start Register High)
						v.alu().set_start(bitfield<u32>(data, 0, 13) << 16, 0x1fff0000);
						break;
					case 3:	 // STRT-L (Loop Start Register Low)
						v.alu().set_start(data & 0xffe0, 0xffe0);
						break;
					case 4:	 // END-H (Loop End Register High)
						v.alu().set_end(bitfield<u32>(data, 0, 13) << 16, 0x1fff0000);
						break;
					case 5:	 // END-L (Loop End Register Low)
						v.alu().set_end(data & 0xffe0, 0xffe0);
						break;
					case 6:	 // K2 (Filter Cutoff Coefficient #2)
						v.filter().set_k2(data & 0xfff0);
						break;
					case 7:	 // K1 (Filter Cutoff Coefficient #1)
						v.filter().set_k1(data & 0xfff0);
						break;
					case 8:	 // LVOL (Left Volume)
						v.set_lvol(bitfield(data, 8, 8));
						break;
					case 9:	 // RVOL (Right Volume)
						v.set_rvol(bitfield(data, 8, 8));
						break;
					case 10:  // ACCH (Accumulator High)
						v.alu().set_accum(bitfield<u32>(data, 0, 13) << 16, 0x1fff0000);
						break;
					case 11:  // ACCL (Accumulator Low)
						v.alu().set_accum(data, 0xffff);
						break;
				}
			}
		}
	}
}
