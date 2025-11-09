/*

============================================================================

NDS sound emulator
by cam900

MODIFIED BY TILDEARROW!!!
MODIFIED BY TILDEARROW!!!
MODIFIED BY TILDEARROW!!!
MODIFIED BY TILDEARROW!!!
MODIFIED BY TILDEARROW!!!
MODIFIED BY TILDEARROW!!!

making it SUPER CLEAR to comply with the license
this is NOT the original version! for the original version, git checkout
any commit from January 2025.

This file is licensed under zlib license.

============================================================================

zlib License

(C) 2024-present cam900 and contributors

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

============================================================================
TODO:
- needs to further verifications from real hardware

Tech info: https://problemkaputt.de/gbatek.htm

*/

#ifndef NDS_SOUND_EMU_H
#define NDS_SOUND_EMU_H

#include <stdlib.h>
#include "blip_buf.h"
#include "../../dispatch.h"

namespace nds_sound_emu
{
	using u8 = unsigned char;
	using u16 = unsigned short;
	using u32 = unsigned int;
	using u64 = unsigned long long;
	using s8 = signed char;
	using s16 = signed short;
	using s32 = signed int;
	using s64 = signed long long;

	template<typename T>
	static const inline T bitfield(const T in, const u8 pos)
	{
		return (in >> pos) & 1;
	} // bitfield

	template<typename T>
	static const inline T bitfield(const T in, const u8 pos, const u8 len)
	{
		return (in >> pos) & ((1 << len) - 1);
	} // bitfield

	template<typename T>
	static const inline T clamp(const T in, const T min, const T max)
	{
		return (in < min) ? min : ((in > max) ? max : in);
	} // clamp

	class nds_sound_intf
	{
		public:
			nds_sound_intf()
			{
			}

			virtual u8 read_byte(u32 addr) { return 0; }
			inline u16 read_word(u32 addr) { return read_byte(addr) | (u16(read_byte(addr + 1)) << 8); }
			inline u32 read_dword(u32 addr) { return read_word(addr) | (u16(read_word(addr + 2)) << 16); }

			virtual void write_byte(u32 addr, u8 data) {}
			inline void write_word(u32 addr, u16 data)
			{
				write_byte(addr, data & 0xff);
				write_byte(addr + 1, data >> 8);
			}
			inline void write_dword(u32 addr, u32 data)
			{
				write_word(addr, data & 0xffff);
				write_word(addr + 2, data >> 16);
			}
	};

	class nds_sound_t
	{
		public:
			nds_sound_t(nds_sound_intf &intf)
				: m_intf(intf)
				, m_channel{
					channel_t(*this, false, false), channel_t(*this, false, false),
					channel_t(*this, false, false), channel_t(*this, false, false), 
					channel_t(*this, false, false), channel_t(*this, false, false),
					channel_t(*this, false, false), channel_t(*this, false, false), 
					channel_t(*this, true, false), channel_t(*this, true, false),
					channel_t(*this, true, false), channel_t(*this, true, false), 
					channel_t(*this, true, false), channel_t(*this, true, false),
					channel_t(*this, false, true), channel_t(*this, false, true)
				}
				, m_capture{
					capture_t(*this, m_channel[0], m_channel[1]),
					capture_t(*this, m_channel[2], m_channel[3])
				}
				, m_control(0)
				, m_bias(0)
				, m_lastts(0)
			{
			}

			void reset();
			void resetTS(u32 what);
			void tick(s32 cycle);
			s32 predict();
			void set_bb(blip_buffer_t* bbLeft, blip_buffer_t* bbRight);
			void set_oscbuf(DivDispatchOscBuffer** oscBuf);

			// host accesses
			u32 read32(u32 addr);
			void write32(u32 addr, u32 data, u32 mask = ~0);

			u16 read16(u32 addr);
			void write16(u32 addr, u16 data, u16 mask = ~0);

			u8 read8(u32 addr);
			void write8(u32 addr, u8 data);

			// for debug
			s32 chan_lout(u8 ch) { return m_channel[ch].loutput(); }
			s32 chan_rout(u8 ch) { return m_channel[ch].routput(); }

