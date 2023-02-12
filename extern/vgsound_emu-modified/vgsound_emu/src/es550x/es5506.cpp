/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Ensoniq ES5506 emulation core
*/

#include "es5506.hpp"

// Internal functions

// DO NOT USE THIS ONE!
void es5506_core::tick()
{
}

// less cycle accurate, but less CPU heavy routine
void es5506_core::tick_perf()
{
	// output
	if (((!m_mode.lrclk_en()) && (!m_mode.bclk_en()) && (!m_mode.wclk_en())) && (m_w_st < m_w_end))
	{
		const int output_bits = (20 - (m_w_end - m_w_st));
		if (output_bits < 20)
		{
			for (int c = 0; c < 6; c++)
			{
        m_output[c].m_left=m_ch[c].m_left>>output_bits;
        if (m_output[c].m_left<-0x80000) m_output[c].m_left=-0x80000;
        if (m_output[c].m_left>0x7ffff) m_output[c].m_left=0x7ffff;

        m_output[c].m_right=m_ch[c].m_right>>output_bits;
        if (m_output[c].m_right<-0x80000) m_output[c].m_right=-0x80000;
        if (m_output[c].m_right>0x7ffff) m_output[c].m_right=0x7ffff;
			}
		}
	}
	else
	{
		for (int c = 0; c < 6; c++)
		{
			m_output[c].reset();
		}
	}

	// update
  voice_tick();
}

void es5506_core::voice_tick()
{
  // Refresh output
  m_voice_end	  = true;
  m_voice_cycle = 0;
  for (int i=0; i<6; i++)
  {
    m_ch[i].reset();
  }
  // Update voice
  const int total=VGS_CLAMP(m_active,4,31);
  for (int i=0; i<=total; i++) {
    m_voice[i].tick(i);

    const u8 ca = m_voice[i].cr().ca()&7;
    if (ca < 6)
    {
      m_ch[ca] += m_voice[i].ch();
    }
  }
}

void es5506_core::voice_t::fetch(u8 cycle)
{
	m_alu.set_sample(
	  0,
	  m_host.m_intf.read_sample(m_cr.m_bs,
								(m_alu.get_accum_integer())&((1<<m_alu.m_integer)-1)));
	if (m_cr.cmpd())
	{  // Decompress (Upper 8 bit is used for compressed format)
		m_alu.set_sample(0, decompress((m_alu.sample(0)>>8)&255));
	}

  m_alu.set_sample(
	  1,
	  m_host.m_intf.read_sample(m_cr.m_bs,
								(m_alu.get_accum_integer() + 1)&((1<<m_alu.m_integer)-1)));
	if (m_cr.cmpd())
	{  // Decompress (Upper 8 bit is used for compressed format)
		m_alu.set_sample(1, decompress((m_alu.sample(1)>>8)&255));
	}
}

void es5506_core::voice_t::tick(u8 voice)
{
	// Filter execute

	if (m_alu.busy())
	{
          if ((m_alu.m_last_accum&(~m_alu.m_fraction))!=(m_alu.m_accum&(~m_alu.m_fraction))) fetch(0);
         	m_filter.tick(m_alu.interpolation());
		// Send to output
		m_output[0] = m_mute ? 0 : volume_calc(m_lvol, (short)m_filter.o4_1());
		m_output[1] = m_mute ? 0 : volume_calc(m_rvol, (short)m_filter.o4_1());

		m_ch.set_left(m_output[0]);
		m_ch.set_right(m_output[1]);

		// ALU execute
		if (m_alu.tick())
		{
			m_alu.loop_exec();
		}
	} else {
         	m_filter.tick(m_alu.interpolation());
	        m_output[0] = m_output[1] = 0;
         	m_ch.reset();

        }
	// Envelope
	if (m_ecount != 0)
	{
		// Left and Right volume
		if (bitfield(m_lvramp, 0, 8) != 0)
		{
			m_lvol = VGS_CLAMP(m_lvol + sign_ext<s32>(bitfield(m_lvramp, 0, 8), 8), 0, 0xffff);
		}
		if (bitfield(m_rvramp, 0, 8) != 0)
		{
			m_rvol = VGS_CLAMP(m_rvol + sign_ext<s32>(bitfield(m_rvramp, 0, 8), 8), 0, 0xffff);
		}

		// Filter coeffcient
		if ((m_k1ramp.ramp() != 0) &&
			((m_k1ramp.slow() == 0) || (bitfield(m_filtcount, 0, 3) == 0)))
		{
			m_filter.set_k1(
			  VGS_CLAMP(m_filter.k1() + sign_ext<s32>(m_k1ramp.ramp(), 8), 0, 0xffff));
		}
		if ((m_k2ramp.ramp() != 0) &&
			((m_k2ramp.slow() == 0) || (bitfield(m_filtcount, 0, 3) == 0)))
		{
			m_filter.set_k2(
			  VGS_CLAMP(m_filter.k2() + sign_ext<s32>(m_k2ramp.ramp(), 8), 0, 0xffff));
		}

		m_ecount--;
	}
	m_filtcount = bitfield(m_filtcount + 1, 0, 3);

	// Update IRQ
	//m_alu.irq_exec(m_host.m_intf, m_host.m_irqv, voice);
}

