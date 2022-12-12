/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Konami VRC VI sound emulation core
*/

#ifndef _VGSOUND_EMU_SRC_VRCVI_HPP
#define _VGSOUND_EMU_SRC_VRCVI_HPP

#pragma once

#include "../core/core.hpp"

using namespace vgsound_emu;

class vrcvi_intf : public vgsound_emu_core
{
	public:
		vrcvi_intf()
			: vgsound_emu_core("vrc_vi_intf")
		{
		}

		virtual void irq_w(bool irq) {}
};

class vrcvi_core : public vgsound_emu_core
{
		friend class vrcvi_intf;

	private:
		// Common ALU for sound channels
		class alu_t : public vgsound_emu_core
		{
			private:
				class divider_t : public vgsound_emu_core
				{
					public:
						divider_t()
							: vgsound_emu_core("vrc_vi_frequency_divider")
							, m_divider(0)
							, m_enable(0)
						{
						}

						void reset()
						{
							m_divider = 0;
							m_enable  = 0;
						}

						void write(const bool msb, const u8 data);

						// getters
						inline u16 divider() const { return m_divider; }

						inline bool enable() const { return m_enable; }

					private:
						u16 m_divider : 12;	 // divider (pitch)
						u16 m_enable  : 1;	 // channel disable flag
				};

			public:
				alu_t(std::string tag, vrcvi_core &host)
					: vgsound_emu_core(tag)
					, m_host(host)
					, m_divider(divider_t())
					, m_out(0)
					, m_counter(0)
					, m_cycle(0)
				{
				}

				virtual void reset();
				virtual bool tick();

				virtual s8 get_output()
				{
					m_out = 0;
					return 0;
				}

				// accessors
				inline void clear_cycle() { m_cycle = 0; }

				// getters
				divider_t &divider() { return m_divider; }

				inline u16 counter() const { return m_counter; }

				inline u8 cycle() const { return m_cycle; }

				// for previwe/debug only
				inline s8 out() const { return m_out; }

			protected:
				vrcvi_core &m_host;
				divider_t m_divider;
				s8 m_out = 0;		 // output per channel
				u16 m_counter : 12;	 // clock counter
				u16 m_cycle	  : 4;	 // clock cycle
		};

		// 2 Pulse channels
		class pulse_t : public alu_t
		{
			private:
				// Control bits
				class pulse_control_t : public vgsound_emu_core
				{
					public:
						pulse_control_t()
							: vgsound_emu_core("vrc_vi_pulse_control")
							, m_mode(0)
							, m_duty(0)
							, m_volume(0)
						{
						}

						void reset()
						{
							m_mode	 = 0;
							m_duty	 = 0;
							m_volume = 0;
						}

						// accessors
						inline void write(const u8 data)
						{
							m_mode	 = bitfield(data, 7, 1);
							m_duty	 = bitfield(data, 4, 3);
							m_volume = bitfield(data, 0, 4);
						}

						// getters
						inline bool mode() const { return m_mode; }

						inline u8 duty() const { return m_duty; }

						inline u8 volume() const { return m_volume; }

					private:
						u8 m_mode	: 1;  // duty toggle flag
						u8 m_duty	: 3;  // 3 bit duty cycle
						u8 m_volume : 4;  // 4 bit volume
				};

			public:
				pulse_t(vrcvi_core &host)
					: alu_t("vrc_vi_pulse", host)
					, m_control(pulse_control_t())
				{
				}

				virtual void reset() override;
				virtual bool tick() override;
				virtual s8 get_output() override;

				// getters
				pulse_control_t &control() { return m_control; }

			private:
				pulse_control_t m_control;
		};

		// 1 Sawtooth channel
		class sawtooth_t : public alu_t
		{
			public:
				sawtooth_t(vrcvi_core &host)
					: alu_t("vrc_vi_sawtooth", host)
					, m_rate(0)
					, m_accum(0)
				{
				}

				virtual void reset() override;
				virtual bool tick() override;
				virtual s8 get_output() override;

				// accessors
				inline void clear_accum() { m_accum = 0; }

				// setters
				inline void set_rate(const u8 rate) { m_rate = rate; }

				// getters
				inline u8 rate() const { return m_rate; }

				inline u8 accum() const { return m_accum; }

			private:
				u8 m_rate  = 0;	 // sawtooth accumulate rate
				u8 m_accum = 0;	 // sawtooth accumulator, high 5 bit is accumulated to output
		};

		// Internal timer
		class timer_t : public vgsound_emu_core
		{
			private:
				// Control bits
				class timer_control_t : public vgsound_emu_core
				{
					public:
						timer_control_t()
							: vgsound_emu_core("vrc_vi_timer_control")
							, m_irq_trigger(0)
							, m_enable_ack(0)
							, m_enable(0)
							, m_sync(0)
						{
						}

						void reset()
						{
							m_irq_trigger = 0;
							m_enable_ack  = 0;
							m_enable	  = 0;
							m_sync		  = 0;
						}

