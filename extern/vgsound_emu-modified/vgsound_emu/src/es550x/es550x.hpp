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

				inline void reset()
				{
					m_voice = 0;
					m_irqb	= 1;
				}

				// setter
				inline void set(const u8 index)
				{
					m_irqb	= 0;
					m_voice = bitfield(index, 0, 5);
				}

				inline void clear()
				{
					m_irqb	= 1;
					m_voice = 0;
				}

				// getter
				inline bool irqb() const { return m_irqb; }

				inline u8 voice() const { return m_voice; }

				inline u8 get() const { return (m_irqb << 7) | (m_voice & 0x1f); }

				u8 m_voice : 5;
				u8 m_irqb  : 1;
				u8 m_dummy : 2;
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
						inline void set_ca(const u8 ca) { m_ca = bitfield(ca, 0, 4); }

						inline void set_adc(const bool adc) { m_adc = boolmask<u8>(adc); }

						inline void set_bs(const u8 bs) { m_bs = bitfield(bs, 0, 2); }

						inline void set_cmpd(const bool cmpd) { m_cmpd = boolmask<u8>(cmpd); }

						// getters
						inline u8 ca() const { return m_ca; }

						inline bool adc() const { return m_adc; }

						inline u8 bs() const { return m_bs; }

						inline bool cmpd() const { return m_cmpd; }

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
								inline void set_stop0(const bool stop0)
								{
									m_stop0 = boolmask<u8>(stop0);
								}

								inline void set_stop1(const bool stop1)
								{
									m_stop1 = boolmask<u8>(stop1);
								}

								inline void set_lpe(const bool lpe) { m_lpe = boolmask<u8>(lpe); }

								inline void set_ble(const bool ble) { m_ble = boolmask<u8>(ble); }

								inline void set_irqe(const bool irqe)
								{
									m_irqe = boolmask<u8>(irqe);
								}

								inline void set_dir(const bool dir) { m_dir = boolmask<u8>(dir); }

								inline void set_irq(const bool irq) { m_irq = boolmask<u8>(irq); }

								inline void set_lei(const bool lei) { m_lei = boolmask<u8>(lei); }

								inline void set_stop(const u8 stop)
								{
									m_stop0 = bitfield(stop, 0);
									m_stop1 = bitfield(stop, 1);
								}

								inline void set_loop(const u8 loop)
								{
									m_lpe = bitfield(loop, 0);
									m_ble = bitfield(loop, 1);
								}

								// getters
								inline bool stop0() const { return m_stop0; }

								inline bool stop1() const { return m_stop1; }

								inline bool lpe() const { return m_lpe; }

								inline bool ble() const { return m_ble; }

								inline bool irqe() const { return m_irqe; }

								inline bool dir() const { return m_dir; }

								inline bool irq() const { return m_irq; }

								inline bool lei() const { return m_lei; }

								inline u8 stop() const { return (m_stop0 << 0) | (m_stop1 << 1); }

								inline u8 loop() const { return (m_lpe << 0) | (m_ble << 1); }

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

					public:
						es550x_alu_t(const u8 integer, const u8 fraction, const bool transwave)
							: vgsound_emu_core("es550x_voice_alu")
							, m_integer(integer)
							, m_fraction(fraction)
							, m_total_bits(integer + fraction)
							, m_accum_mask(u32(std::min<u64>(~0, bitmask<u64>(integer + fraction))))
							, m_transwave(transwave)
							, m_fc(0)
							, m_start(0)
							, m_end(0)
							, m_accum(0)
							, m_sample({0})
						{
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

						void irq_exec(es550x_intf &intf, es550x_irq_t &irqv, const u8 index);

						void irq_update(es550x_intf &intf, es550x_irq_t &irqv)
						{
							intf.irqb(irqv.irqb() ? false : true);
						}

						// setters
						inline void set_stop0(const bool stop0) { m_cr.set_stop0(stop0); }

						inline void set_stop1(const bool stop1) { m_cr.set_stop1(stop1); }

						inline void set_lpe(const bool lpe) { m_cr.set_lpe(lpe); }

						inline void set_ble(const bool ble) { m_cr.set_ble(ble); }

						inline void set_irqe(const bool irqe) { m_cr.set_irqe(irqe); }

						inline void set_dir(const bool dir) { m_cr.set_dir(dir); }

						inline void set_irq(const bool irq) { m_cr.set_irq(irq); }

						inline void set_lei(const bool lei) { m_cr.set_lei(lei); }

						inline void set_stop(const u8 stop) { m_cr.set_stop(stop); }

						inline void set_loop(const u8 loop) { m_cr.set_loop(loop); }

						inline void set_fc(const u32 fc) { m_fc = fc; }

						inline void set_start(const u32 start, const u32 mask = ~0)
						{
							merge_data(m_start, start, mask);
						}

						inline void set_end(const u32 end, const u32 mask = ~0)
						{
							merge_data(m_end, end, mask);
						}

						inline void set_accum(const u32 accum, const u32 mask = ~0)
						{
							merge_data(m_accum, accum, mask);
						}

						inline void set_sample(const u8 slot, const s32 sample)
						{
							m_sample[slot & 1] = sample;
						}

						// getters
						inline bool stop0() const { return m_cr.stop0(); }

						inline bool stop1() const { return m_cr.stop1(); }

						inline bool lpe() const { return m_cr.lpe(); }

						inline bool ble() const { return m_cr.ble(); }

						inline bool irqe() const { return m_cr.irqe(); }

						inline bool dir() const { return m_cr.dir(); }

						inline bool irq() const { return m_cr.irq(); }

						inline bool lei() const { return m_cr.lei(); }

						inline u8 stop() const { return m_cr.stop(); }

						inline u8 loop() const { return m_cr.loop(); }

						inline u32 fc() const { return m_fc; }

						inline u32 start() const { return m_start; }

						inline u32 end() const { return m_end; }

						inline u32 accum() const { return m_accum; }

						inline s32 sample(const u8 slot) const { return m_sample[slot & 1]; }

					private:
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
						std::array<s32, 2> m_sample = {0};
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
								elem.fill(0);
							}
						}

						void reset();
						void tick(s32 in);

						// setters
						inline void set_lp(const u8 lp) { m_lp = lp & 3; }

						inline void set_k2(const s32 k2) { m_k2 = k2; }

						inline void set_k1(const s32 k1) { m_k1 = k1; }

						inline void set_o1_1(const s32 o1_1) { m_o[1][0] = o1_1; }

						inline void set_o2_1(const s32 o2_1) { m_o[2][0] = o2_1; }

						inline void set_o2_2(const s32 o2_2) { m_o[2][1] = o2_2; }

						inline void set_o3_1(const s32 o3_1) { m_o[3][0] = o3_1; }

						inline void set_o3_2(const s32 o3_2) { m_o[3][1] = o3_2; }

						inline void set_o4_1(const s32 o4_1) { m_o[4][0] = o4_1; }

						// getters
						inline u8 lp() const { return m_lp; }

						inline s32 k2() const { return m_k2; }

						inline s32 k1() const { return m_k1; }

						inline s32 o1_1() const { return m_o[1][0]; }

						inline s32 o2_1() const { return m_o[2][0]; }

						inline s32 o2_2() const { return m_o[2][1]; }

						inline s32 o3_1() const { return m_o[3][0]; }

						inline s32 o3_2() const { return m_o[3][1]; }

						inline s32 o4_1() const { return m_o[4][0]; }

					private:
						void lp_exec(const s32 coeff, const s32 in, const s32 out);
						void hp_exec(const s32 coeff, const s32 in, const s32 out);

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
				es550x_voice_t(const std::string tag,
							   const u8 integer,
							   const u8 fraction,
							   const bool transwave,
							   std::function<u32(u32)> &cr_r,
							   std::function<void(u32)> &cr_w)
					: vgsound_emu_core(tag)
					, m_cr(es550x_control_t())
					, m_alu(integer, fraction, transwave)
					, m_filter(es550x_filter_t())
					, m_cr_r(cr_r)
					, m_cr_w(cr_w)
				{
				}

				// internal state
				virtual void reset();
				virtual void fetch(const u8 voice, const u8 cycle) = 0;
				virtual void tick(const u8 voice)				   = 0;

				void irq_update(es550x_intf &intf, es550x_irq_t &irqv)
				{
					m_alu.irq_update(intf, irqv);
				}

				// setters
				inline void cr_w(u32 data) { m_cr_w(data); }

				// getters
				es550x_control_t &cr() { return m_cr; }

				es550x_alu_t &alu() { return m_alu; }

				es550x_filter_t &filter() { return m_filter; }

				inline u32 cr_r(u32 ret) { return m_cr_r(ret); }

			protected:
				es550x_control_t m_cr;
				es550x_alu_t m_alu;
				es550x_filter_t m_filter;
				std::function<u32(u32)> &m_cr_r;
				std::function<void(u32)> &m_cr_w;
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
				inline void reset()
				{
					m_host_access		 = 0;
					m_host_access_strobe = 0;
					m_rw				 = 0;
					m_rw_strobe			 = 0;
				}

				// Setters
				inline void set_strobe(const bool rw)
				{
					m_rw_strobe			 = boolmask<u8>(rw);
					m_host_access_strobe = 1;
				}

				inline void clear_strobe() { m_host_access_strobe = 0; }

				inline void clear_host_access() { m_host_access = 0; }

				inline void update_strobe()
				{
					m_rw		  = m_rw_strobe;
					m_host_access = m_host_access_strobe;
				}

				// Getters
				inline bool host_access() const { return m_host_access; }

				inline bool rw() const { return m_rw; }

			private:
				u8 m_host_access		: 1;  // Host access trigger
				u8 m_host_access_strobe : 1;  // Host access strobe
				u8 m_rw					: 1;  // R/W state
				u8 m_rw_strobe			: 1;  // R/W strobe
				u8 m_dummy				: 4;  // Dummy
		};

	public:
		// internal state
		virtual void reset();

		virtual void tick() {}

		// clock outputs
		inline bool _cas() const { return m_cas.current_edge(); }

		inline bool _cas_rising_edge() const { return m_cas.rising_edge(); }

		inline bool _cas_falling_edge() const { return m_cas.falling_edge(); }

		inline bool e() const { return m_e.current_edge(); }

		inline bool e_rising_edge() const { return m_e.rising_edge(); }

		inline bool e_falling_edge() const { return m_e.falling_edge(); }

		//-----------------------------------------------------------------
		//
		//	for preview/debug purpose only, not for serious emulators
		//
		//-----------------------------------------------------------------

		// voice cycle
		inline u8 voice_cycle() const { return m_voice_cycle; }

		// voice update flag
		inline bool voice_update() const { return m_voice_update; }

		inline bool voice_end() const { return m_voice_end; }

	protected:
		// constructor
		es550x_shared_core(const std::string tag, const u8 voice, es550x_intf &intf)
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
		virtual inline u8 max_voices() const { return m_max_voices; }

		// Shared registers, functions
		virtual void voice_tick() {}  // voice tick

		es550x_intf &m_intf;				// es550x specific memory interface
		host_interface_flag_t m_host_intf;	// Host interface flag
		u8 m_ha	  = 0;						// Host address (4 bit)
		u16 m_hd  = 0;		  // Host data (16 bit for ES5504/ES5505, 8 bit for ES5506)
		u8 m_page = 0;		  // Page
		es550x_irq_t m_irqv;  // Voice interrupt vector registers

		// Internal states
		u8 m_active = 31;			// Activated voices
									// -1, ~25 for ES5504,
									// ~32 for ES5505/ES5506
		u8 m_voice_cycle = 0;		// Voice cycle
		u8 m_voice_fetch  : 1;		// Voice fetch cycle
		u8 m_voice_update : 1;		// Voice update flag
		u8 m_voice_end	  : 1;		// End of one voice cycle flag
		u8 m_dummy		  : 5;		// Dummy
		clock_pulse_t<s8> m_clkin;	// CLKIN clock
		clock_pulse_t<s8> m_cas;	// /CAS clock (CLKIN / 4), falling edge of
									// CLKIN trigger this clock
		clock_pulse_t<s8> m_e;		// E clock (CLKIN / 8),
									// falling edge of CLKIN trigger this clock
};

#endif
