/*
	License: BSD-3-Clause
	see https://github.com/cam900/vgsound_emu/blob/vgsound_emu_v1/LICENSE for more details

	Copyright holder(s): cam900
	Modifiers and Contributors for Furnace: cam900
	Ensoniq ES5504/ES5505/ES5506 emulation core

	See es550x.cpp for more info
*/

#ifndef _VGSOUND_EMU_ES550X_HPP
#define _VGSOUND_EMU_ES550X_HPP

#pragma once

#include <algorithm>
#include <memory>
#include "util.hpp"

// ES5504/ES5505/ES5506 interface
class es550x_intf
{
public:
	virtual void e_pin(bool state) {}     // E output
	virtual void bclk(bool state) {}  // BCLK output (serial specific)
	virtual void lrclk(bool state) {} // LRCLK output (serial specific)
	virtual void wclk(bool state) {}  // WCLK output (serial specific)

	virtual void irqb(bool state) {} // IRQB output
	virtual u16 adc_r() { return 0; } // ADC input
	virtual void adc_w(u16 data) {} // ADC output
	virtual s16 read_sample(u8 voice, u8 bank, u32 address) { return 0; }
};

// Shared functions for ES5504/ES5505/ES5506
class es550x_shared_core
{
	friend class es550x_intf; // es550x specific memory interface
public:
	// constructor
	es550x_shared_core(es550x_intf &intf)
		: m_intf(intf)
	{ };

	// internal state
	virtual void reset();
	virtual void tick() {}

	// clock outputs
	bool _cas() { return m_cas.current_edge(); }
	bool _cas_rising_edge() { return m_cas.rising_edge(); }
	bool _cas_falling_edge() { return m_cas.falling_edge(); }

	bool e() { return m_e.current_edge(); }
	bool e_rising_edge() { return m_e.rising_edge(); }
	bool e_falling_edge() { return m_e.falling_edge(); }

//-----------------------------------------------------------------
//
//	for preview/debug purpose only, not for serious emulators
//
//-----------------------------------------------------------------

	// voice cycle
	u8 voice_cycle() { return m_voice_cycle; }

	// voice update flag
	bool voice_update() { return m_voice_update; }
	bool voice_end() { return m_voice_end; }

protected:
	// Constants
	virtual inline u8 max_voices() { return 32; }

	// Shared registers, functions
	virtual void voice_tick() {} // voice tick

	// Interrupt bits
	struct es550x_irq_t
	{
		es550x_irq_t()
			: voice(0)
			, irqb(1)
		{ };

		void reset()
		{
			voice = 0;
			irqb = 1;
		}

		void set(u8 index)
		{
			irqb = 0;
			voice = index;
		}

		void clear()
		{
			irqb = 1;
			voice = 0;
		}

		u8 voice : 5;
		u8 irqb : 1;
	};

	// Common control bits
	struct es550x_control_t
	{
		es550x_control_t()
			: ca(0)
			, adc(0)
			, bs(0)
			, cmpd(0)
		{ };

		void reset()
		{
			ca = 0;
			adc = 0;
			bs = 0;
			cmpd = 0;
		}

		u8 ca    : 4; // Channel assign (4 bit (16 channel or Bank) for ES5504, 2 bit (4 stereo channels) for ES5505, 3 bit (6 stereo channels) for ES5506)
		// ES5504 Specific
		u8 adc   : 1; // Start ADC
		// ES5505/ES5506 Specific
		u8 bs    : 2; // Bank bit (1 bit for ES5505, 2 bit for ES5506)
		u8 cmpd  : 1; // Use compressed sample format
	};

	// Accumulator
	struct es550x_alu_t
	{
		es550x_alu_t(u8 integer, u8 fraction, bool transwave)
			: m_integer(integer)
			, m_fraction(fraction)
			, m_total_bits(integer + fraction)
			, m_accum_mask(u32(std::min<u64>(~0, u64(u64(1) << u64(integer + fraction)) - 1)))
			, m_transwave(transwave)
		{}

		const u8 m_integer;
		const u8 m_fraction;
		const u8 m_total_bits;
		const u32 m_accum_mask;
		const bool m_transwave;

		void reset();
		bool busy();
		bool tick();
		void loop_exec();
		s32 interpolation();
		u32 get_accum_integer();
		void irq_exec(es550x_intf &intf, es550x_irq_t &irqv, u8 index);
		void irq_update(es550x_intf &intf, es550x_irq_t &irqv) { intf.irqb(irqv.irqb ? false : true); }