		private:
			// ADPCM tables
			s8 adpcm_index_table[8] =
			{
				-1, -1, -1, -1, 2, 4, 6, 8
			};

			u16 adpcm_diff_table[89] =
			{
				0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x0010,
				0x0011, 0x0013, 0x0015, 0x0017, 0x0019, 0x001c, 0x001f, 0x0022, 0x0025,
				0x0029, 0x002d, 0x0032, 0x0037, 0x003c, 0x0042, 0x0049, 0x0050, 0x0058,
				0x0061, 0x006b, 0x0076, 0x0082, 0x008f, 0x009d, 0x00ad, 0x00be, 0x00d1,
				0x00e6, 0x00fd, 0x0117, 0x0133, 0x0151, 0x0173, 0x0198, 0x01c1, 0x01ee,
				0x0220, 0x0256, 0x0292, 0x02d4, 0x031c, 0x036c, 0x03c3, 0x0424, 0x048e,
				0x0502, 0x0583, 0x0610, 0x06ab, 0x0756, 0x0812, 0x08e0, 0x09c3, 0x0abd,
				0x0bd0, 0x0cff, 0x0e4c, 0x0fba, 0x114c, 0x1307, 0x14ee, 0x1706, 0x1954,
				0x1bdc, 0x1ea5, 0x21b6, 0x2515, 0x28ca, 0x2cdf, 0x315b, 0x364b, 0x3bb9,
				0x41b2, 0x4844, 0x4f7e, 0x5771, 0x602f, 0x69ce, 0x7462, 0x7fff
			};

			// structs
			enum
			{
				STATE_ADPCM_LOAD = 0,
				STATE_PRE_LOOP,
				STATE_POST_LOOP
			};

			class channel_t 
			{
				public:
					channel_t(nds_sound_t &host, bool psg, bool noise)
						: m_host(host)

						, m_psg(psg)
						, m_noise(noise)

						, m_bb{NULL,NULL}
						, m_oscBuf(NULL)

						, m_control(0)
						, m_sourceaddr(0)
						, m_freq(0)
						, m_loopstart(0)
						, m_length(0)
						, m_ctl_volume(0)
						, m_ctl_voldiv(0)
						, m_ctl_hold(0)
						, m_ctl_pan(0)
						, m_ctl_duty(0)
						, m_ctl_repeat(0)
						, m_ctl_format(0)
						, m_ctl_busy(0)
						, m_playing(false)
						, m_adpcm_out(0)
						, m_adpcm_index(0)
						, m_prev_adpcm_out(0)
						, m_prev_adpcm_index(0)
						, m_cur_addr(0)
						, m_cur_state(0)
						, m_cur_bitaddr(0)
						, m_delay(0)
						, m_sample(0)
						, m_lfsr(0x7fff)
						, m_lfsr_out(0x7fff)
						, m_counter(0x10000)
						, m_output(0)
						, m_loutput(0)
						, m_routput(0)
						, m_lastts(0)
					{
					}

					void reset();
					void write(u32 offset, u32 data, u32 mask = ~0);

					void update(s32 cycle);
          void setMasterVol(s32 masterVol);
					void set_bb(blip_buffer_t* bbLeft, blip_buffer_t* bbRight) { m_bb[0] = bbLeft; m_bb[1] = bbRight; }
					void set_oscbuf(DivDispatchOscBuffer* oscBuf) { m_oscBuf = oscBuf; }
					void resetTS(u32 what) { m_lastts = what; }
					s32 predict();

					// getters
					// control word
					u32 control() const { return m_control; }
					u32 freq() const    { return m_freq; }

					// outputs
					s32 output() const  { return m_output; }
					s32 loutput() const { return m_loutput; }
					s32 routput() const { return m_routput; }

				private:
					// inline constants
					const u8 m_voldiv_shift[4] = {0, 1, 2, 4};

