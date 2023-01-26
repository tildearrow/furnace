/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Ensoniq ES5504/ES5505/ES5506 emulation core
*/

#ifndef _VGSOUND_EMU_SRC_ES550X_HPP
#define _VGSOUND_EMU_SRC_ES550X_HPP

#pragma once

#include "../core/core.hpp"
#include "../core/util/clock_pulse.hpp"

using namespace vgsound_emu;

// ES5504/ES5505/ES5506 interface
class es550x_intf : public vgsound_emu_core
{
	public:
		es550x_intf()
			: vgsound_emu_core("es550x_intf")
		{
		}

		virtual void e_pin(bool state) {}  // E output

		virtual void bclk(bool state) {}  // BCLK output (serial specific)

		virtual void lrclk(bool state) {}  // LRCLK output (serial specific)

		virtual void wclk(bool state) {}  // WCLK output (serial specific)

		virtual void irqb(bool state) {}  // irqb output

		virtual u16 adc_r() { return 0; }  // ADC input

		virtual void adc_w(u16 data) {}	 // ADC output

		virtual s16 read_sample(u8 voice, u8 bank, u32 address) { return 0; }
};

// Shared functions for ES5504/ES5505/ES5506
class es550x_shared_core : public vgsound_emu_core
{
		friend class es550x_intf;  // es550x specific memory interface

	private:
		const u8 m_max_voices = 32;

	protected:
		// Interrupt bits
		class es550x_irq_t : public vgsound_emu_core
		{
			public:
				es550x_irq_t()
					: vgsound_emu_core("es550x_irq")
					, m_voice(0)
					, m_irqb(1)
				{
				}

				void reset()
				{
					m_voice = 0;
					m_irqb	= 1;
				}

				// setter
				void set(u8 index)
				{
					m_irqb	= 0;
					m_voice = index & 0x1f;
				}

				void clear()
				{
					m_irqb	= 1;
					m_voice = 0;
				}

				// getter
				inline bool irqb() { return m_irqb; }

				inline u8 voice() { return m_voice; }

				inline u8 get() { return (m_irqb << 7) | (m_voice & 0x1f); }

				u8 m_voice : 5;
				u8 m_irqb  : 1;
		};

		// Common voice class
		class es550x_voice_t : public vgsound_emu_core
		{
			private:
				// Common control bits
				class es550x_control_t : public vgsound_emu_core
				{
					public:
						es550x_control_t()
							: vgsound_emu_core("es550x_voice_control")
							, m_ca(0)
							, m_adc(0)
							, m_bs(0)
							, m_cmpd(0)
						{
						}

						void reset()
						{
							m_ca   = 0;
							m_adc  = 0;
							m_bs   = 0;
							m_cmpd = 0;
						}

						// setters
						inline void set_ca(u8 ca) { m_ca = ca & 0xf; }

						inline void set_adc(bool adc) { m_adc = adc ? 1 : 0; }

						inline void set_bs(u8 bs) { m_bs = bs & 0x3; }

						inline void set_cmpd(bool cmpd) { m_cmpd = cmpd ? 1 : 0; }

						// getters
						inline u8 ca() { return m_ca; }

						inline bool adc() { return m_adc; }

						inline u8 bs() { return m_bs; }

						inline bool cmpd() { return m_cmpd; }

					protected:
						// Channel assign -
						// 4 bit (16 channel or Bank) for ES5504
						// 2 bit (4 stereo channels) for ES5505
						// 3 bit (6 stereo channels) for ES5506
						u8 m_ca : 4;

						// ES5504 Specific
						u8 m_adc : 1;  // Start ADC

						// ES5505/ES5506 Specific
						u8 m_bs	  : 2;	// Bank bit (1 bit for ES5505, 2 bit for ES5506)
						u8 m_cmpd : 1;	// Use compressed sample format (ES5506)
				};

				// Accumulator
				class es550x_alu_t : public vgsound_emu_core
				{
					public:
						es550x_alu_t(u8 integer, u8 fraction, bool transwave)
							: vgsound_emu_core("es550x_voice_alu")
							, m_integer(integer)
							, m_fraction(fraction)
							, m_total_bits(integer + fraction)
							, m_accum_mask(
								u32(std::min<u64>(~0, u64(u64(1) << u64(integer + fraction)) - 1)))
							, m_transwave(transwave)
							, m_fc(0)
							, m_start(0)
							, m_end(0)
							, m_accum(0)
						{
							m_sample.fill(0);
						}