		struct es550x_alu_cr_t
		{
			es550x_alu_cr_t()
				: stop0(0)
				, stop1(0)
				, lpe(0)
				, ble(0)
				, irqe(0)
				, dir(0)
				, irq(0)
				, lei(0)
			{ };

			void reset()
			{
				stop0 = 0;
				stop1 = 0;
				lpe = 0;
				ble = 0;
				irqe = 0;
				dir = 0;
				irq = 0;
				lei = 0;
			}

			u8 stop0 : 1; // Stop with ALU
			u8 stop1 : 1; // Stop with processor
			u8 lpe   : 1; // Loop enable
			u8 ble   : 1; // Bidirectional loop enable
			u8 irqe  : 1; // IRQ enable
			u8 dir   : 1; // Playback direction
			u8 irq   : 1; // IRQ bit
			u8 lei   : 1; // Loop end ignore (ES5506 specific)
		};

		es550x_alu_cr_t m_cr;
		u32 m_fc        = 0;   // Frequency - 6 integer, 9 fraction for ES5506/ES5505, 6 integer, 11 fraction for ES5506
		u32 m_start     = 0;   // Start register
		u32 m_end       = 0;   // End register
		u32 m_accum     = 0;   // Accumulator - 20 integer, 9 fraction for ES5506/ES5505, 21 integer, 11 fraction for ES5506
		s32 m_sample[2] = {0}; // Samples
	};

	// Filter
	struct es550x_filter_t
	{
		void reset();
		void tick(s32 in);
		s32 lp_exec(s32 coeff, s32 in, s32 prev_out);
		s32 hp_exec(s32 coeff, s32 in, s32 prev_out, s32 prev_in);

		// Registers
		u8 m_lp = 0; // Filter mode
		// Filter coefficient registers
		s32 m_k2 = 0; // Filter coefficient 2 - 12 bit for filter calculation, 4 LSBs are used for fine control of ramp increment for hardware envelope (ES5506)
		s32 m_k1 = 0; // Filter coefficient 1
		// Filter storage registers
		s32 m_o1_1 = 0; // First stage
		s32 m_o2_1 = 0; // Second stage
		s32 m_o2_2 = 0; // Second stage HP
		s32 m_o3_1 = 0; // Third stage
		s32 m_o3_2 = 0; // Third stage HP
		s32 m_o4_1 = 0; // Final stage
	};

	// Common voice struct
	struct es550x_voice_t
	{
		es550x_voice_t(u8 integer, u8 fraction, bool transwave)
			: m_alu(integer, fraction, transwave)
		{}

		// internal state
		virtual void reset();
		virtual void fetch(u8 voice, u8 cycle) = 0;
		virtual void tick(u8 voice) = 0;

		es550x_control_t m_cr;
		es550x_alu_t m_alu;
		es550x_filter_t m_filter;
	};


	// Host interfaces
	struct host_interface_flag_t
	{
		host_interface_flag_t()
			: m_host_access(0)
			, m_host_access_strobe(0)
			, m_rw(0)
			, m_rw_strobe(0)
		{}

		void reset()
		{
			m_host_access = 0;
			m_host_access_strobe = 0;
			m_rw = 0;
			m_rw_strobe = 0;
		}

		u8 m_host_access        : 1; // Host access trigger
		u8 m_host_access_strobe : 1; // Host access strobe
		u8 m_rw                 : 1; // R/W state
		u8 m_rw_strobe          : 1; // R/W strobe
	};
	host_interface_flag_t m_host_intf; // Host interface flag
	u8 m_ha = 0;                       // Host address (4 bit)
	u16 m_hd = 0;                      // Host data (16 bit for ES5504/ES5505, 8 bit for ES5506)
	u8 m_page = 0;                     // Page
	es550x_irq_t m_irqv;               // Voice interrupt vector registers
	// Internal states
	u8 m_active = max_voices() - 1;    // Activated voices (-1, ~25 for ES5504, ~32 for ES5505/ES5506)
	u8 m_voice_cycle = 0;              // Voice cycle
	u8 m_voice_fetch = 0;              // Voice fetch cycle
	bool m_voice_update = false;       // Voice update flag
	bool m_voice_end = false;          // End of one voice cycle flag
	es550x_intf &m_intf;               // es550x specific memory interface
	clock_pulse_t<s8, 1, 0> m_clkin;      // CLKIN clock
	clock_pulse_t<s8, 2, 1> m_cas;        // /CAS clock (CLKIN / 4), falling edge of CLKIN trigger this clock
	clock_pulse_t<s8, 4, 0> m_e;          // E clock (CLKIN / 8), falling edge of CLKIN trigger this clock
};

#endif