					// control bits
					s32 volume() const  { return m_ctl_volume; } // global volume
					u32 voldiv() const  { return m_ctl_voldiv; } // volume shift
					bool hold() const   { return m_ctl_hold; } // hold bit
					u32 pan() const     { return m_ctl_pan; } // panning (0...127, 0 = left, 127 = right, 64 = half)
					u32 duty() const    { return m_ctl_duty; } // PSG duty
					u32 repeat() const  { return m_ctl_repeat; } // Repeat mode (Manual, Loop infinitely, One-shot)
					u32 format() const  { return m_ctl_format; } // Sound Format (PCM8, PCM16, ADPCM, PSG/Noise when exists)
					bool busy() const   { return m_ctl_busy; } // Busy flag

					// calculated values
					s32 lvol() const    { return (m_ctl_pan == 0x7f) ? 0 : 128 - m_ctl_pan; } // calculated left volume
					s32 rvol() const    { return (m_ctl_pan == 0x7f) ? 128 : m_ctl_pan; } // calculated right volume

					// calculated address
					u32 addr() const    { return (m_sourceaddr & ~3) + (m_cur_bitaddr >> 3) + (m_cur_state == STATE_POST_LOOP ? ((m_loopstart + m_cur_addr) << 2) : (m_cur_addr << 2)); }

					void keyon();
					void keyoff();
					void fetch();
					void advance();
          void computeVol();

					// interfaces
					nds_sound_t &m_host; // host device

					// configuration
					bool m_psg             = false;   // PSG Enable
					bool m_noise           = false;   // Noise Enable

					// blip_buf
					blip_buffer_t* m_bb[2];
					DivDispatchOscBuffer* m_oscBuf;

					// registers
					u32 m_control    = 0; // Control
					u32 m_sourceaddr = 0; // Source Address
					u16 m_freq       = 0; // Frequency
					u16 m_loopstart  = 0; // Loop Start
					u32 m_length     = 0; // Length

                                        // exploded control
                                        s32 m_ctl_volume  = 0; // Volume (0-6)
                                        u32 m_ctl_voldiv  = 0; // Volume Shift (8-9)
                                        bool m_ctl_hold  = 0; // Hold (15)
                                        u32 m_ctl_pan     = 0; // Panning (16-22)
                                        u32 m_ctl_duty    = 0; // Duty (24-26)
                                        u32 m_ctl_repeat  = 0; // Repeat Mode (27-28)
                                        u32 m_ctl_format  = 0; // Sound Format (29-30)
                                        bool m_ctl_busy  = 0; // Busy Flag (31)

					// internal states
					bool m_playing         = false;   // playing flag
          s32 m_final_volume     = 0;       // calculated volume
          s32 m_master_volume    = 0;       // master volume cache
					s32 m_adpcm_out        = 0;       // current ADPCM sample value
					s32 m_adpcm_index      = 0;       // current ADPCM step
					s32 m_prev_adpcm_out   = 0;       // previous ADPCM sample value
					s32 m_prev_adpcm_index = 0;       // previous ADPCM step
					u32 m_cur_addr         = 0;       // current address
					s32 m_cur_state        = 0;       // current state
					s32 m_cur_bitaddr      = 0;       // bit address
					s32 m_delay            = 0;       // delay
					s16 m_sample           = 0;       // current sample
					u32 m_lfsr             = 0x7fff;  // noise LFSR
					s16 m_lfsr_out         = 0x7fff;  // LFSR output
					s32 m_counter          = 0x10000; // clock counter
					s32 m_output           = 0;       // current output
					s32 m_loutput          = 0;       // current left output
					s32 m_routput          = 0;       // current right output
					u32 m_lastts           = 0;       // running timestamp
			};

			class capture_t
			{
				public:
					capture_t(nds_sound_t &host, channel_t &input, channel_t &output)
						: m_host(host)
						, m_input(input)
						, m_output(output)
						
						, m_control(0)
						, m_dstaddr(0)
						, m_length(0)

						, m_counter(0x10000)
						, m_cur_addr(0)
						, m_cur_waddr(0)
						, m_cur_bitaddr(0)
						, m_enable(0)