						// accessors
						inline void irq_set(const bool irq) { m_irq_trigger = boolmask<u8>(irq); }

						// setters
						inline void set_enable_ack(const bool enable_ack)
						{
							m_enable_ack = boolmask<u8>(enable_ack);
						}

						inline void set_enable(const bool enable)
						{
							m_enable = boolmask<u8>(enable);
						}

						inline void set_sync(const bool sync) { m_sync = boolmask<u8>(sync); }

						// getters
						inline bool irq_trigger() const { return m_irq_trigger; }

						inline bool enable_ack() const { return m_enable_ack; }

						inline bool enable() const { return m_enable; }

						inline bool sync() const { return m_sync; }

					private:
						u8 m_irq_trigger : 1;
						u8 m_enable_ack	 : 1;
						u8 m_enable		 : 1;
						u8 m_sync		 : 1;
				};

			public:
				timer_t(vrcvi_core &host)
					: vgsound_emu_core("vrc_vi_timer")
					, m_host(host)
					, m_timer_control(timer_control_t())
					, m_prescaler(341)
					, m_counter(0)
					, m_counter_latch(0)
				{
				}

				void reset();
				bool tick();
				void counter_tick();

				// IRQ update
				void update() { m_host.m_intf.irq_w(m_timer_control.irq_trigger()); }

				void irq_set()
				{
					if (!m_timer_control.irq_trigger())
					{
						m_timer_control.irq_set(true);
						update();
					}
				}

				void irq_clear()
				{
					if (m_timer_control.irq_trigger())
					{
						m_timer_control.irq_set(false);
						update();
					}
				}

				// accessors
				void reset_counter()
				{
					m_counter	= m_counter_latch;
					m_prescaler = 341;
				}

				void timer_control_w(const u8 data)
				{
					m_timer_control.set_enable_ack(bitfield(data, 0));
					m_timer_control.set_enable(bitfield(data, 1));
					m_timer_control.set_sync(bitfield(data, 2));
					if (m_timer_control.enable())
					{
						reset_counter();
					}
					irq_clear();
				}

				void irq_ack()
				{
					irq_clear();
					m_timer_control.set_enable(m_timer_control.enable_ack());
				}

				// setters
				inline void set_counter_latch(const u8 counter_latch)
				{
					m_counter_latch = counter_latch;
				}

				// getters
				timer_control_t &timer_control() { return m_timer_control; }

				inline s16 prescaler() const { return m_prescaler; }

				inline u8 counter() const { return m_counter; }

				inline u8 counter_latch() const { return m_counter_latch; }

			private:
				vrcvi_core &m_host;				  // host core
				timer_control_t m_timer_control;  // timer control bits
				s16 m_prescaler	   = 341;		  // prescaler
				u8 m_counter	   = 0;			  // clock counter
				u8 m_counter_latch = 0;			  // clock counter latch
		};

		class global_control_t : public vgsound_emu_core
		{
			public:
				global_control_t()
					: vgsound_emu_core("vrc_vi_global_control")
					, m_halt(0)
					, m_shift(0)
				{
				}

				void reset()
				{
					m_halt	= 0;
					m_shift = 0;
				}

				// accessors
				inline void write(const u8 data)
				{
					m_halt	= bitfield(data, 0);
					m_shift = bitfield(data, 1, 2);
				}

				// getters
				inline bool halt() const { return m_halt; }

				inline u8 shift() const { return m_shift; }

			private:
				u8 m_halt  : 1;	 // halt sound
				u8 m_shift : 2;	 // 4/8 bit right shift
		};

	public:
		// constructor
		vrcvi_core(vrcvi_intf &intf)
			: vgsound_emu_core("vrc_vi")
			, m_intf(intf)
			, m_pulse{*this, *this}
			, m_sawtooth(*this)
			, m_timer(*this)
			, m_control(global_control_t())
			, m_out(0)
		{
		}

		// accessors, getters, setters
		void pulse_w(const u8 voice, const u8 address, const u8 data);
		void saw_w(const u8 address, const u8 data);
		void timer_w(const u8 address, const u8 data);
		void control_w(const u8 data);

		// internal state
		void reset();
		s8 tick();

		template<typename T>
		void tick_stream(const std::size_t stream_len, T *out)
		{
			for (std::size_t s = 0; s < stream_len; s++)
			{
				out[s] = tick();
			}
		}

		// 6 bit output
		inline s8 out() const { return m_out; }

		// for debug/preview only
		inline s8 pulse_out(const u8 pulse) const { return (pulse < 2) ? m_pulse[pulse].out() : 0; }

		inline s8 sawtooth_out() const { return m_sawtooth.out(); }

	protected:
		global_control_t &control() { return m_control; }

	private:
		vrcvi_intf &m_intf;

		std::array<pulse_t, 2> m_pulse;	 // 2 pulse channels
		sawtooth_t m_sawtooth;			 // sawtooth channel
		timer_t m_timer;				 // internal timer
		global_control_t m_control;		 // control

		s8 m_out = 0;  // 6 bit output
};

#endif