						// configurations
						const u8 m_integer	   = 21;
						const u8 m_fraction	   = 11;
						const u8 m_total_bits  = 32;
						const u32 m_accum_mask = 0xffffffff;
						const bool m_transwave = true;

						// internal states
						void reset();
						bool tick();

						void loop_exec();
						bool busy();
						s32 interpolation();
						u32 get_accum_integer();

						void irq_exec(es550x_intf &intf, es550x_irq_t &irqv, u8 index);

						void irq_update(es550x_intf &intf, es550x_irq_t &irqv)
						{
							intf.irqb(irqv.irqb() ? false : true);
						}

						// setters
						inline void set_stop0(bool stop0) { m_cr.set_stop0(stop0); }

						inline void set_stop1(bool stop1) { m_cr.set_stop1(stop1); }

						inline void set_lpe(bool lpe) { m_cr.set_lpe(lpe); }

						inline void set_ble(bool ble) { m_cr.set_ble(ble); }

						inline void set_irqe(bool irqe) { m_cr.set_irqe(irqe); }

						inline void set_dir(bool dir) { m_cr.set_dir(dir); }

						inline void set_irq(bool irq) { m_cr.set_irq(irq); }

						inline void set_lei(bool lei) { m_cr.set_lei(lei); }

						inline void set_stop(u8 stop) { m_cr.set_stop(stop); }

						inline void set_loop(u8 loop) { m_cr.set_loop(loop); }

						inline void set_fc(u32 fc) { m_fc = fc; }

						inline void set_start(u32 start, u32 mask = ~0)
						{
							m_start = (m_start & ~mask) | (start & mask);
						}

						inline void set_end(u32 end, u32 mask = ~0)
						{
							m_end = (m_end & ~mask) | (end & mask);
						}

						inline void set_accum(u32 accum, u32 mask = ~0)
						{
							m_accum = (m_accum & ~mask) | (accum & mask);
						}

						inline void set_sample(u8 slot, s32 sample) { m_sample[slot & 1] = sample; }

						// getters
						inline bool stop0() { return m_cr.stop0(); }

						inline bool stop1() { return m_cr.stop1(); }

						inline bool lpe() { return m_cr.lpe(); }

						inline bool ble() { return m_cr.ble(); }

						inline bool irqe() { return m_cr.irqe(); }

						inline bool dir() { return m_cr.dir(); }

						inline bool irq() { return m_cr.irq(); }

						inline bool lei() { return m_cr.lei(); }

						inline u8 stop() { return m_cr.stop(); }

						inline u8 loop() { return m_cr.loop(); }

						inline u32 fc() { return m_fc; }

						inline u32 start() { return m_start; }

						inline u32 end() { return m_end; }

						inline u32 accum() { return m_accum; }

						inline s32 sample(u8 slot) { return m_sample[slot & 1]; }

					private:
						class es550x_alu_cr_t : public vgsound_emu_core
						{
							public:
								es550x_alu_cr_t()
									: vgsound_emu_core("es550x_voice_alu_cr")
									, m_stop0(0)
									, m_stop1(0)
									, m_lpe(0)
									, m_ble(0)
									, m_irqe(0)
									, m_dir(0)
									, m_irq(0)
									, m_lei(0)
								{
								}

								void reset()
								{
									m_stop0 = 0;
									m_stop1 = 0;
									m_lpe	= 0;
									m_ble	= 0;
									m_irqe	= 0;
									m_dir	= 0;
									m_irq	= 0;
									m_lei	= 0;
								}

								// setters
								inline void set_stop0(bool stop0) { m_stop0 = stop0 ? 1 : 0; }

								inline void set_stop1(bool stop1) { m_stop1 = stop1 ? 1 : 0; }

								inline void set_lpe(bool lpe) { m_lpe = lpe ? 1 : 0; }

								inline void set_ble(bool ble) { m_ble = ble ? 1 : 0; }

								inline void set_irqe(bool irqe) { m_irqe = irqe ? 1 : 0; }

								inline void set_dir(bool dir) { m_dir = dir ? 1 : 0; }

								inline void set_irq(bool irq) { m_irq = irq ? 1 : 0; }

								inline void set_lei(bool lei) { m_lei = lei ? 1 : 0; }

								inline void set_stop(u8 stop)
								{
									m_stop0 = (stop >> 0) & 1;
									m_stop1 = (stop >> 1) & 1;
								}

								inline void set_loop(u8 loop)
								{
									m_lpe = (loop >> 0) & 1;
									m_ble = (loop >> 1) & 1;
								}

								// getters
								inline bool stop0() { return m_stop0; }