// Compressed format
s16 es5506_core::voice_t::decompress(u8 sample)
{
	u8 exponent = bitfield(sample, 5, 3);
	u8 mantissa = bitfield(sample, 0, 5);
	return (exponent > 0)
		   ? s16(((bitfield(mantissa, 4) ? 0x10 : ~0x1f) | bitfield(mantissa, 0, 4))
				 << (4 + (exponent - 1)))
		   : s16(((bitfield(mantissa, 4) ? ~0xf : 0) | bitfield(mantissa, 0, 4)) << 4);
}

// volume calculation
s32 es5506_core::voice_t::volume_calc(u16 volume, s32 in)
{
	return (in * s32(0x100 | ((volume>>4)&255))) >> (20 - ((volume>>12)&15));
}

void es5506_core::reset()
{
	es550x_shared_core::reset();
	for (auto &elem : m_voice)
	{
		elem.reset();
	}

	m_read_latch  = 0xffffffff;
	m_write_latch = 0xffffffff;
	m_w_st		  = 0;
	m_w_end		  = 0;
	m_lr_end	  = 0;
	m_w_st_curr	  = 0;
	m_w_end_curr  = 0;
	m_mode.reset();
	m_bclk.reset();
	m_lrclk.reset(32);
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

void es5506_core::voice_t::reset()
{
	es550x_shared_core::es550x_voice_t::reset();
	m_lvol	 = 0;
	m_rvol	 = 0;
	m_lvramp = 0;
	m_rvramp = 0;
	m_ecount = 0;
	m_k2ramp.reset();
	m_k1ramp.reset();
	m_filtcount = 0;
	m_ch.reset();
	m_mute		= false;
	m_output[0] = m_output[1] = 0;
}

// Accessors
u8 es5506_core::host_r(u8 address)
{
		m_ha = address;
			m_hd = read(m_ha, true);
	return m_hd;
}

void es5506_core::host_w(u8 address, u8 data)
{
		m_ha = address;
		m_hd = data;
			write(m_ha, m_hd);
}

u8 es5506_core::read(u8 address, bool cpu_access)
{
	const u8 byte  = bitfield(address, 0, 2);  // byte select
	const u8 shift = 24 - (byte << 3);
	if (byte != 0)
	{  // Return already latched register if not highest byte is accessing
		return bitfield(m_read_latch, shift, 8);
	}

	address = bitfield(address, 2, 4);	// 4 bit address for CPU access

	// get read register
	m_read_latch = regs_r(m_page, address, cpu_access);

	return bitfield(m_read_latch, 24, 8);
}

void es5506_core::write(u8 address, u8 data)
{
	const u8 byte  = bitfield(address, 0, 2);  // byte select
	const u8 shift = 24 - (byte << 3);
	address		   = bitfield(address, 2, 4);  // 4 bit address for CPU access

	// Update register latch
	m_write_latch = (m_write_latch & ~(0xff << shift)) | (u32(data) << shift);

	if (byte != 3)
	{  // Wait until lowest byte is writed
		return;
	}

	regs_w(m_page, address, m_write_latch);

	// Reset latch
	m_write_latch = 0;
}

u32 es5506_core::regs_r(u8 page, u8 address, bool cpu_access)
{
	u32 read_latch = 0xffffffff;
	// Global registers
	if (address >= 13)
	{
		switch (address)
		{
			case 13:  // POT (Pot A/D Register)
				read_latch = (read_latch & ~0x3ff) | bitfield(m_intf.adc_r(), 0, 10);
				break;
			case 14:  // IRQV (Interrupting voice vector)
				read_latch = (read_latch & ~0x9f) | (m_irqv.irqb() ? 0x80 : 0) |
							 bitfield(m_irqv.voice(), 0, 5);
				if (cpu_access)
				{
					m_irqv.clear();
					if (bool(bitfield(read_latch, 7)) != m_irqv.irqb())
					{
						m_voice[m_irqv.voice()].irq_update(m_intf, m_irqv);
					}
				}
				break;
			case 15:  // PAGE (Page select register)
				read_latch = (read_latch & ~0x7f) | bitfield(m_page, 0, 7);
				break;
		}
	}
	else
	{
		// Channel registers are Write only
		if (bitfield(page, 6))
		{
			if (!cpu_access)  // CPU can't read here
			{
				switch (address)
				{
					case 0:	  // CH0L (Channel 0 Left)
					case 2:	  // CH1L (Channel 1 Left)
					case 4:	  // CH2L (Channel 2 Left)
					case 6:	  // CH3L (Channel 3 Left)
					case 8:	  // CH4L (Channel 4 Left)
					case 10:  // CH5L (Channel 5 Left)
						read_latch = m_ch[bitfield(address, 1, 3)].left();
						break;
					case 1:	  // CH0R (Channel 0 Right)
					case 3:	  // CH1R (Channel 1 Right)
					case 5:	  // CH2R (Channel 2 Right)
					case 7:	  // CH3R (Channel 3 Right)
					case 9:	  // CH4R (Channel 4 Right)
					case 11:  // CH5R (Channel 5 Right)
						read_latch = m_ch[bitfield(address, 1, 3)].right();
						break;
				}
			}
		}
		else
		{
			const u8 voice = bitfield(page, 0, 5);	// Voice select
			voice_t &v	   = m_voice[voice];
			if (bitfield(page, 5))	// Page 32 - 63
			{
				switch (address)
				{
					case 0:	 // CR (Control Register)
						read_latch = (read_latch & ~0xffff) | (v.alu().stop() << 0) |
									 (v.alu().lei() ? 0x0004 : 0x0000) | (v.alu().loop() << 3) |
									 (v.alu().irqe() ? 0x0020 : 0x0000) |
									 (v.alu().dir() ? 0x0040 : 0x0000) |
									 (v.alu().irq() ? 0x0080 : 0x0000) |
									 (bitfield(v.filter().lp(), 0, 2) << 8) | (v.cr().ca() << 10) |
									 (v.cr().cmpd() ? 0x2000 : 0x0000) | (v.cr().bs() << 14);
						break;
					case 1:	 // START (Loop Start Register)
						read_latch = (read_latch & ~0xfffff800) | (v.alu().start() & 0xfffff800);
						break;
					case 2:	 // END (Loop End Register)
						read_latch = (read_latch & ~0xffffff80) | (v.alu().end() & 0xffffff80);
						break;
					case 3:	 // ACCUM (Accumulator Register)
						read_latch = v.alu().accum();
						break;
					case 4:	 // O4(n-1) (Filter 4 Temp Register)
						if (cpu_access)
						{
							read_latch =
							  (read_latch & ~0x3ffff) | bitfield(v.filter().o4_1(), 0, 18);
						}
						else
						{
							read_latch = v.filter().o4_1();
						}
						break;
					case 5:	 // O3(n-2) (Filter 3 Temp Register #2)
						if (cpu_access)
						{
							read_latch =
							  (read_latch & ~0x3ffff) | bitfield(v.filter().o3_2(), 0, 18);
						}
						else
						{
							read_latch = v.filter().o3_2();
						}
						break;
					case 6:	 // O3(n-1) (Filter 3 Temp Register #1)
						if (cpu_access)
						{
							read_latch =
							  (read_latch & ~0x3ffff) | bitfield(v.filter().o3_1(), 0, 18);
						}
						else
						{
							read_latch = v.filter().o3_1();
						}
						break;
					case 7:	 // O2(n-2) (Filter 2 Temp Register #2)
						if (cpu_access)
						{
							read_latch =
							  (read_latch & ~0x3ffff) | bitfield(v.filter().o2_2(), 0, 18);
						}
						else
						{
							read_latch = v.filter().o2_2();
						}
						break;
					case 8:	 // O2(n-1) (Filter 2 Temp Register #1)
						if (cpu_access)
						{
							read_latch =
							  (read_latch & ~0x3ffff) | bitfield(v.filter().o2_1(), 0, 18);
						}
						else
						{
							read_latch = v.filter().o2_1();
						}
						break;
					case 9:	 // O1(n-1) (Filter 1 Temp Register)
						if (cpu_access)
						{
							read_latch =
							  (read_latch & ~0x3ffff) | bitfield(v.filter().o1_1(), 0, 18);
						}
						else
						{
							read_latch = v.filter().o1_1();
						}
						break;
					case 10:  // W_ST (Word Clock Start Register)
						read_latch = (read_latch & ~0x7f) | bitfield(m_w_st, 0, 7);
						break;
					case 11:  // W_END (Word Clock End Register)
						read_latch = (read_latch & ~0x7f) | bitfield(m_w_end, 0, 7);
						break;
					case 12:  // LR_END (Left/Right Clock End Register)
						read_latch = (read_latch & ~0x7f) | bitfield(m_lr_end, 0, 7);
						break;
				}
			}
			else  // Page 0 - 31
			{
				switch (address)
				{
					case 0:	 // CR (Control Register)
						read_latch = (read_latch & ~0xffff) | (v.alu().stop() << 0) |
									 (v.alu().lei() ? 0x0004 : 0x0000) | (v.alu().loop() << 3) |
									 (v.alu().irqe() ? 0x0020 : 0x0000) |
									 (v.alu().dir() ? 0x0040 : 0x0000) |
									 (v.alu().irq() ? 0x0080 : 0x0000) |
									 (bitfield(v.filter().lp(), 0, 2) << 8) | (v.cr().ca() << 10) |
									 (v.cr().cmpd() ? 0x2000 : 0x0000) | (v.cr().bs() << 14);
						break;
					case 1:	 // FC (Frequency Control)
						read_latch = (read_latch & ~0x1ffff) | bitfield(v.alu().fc(), 0, 17);
						break;
					case 2:	 // LVOL (Left Volume)
						read_latch = (read_latch & ~0xffff) | bitfield(v.lvol(), 0, 16);
						break;
					case 3:	 // LVRAMP (Left Volume Ramp)
						read_latch = (read_latch & ~0xff00) | (bitfield(v.lvramp(), 0, 8) << 8);
						break;
					case 4:	 // RVOL (Right Volume)
						read_latch = (read_latch & ~0xffff) | bitfield(v.rvol(), 0, 16);
						break;
					case 5:	 // RVRAMP (Right Volume Ramp)
						read_latch = (read_latch & ~0xff00) | (bitfield(v.rvramp(), 0, 8) << 8);
						break;
					case 6:	 // ECOUNT (Envelope Counter)
						read_latch = (read_latch & ~0x01ff) | bitfield(v.ecount(), 0, 9);
						break;
					case 7:	 // K2 (Filter Cutoff Coefficient #2)
						read_latch = (read_latch & ~0xffff) | bitfield(v.filter().k2(), 0, 16);
						break;
					case 8:	 // K2RAMP (Filter Cutoff Coefficient #2 Ramp)
						read_latch = (read_latch & ~0xff01) |
									 (bitfield(v.k2ramp().ramp(), 0, 8) << 8) |
									 (v.k2ramp().slow() ? 0x0001 : 0x0000);
						break;
					case 9:	 // K1 (Filter Cutoff Coefficient #1)
						read_latch = (read_latch & ~0xffff) | bitfield(v.filter().k1(), 0, 16);
						break;
					case 10:  // K1RAMP (Filter Cutoff Coefficient #1 Ramp)
						read_latch = (read_latch & ~0xff01) |
									 (bitfield(v.k1ramp().ramp(), 0, 8) << 8) |
									 (v.k1ramp().slow() ? 0x0001 : 0x0000);
						break;
					case 11:  // ACT (Number of voices)
						read_latch = (read_latch & ~0x1f) | bitfield(m_active, 0, 5);
						break;
					case 12:  // MODE (Global Mode)
						read_latch =
						  (read_latch & ~0x1f) | (m_mode.lrclk_en() ? 0x01 : 0x00) |
						  (m_mode.wclk_en() ? 0x02 : 0x00) | (m_mode.bclk_en() ? 0x04 : 0x00) |
						  (m_mode.master() ? 0x08 : 0x00) | (m_mode.dual() ? 0x10 : 0x00);
						break;
				}
			}
		}
	}

	return read_latch;
}

void es5506_core::regs_w(u8 page, u8 address, u32 data)
{
	// Global registers
	if (address >= 13)
	{
		switch (address)
		{
			case 13:  // POT (Pot A/D Register)
				// Read only
				break;
			case 14:  // IRQV (Interrupting voice vector)
				// Read only
				break;
			case 15:  // PAGE (Page select register)
				m_page = bitfield(data, 0, 7);
				break;
		}
	}
	else
	{
		// Channel registers are Write only, and for test purposes
		if (bitfield(page, 6))
		{
			switch (address)
			{
				case 0:	  // CH0L (Channel 0 Left)
				case 2:	  // CH1L (Channel 1 Left)
				case 4:	  // CH2L (Channel 2 Left)
				case 6:	  // CH3L (Channel 3 Left)
				case 8:	  // CH4L (Channel 4 Left)
				case 10:  // CH5L (Channel 5 Left)
					m_ch[bitfield(address, 1, 3)].set_left(
					  sign_ext<s32>(bitfield(data, 0, 23), 23));
					break;
				case 1:	  // CH0R (Channel 0 Right)
				case 3:	  // CH1R (Channel 1 Right)
				case 5:	  // CH2R (Channel 2 Right)
				case 7:	  // CH3R (Channel 3 Right)
				case 9:	  // CH4R (Channel 4 Right)
				case 11:  // CH5R (Channel 5 Right)
					m_ch[bitfield(address, 1, 3)].set_right(
					  sign_ext<s32>(bitfield(data, 0, 23), 23));
					break;
			}
		}
		else
		{
			const u8 voice = bitfield(page, 0, 5);	// Voice select
			voice_t &v	   = m_voice[voice];
			if (bitfield(page, 5))	// Page 32 - 63
			{
				switch (address)
				{
					case 0:	 // CR (Control Register)
						v.alu().set_stop(bitfield(data, 0, 2));
						v.alu().set_lei(bitfield(data, 2));
						v.alu().set_loop(bitfield(data, 3, 2));
						v.alu().set_irqe(bitfield(data, 5));
						v.alu().set_dir(bitfield(data, 6));
						v.alu().set_irq(bitfield(data, 7));
						v.filter().set_lp(bitfield(data, 8, 2));
						v.cr().set_ca(std::min<u8>(5, bitfield(data, 10, 3)));
						v.cr().set_cmpd(bitfield(data, 13));
						v.cr().set_bs(bitfield(data, 14, 2));
						break;
					case 1:	 // START (Loop Start Register)
						v.alu().set_start(data & 0xfffff800);
						break;
					case 2:	 // END (Loop End Register)
						v.alu().set_end(data & 0xffffff80);
						break;
					case 3:	 // ACCUM (Accumulator Register)
						v.alu().set_accum(data);
						break;
					case 4:	 // O4(n-1) (Filter 4 Temp Register)
						v.filter().set_o4_1(sign_ext<s32>(bitfield(data, 0, 18), 18));
						break;
					case 5:	 // O3(n-2) (Filter 3 Temp Register #2)
						v.filter().set_o3_2(sign_ext<s32>(bitfield(data, 0, 18), 18));
						break;
					case 6:	 // O3(n-1) (Filter 3 Temp Register #1)
						v.filter().set_o3_1(sign_ext<s32>(bitfield(data, 0, 18), 18));
						break;
					case 7:	 // O2(n-2) (Filter 2 Temp Register #2)
						v.filter().set_o2_2(sign_ext<s32>(bitfield(data, 0, 18), 18));
						break;
					case 8:	 // O2(n-1) (Filter 2 Temp Register #1)
						v.filter().set_o2_1(sign_ext<s32>(bitfield(data, 0, 18), 18));
						break;
					case 9:	 // O1(n-1) (Filter 1 Temp Register)
						v.filter().set_o1_1(sign_ext<s32>(bitfield(data, 0, 18), 18));
						break;
					case 10:  // W_ST (Word Clock Start Register)
						m_w_st = bitfield(data, 0, 7);
						break;
					case 11:  // W_END (Word Clock End Register)
						m_w_end = bitfield(data, 0, 7);
						break;
					case 12:  // LR_END (Left/Right Clock End Register)
						m_lr_end = bitfield(data, 0, 7);
						m_lrclk.set_width(m_lr_end);
						break;
				}
			}
			else  // Page 0 - 31
			{
				switch (address)
				{
					case 0:	 // CR (Control Register)
						v.alu().set_stop(bitfield(data, 0, 2));
						v.alu().set_lei(bitfield(data, 2));
						v.alu().set_loop(bitfield(data, 3, 2));
						v.alu().set_irqe(bitfield(data, 5));
						v.alu().set_dir(bitfield(data, 6));
						v.alu().set_irq(bitfield(data, 7));
						v.filter().set_lp(bitfield(data, 8, 2));
						v.cr().set_ca(std::min<u8>(5, bitfield(data, 10, 3)));
						v.cr().set_cmpd(bitfield(data, 13));
						v.cr().set_bs(bitfield(data, 14, 2));
						break;
					case 1:	 // FC (Frequency Control)
						v.alu().set_fc(bitfield(data, 0, 17));
						break;
					case 2:	 // LVOL (Left Volume)
						v.set_lvol(bitfield(data, 0, 16));
						break;
					case 3:	 // LVRAMP (Left Volume Ramp)
						v.set_lvramp(bitfield(data, 8, 8));
						break;
					case 4:	 // RVOL (Right Volume)
						v.set_rvol(bitfield(data, 0, 16));
						break;
					case 5:	 // RVRAMP (Right Volume Ramp)
						v.set_rvramp(bitfield(data, 8, 8));
						break;
					case 6:	 // ECOUNT (Envelope Counter)
						v.set_ecount(bitfield(data, 0, 9));
						break;
					case 7:	 // K2 (Filter Cutoff Coefficient #2)
						v.filter().set_k2(bitfield(data, 0, 16));
						break;
					case 8:	 // K2RAMP (Filter Cutoff Coefficient #2 Ramp)
						v.k2ramp().write(data);
						break;
					case 9:	 // K1 (Filter Cutoff Coefficient #1)
						v.filter().set_k1(bitfield(data, 0, 16));
						break;
					case 10:  // K1RAMP (Filter Cutoff Coefficient #1 Ramp)
						v.k1ramp().write(data);
						break;
					case 11:  // ACT (Number of voices)
						m_active = std::max<u8>(4, bitfield(data, 0, 5));
						break;
					case 12:  // MODE (Global Mode)
						m_mode.write(data);
						break;
				}
			}
		}
	}
}
