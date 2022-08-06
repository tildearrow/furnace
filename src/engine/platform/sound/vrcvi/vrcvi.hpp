/*
	License: BSD-3-Clause
	see https://gitlab.com/cam900/vgsound_emu/-/blob/V1/LICENSE for more details

	Copyright holder(s): cam900
	Modifiers and Contributors for Furnace: cam900, tildearrow
	Konami VRC VI sound emulation core

	See vrcvi.cpp to more infos.
*/

#include <algorithm>
#include <memory>

#ifndef _VGSOUND_EMU_VRCVI_HPP
#define _VGSOUND_EMU_VRCVI_HPP

#pragma once

namespace vrcvi
{
	typedef unsigned char       u8;
	typedef unsigned short     u16;
	typedef unsigned int       u32;
	typedef signed char         s8;
	typedef signed short       s16;

	// get bitfield, bitfield(input, position, len)
	template<typename T> T bitfield(T in, u8 pos, u8 len = 1)
	{
		return (in >> pos) & (len ? (T(1 << len) - 1) : 1);
	}
};

class vrcvi_intf
{
public:
	virtual void irq_w(bool irq) { }
};

using namespace vrcvi;
class vrcvi_core
{
public:
	friend class vrcvi_intf;
	// constructor
	vrcvi_core(vrcvi_intf &intf)
		: m_pulse{*this,*this}
		, m_sawtooth(*this)
		, m_timer(*this)
		, m_intf(intf)
	{
	}
	// accessors, getters, setters
	void pulse_w(u8 voice, u8 address, u8 data);
	void saw_w(u8 address, u8 data);
	void timer_w(u8 address, u8 data);
	void control_w(u8 data);

	// internal state
	void reset();
	void tick();

	// 6 bit output
	s8 out() { return m_out; }
  // channel output
  s16 chan_out(u8 ch) { return m_ch_out[ch]; }
private:
	// Common ALU for sound channels
	struct alu_t
	{
		alu_t(vrcvi_core &host)
			: m_host(host)
		{ };


		virtual void reset();
		virtual bool tick();

		struct divider_t
		{
			divider_t()
				: m_divider(0)
				, m_enable(0)
			{ };

			void reset()
			{
				m_divider = 0;
				m_enable = 0;
			}

			void write(bool msb, u8 data);

			u16 m_divider : 12; // divider (pitch)
			u16 m_enable  : 1;   // channel enable flag
		};

		vrcvi_core &m_host;
		divider_t m_divider;
		u16 m_counter = 0; // clock counter
		u8 m_cycle = 0;    // clock cycle
	};

	// 2 Pulse channels
	struct pulse_t : alu_t
	{
		pulse_t(vrcvi_core &host)
			: alu_t(host)
		{ };

		virtual void reset() override;
		virtual bool tick() override;

		// Control bits
		struct pulse_control_t
		{
			pulse_control_t()
				: m_mode(0)
				, m_duty(0)
				, m_volume(0)
			{ };

			void reset()
			{
				m_mode = 0;
				m_duty = 0;
				m_volume = 0;
			}

			u8 m_mode   : 1; // duty toggle flag
			u8 m_duty   : 3; // 3 bit duty cycle
			u8 m_volume : 4; // 4 bit volume
		};

		pulse_control_t m_control;
	};

	// 1 Sawtooth channel
	struct sawtooth_t : alu_t
	{
		sawtooth_t(vrcvi_core &host)
			: alu_t(host)
		{ };

		virtual void reset() override;
		virtual bool tick() override;

		u8 m_rate = 0;  // sawtooth accumulate rate
		u8 m_accum = 0; // sawtooth accumulator, high 5 bit is accumulated to output
	};

	// Internal timer
	struct timer_t
	{
		timer_t(vrcvi_core &host)
			: m_host(host)
		{ };

		void reset();
		bool tick();
		void counter_tick();

		// IRQ update
		void update() { m_host.m_intf.irq_w(m_timer_control.m_irq_trigger); }
		void irq_set()
		{
			if (!m_timer_control.m_irq_trigger)
			{
				m_timer_control.m_irq_trigger = 1;
				update();
			}
		}
		void irq_clear()
		{
			if (m_timer_control.m_irq_trigger)
			{
				m_timer_control.m_irq_trigger = 0;
				update();
			}
		}

		// Control bits
		struct timer_control_t
		{
			timer_control_t()
				: m_irq_trigger(0)
				, m_enable_ack(0)
				, m_enable(0)
				, m_sync(0)
			{ };

			void reset()
			{
				m_irq_trigger = 0;
				m_enable_ack = 0;
				m_enable = 0;
				m_sync = 0;
			}

			u8 m_irq_trigger : 1;
			u8 m_enable_ack : 1;
			u8 m_enable : 1;
			u8 m_sync : 1;
		};

		vrcvi_core &m_host;              // host core
		timer_control_t m_timer_control; // timer control bits
		s16 m_prescaler = 341;           // prescaler
		u8 m_counter = 0;                // clock counter
		u8 m_counter_latch = 0;          // clock counter latch
	};

	struct global_control_t
	{
		global_control_t()
			: m_halt(0)
			, m_shift(0)
		{ };

		void reset()
		{
			m_halt = 0;
			m_shift = 0;
		}

		u8 m_halt  : 1; // halt sound
		u8 m_shift : 2; // 4/8 bit right shift
	};

	pulse_t m_pulse[2];         // 2 pulse channels
	sawtooth_t m_sawtooth;      // sawtooth channel
	timer_t m_timer;            // internal timer
	global_control_t m_control; // control

	vrcvi_intf &m_intf;

	s8 m_out = 0; // 6 bit output
  s8 m_ch_out[3] = {0}; // per-channel output
};

#endif