								inline bool stop1() { return m_stop1; }

								inline bool lpe() { return m_lpe; }

								inline bool ble() { return m_ble; }

								inline bool irqe() { return m_irqe; }

								inline bool dir() { return m_dir; }

								inline bool irq() { return m_irq; }

								inline bool lei() { return m_lei; }

								inline u8 stop() { return (m_stop0 << 0) | (m_stop1 << 1); }

								inline u8 loop() { return (m_lpe << 0) | (m_ble << 1); }

							private:
								u8 m_stop0 : 1;	 // Stop with ALU
								u8 m_stop1 : 1;	 // Stop with processor
								u8 m_lpe   : 1;	 // Loop enable
								u8 m_ble   : 1;	 // Bidirectional loop enable
								u8 m_irqe  : 1;	 // IRQ enable
								u8 m_dir   : 1;	 // Playback direction
								u8 m_irq   : 1;	 // IRQ bit
								u8 m_lei   : 1;	 // Loop end ignore (ES5506 specific)
						};

						es550x_alu_cr_t m_cr;
						// Frequency -
						// 6 integer, 9 fraction for ES5504/ES5505
						// 6 integer, 11 fraction for ES5506
						u32 m_fc	= 0;
						u32 m_start = 0;  // Start register
						u32 m_end	= 0;  // End register

						// Accumulator -
						// 20 integer, 9 fraction for ES5504/ES5505
						// 21 integer, 11 fraction for ES5506
						u32 m_accum = 0;
						// Samples
						std::array<s32, 2> m_sample;
				};

				// Filter
				class es550x_filter_t : public vgsound_emu_core
				{
					public:
						es550x_filter_t()
							: vgsound_emu_core("es550x_voice_filter")
							, m_lp(0)
							, m_k2(0)
							, m_k1(0)
						{
							for (std::array<s32, 2> &elem : m_o)
							{
								std::fill(elem.begin(), elem.end(), 0);
							}
						}

						void reset();
						void tick(s32 in);

						// setters
						inline void set_lp(u8 lp) { m_lp = lp & 3; }

						inline void set_k2(s32 k2) { m_k2 = k2; }

						inline void set_k1(s32 k1) { m_k1 = k1; }

						inline void set_o1_1(s32 o1_1) { m_o[1][0] = o1_1; }

						inline void set_o2_1(s32 o2_1) { m_o[2][0] = o2_1; }

						inline void set_o2_2(s32 o2_2) { m_o[2][1] = o2_2; }

						inline void set_o3_1(s32 o3_1) { m_o[3][0] = o3_1; }

						inline void set_o3_2(s32 o3_2) { m_o[3][1] = o3_2; }

						inline void set_o4_1(s32 o4_1) { m_o[4][0] = o4_1; }

						// getters
						inline u8 lp() { return m_lp; }

						inline s32 k2() { return m_k2; }

						inline s32 k1() { return m_k1; }

						inline s32 o1_1() { return m_o[1][0]; }

						inline s32 o2_1() { return m_o[2][0]; }

						inline s32 o2_2() { return m_o[2][1]; }

						inline s32 o3_1() { return m_o[3][0]; }

						inline s32 o3_2() { return m_o[3][1]; }

						inline s32 o4_1() { return m_o[4][0]; }

					private:
						void lp_exec(s32 coeff, s32 in, s32 out);
						void hp_exec(s32 coeff, s32 in, s32 out);

						// Registers
						u8 m_lp = 0;  // Filter mode
						// Filter coefficient registers
						// 12 bit for filter calculation, 4
						// LSBs are used for fine control of ramp increment for
						// hardware envelope (ES5506)
						s32 m_k2 = 0;  // Filter coefficient 2
						s32 m_k1 = 0;  // Filter coefficient 1

						// Filter storage registers
						std::array<std::array<s32, 2>, 5> m_o;
				};

			public:
				es550x_voice_t(std::string tag, u8 integer, u8 fraction, bool transwave)
					: vgsound_emu_core(tag)
					, m_cr(es550x_control_t())
					, m_alu(integer, fraction, transwave)
					, m_filter(es550x_filter_t())
				{
				}

				// internal state
				virtual void reset();
				virtual void fetch(u8 voice, u8 cycle) = 0;
				virtual void tick(u8 voice)			   = 0;

				void irq_update(es550x_intf &intf, es550x_irq_t &irqv)
				{
					m_alu.irq_update(intf, irqv);
				}

				// Getters
				es550x_control_t &cr() { return m_cr; }