						, m_fifo{
							fifo_data_t(), fifo_data_t(), fifo_data_t(), fifo_data_t(),
							fifo_data_t(), fifo_data_t(), fifo_data_t(), fifo_data_t()
						}
						, m_fifo_head(0)
						, m_fifo_tail(0)
						, m_fifo_empty(true)
						, m_fifo_full(false)
					{
					}

					void reset();
					void update(s32 mix, s32 cycle);

					void control_w(u8 data);
					void addrlen_w(u32 offset, u32 data, u32 mask = ~0);

					// getters
					u32 control() const { return m_control; }
					u32 dstaddr() const { return m_dstaddr; }

				private:
					// inline constants
					// control bits
					bool addmode() const    { return bitfield(m_control, 0); } // Add mode (add channel 1/3 output with channel 0/2)
					bool get_source() const { return bitfield(m_control, 1); } // Select source (left or right mixer, channel 0/2)
					bool repeat() const     { return bitfield(m_control, 2); } // repeat flag
					bool format() const     { return bitfield(m_control, 3); } // store format (PCM16, PCM8)
					bool busy() const       { return bitfield(m_control, 7); } // busy flag

					// FIFO offset mask
					u32 fifo_mask() const   { return format() ? 7 : 3; }

					// calculated address
					u32 waddr() const { return (m_dstaddr & ~3) + (m_cur_waddr << 2); }

					void capture_on();
					void capture_off();
					bool fifo_write();

					// interfaces
					nds_sound_t &m_host;      // host device
					channel_t &m_input;       // Input channel
					channel_t &m_output;      // Output channel

					// registers
					u8 m_control  = 0;   // Control
					u32 m_dstaddr = 0;   // Destination Address
					u32 m_length  = 0;   // Buffer Length

					// internal states
					u32 m_counter          = 0x10000; // clock counter
					u32 m_cur_addr         = 0;       // current address
					u32 m_cur_waddr        = 0;       // current write address
					s32 m_cur_bitaddr      = 0;       // bit address
					bool m_enable          = false;   // capture enable

					// FIFO
					class fifo_data_t
					{
						public:
							fifo_data_t()
								: m_data(0)
							{
							}

							void reset()
							{
								m_data = 0;
							}

							// accessors
							void write_byte(const u8 bit, const u8 data)
							{
								u32 input = u32(data) << bit;
								u32 mask = (0xff << bit);
								m_data = (m_data & ~mask) | (input & mask);
							}

							void write_word(const u8 bit, const u16 data)
							{
								u32 input = u32(data) << bit;
								u32 mask = (0xffff << bit);
								m_data = (m_data & ~mask) | (input & mask);
							}

							// getters
							u32 data() const { return m_data; }

						private:
							u32 m_data = 0;
					};

					fifo_data_t m_fifo[8];          // FIFO (8 word, for 16 sample delay)
					u32 m_fifo_head        = 0;     // FIFO head
					u32 m_fifo_tail        = 0;     // FIFO tail
					bool m_fifo_empty      = true;  // FIFO empty flag
					bool m_fifo_full       = false; // FIFO full flag
			};

			nds_sound_intf &m_intf; // memory interface

			channel_t m_channel[16]; // 16 channels
			capture_t m_capture[2]; // 2 capture channels

			inline u8 mvol() const      { return bitfield(m_control, 0, 7); } // master volume
			inline u8 lout_from() const { return bitfield(m_control, 8, 2); } // left output source (mixer, channel 1, channel 3, channel 1+3)
			inline u8 rout_from() const { return bitfield(m_control, 10, 2); } // right output source (mixer, channel 1, channel 3, channel 1+3)
			inline bool mix_ch1() const { return bitfield(m_control, 12); } // mix/bypass channel 1
			inline bool mix_ch3() const { return bitfield(m_control, 13); } // mix/bypass channel 3
			inline bool enable() const  { return bitfield(m_control, 15); } // global enable

			u32 m_control = 0; // global control
			u32 m_bias = 0; // output bias
			u32 m_lastts = 0; // running timestamp
	};
}; // namespace nds_sound_emu

#endif // NDS_SOUND_EMU_H