				es550x_alu_t &alu() { return m_alu; }

				es550x_filter_t &filter() { return m_filter; }

			protected:
				es550x_control_t m_cr;
				es550x_alu_t m_alu;
				es550x_filter_t m_filter;
		};

		// Host interfaces
		class host_interface_flag_t : public vgsound_emu_core
		{
			public:
				host_interface_flag_t()
					: vgsound_emu_core("es550x_host_interface_flag")
					, m_host_access(0)
					, m_host_access_strobe(0)
					, m_rw(0)
					, m_rw_strobe(0)
				{
				}

				// Accessors
				void reset()
				{
					m_host_access		 = 0;
					m_host_access_strobe = 0;
					m_rw				 = 0;
					m_rw_strobe			 = 0;
				}

				// Setters
				void set_strobe(bool rw)
				{
					m_rw_strobe			 = rw ? 1 : 0;
					m_host_access_strobe = 1;
				}

				void clear_strobe() { m_host_access_strobe = 0; }

				void clear_host_access() { m_host_access = 0; }

				void update_strobe()
				{
					m_rw		  = m_rw_strobe;
					m_host_access = m_host_access_strobe;
				}

				// Getters
				bool host_access() { return m_host_access; }

				bool rw() { return m_rw; }

			private:
				u8 m_host_access		: 1;  // Host access trigger
				u8 m_host_access_strobe : 1;  // Host access strobe
				u8 m_rw					: 1;  // R/W state
				u8 m_rw_strobe			: 1;  // R/W strobe
		};

	public:
		// internal state
		virtual void reset();

		virtual void tick() {}

		// clock outputs
		inline bool _cas() { return m_cas.current_edge(); }

		inline bool _cas_rising_edge() { return m_cas.rising_edge(); }

		inline bool _cas_falling_edge() { return m_cas.falling_edge(); }

		inline bool e() { return m_e.current_edge(); }

		inline bool e_rising_edge() { return m_e.rising_edge(); }

		inline bool e_falling_edge() { return m_e.falling_edge(); }

		//-----------------------------------------------------------------
		//
		//	for preview/debug purpose only, not for serious emulators
		//
		//-----------------------------------------------------------------

		// voice cycle
		inline u8 voice_cycle() { return m_voice_cycle; }

		// voice update flag
		inline bool voice_update() { return m_voice_update; }

		inline bool voice_end() { return m_voice_end; }

	protected:
		// constructor
		es550x_shared_core(std::string tag, const u8 voice, es550x_intf &intf)
			: vgsound_emu_core(tag)
			, m_max_voices(voice)
			, m_intf(intf)
			, m_host_intf(host_interface_flag_t())
			, m_ha(0)
			, m_hd(0)
			, m_page(0)
			, m_irqv(es550x_irq_t())
			, m_active(m_max_voices - 1)
			, m_voice_cycle(0)
			, m_voice_fetch(0)
			, m_voice_update(0)
			, m_voice_end(0)
			, m_clkin(clock_pulse_t<s8>(1, 0))
			, m_cas(clock_pulse_t<s8>(2, 1))
			, m_e(clock_pulse_t<s8>(4, 0))
		{
		}

		// Constants
		virtual inline u8 max_voices() { return m_max_voices; }

		// Shared registers, functions
		virtual void voice_tick() {}  // voice tick

		es550x_intf &m_intf;				// es550x specific memory interface
		host_interface_flag_t m_host_intf;	// Host interface flag
		u8 m_ha	  = 0;						// Host address (4 bit)
		u16 m_hd  = 0;		  // Host data (16 bit for ES5504/ES5505, 8 bit for ES5506)
		u8 m_page = 0;		  // Page
		es550x_irq_t m_irqv;  // Voice interrupt vector registers

		// Internal states
		u8 m_active = 31;			  // Activated voices
									  // -1, ~25 for ES5504,
									  // ~32 for ES5505/ES5506
		u8 m_voice_cycle	= 0;	  // Voice cycle
		u8 m_voice_fetch	= 0;	  // Voice fetch cycle
		bool m_voice_update = false;  // Voice update flag
		bool m_voice_end	= false;  // End of one voice cycle flag
		clock_pulse_t<s8> m_clkin;	  // CLKIN clock
		clock_pulse_t<s8> m_cas;	  // /CAS clock (CLKIN / 4), falling edge of
									  // CLKIN trigger this clock
		clock_pulse_t<s8> m_e;		  // E clock (CLKIN / 8),
									  // falling edge of CLKIN trigger this clock
};

#endif
