// license:BSD-3-Clause
// copyright-holders:ElSemi, R. Belmont
/*
    SCSP (YMF292-F) header — freestanding port

    Stripped of MAME framework dependencies (device_t, sound_stream,
    emu_timer, devcb, device_rom_interface) so the chip can be embedded
    in any host that supplies a 256 KB or 512 KB sound RAM buffer and
    pumps samples via render(int16_t*, int).

    Behavioral notes:
      - Host owns sound RAM; pass it once via init(ram, ram_size).
        ram_size must be a power of two (typically 256 KB or 512 KB
        for Saturn cart / Saturn console respectively).
      - Sample rate is clock() / 512. Default clock 22.579200 MHz
        gives the standard ~44.1 kHz output.
      - render() takes a count of stereo sample-pairs and writes
        interleaved int16_t L/R into the caller's buffer.
      - Internal timers (TIMA/B/C) are decremented per sample inside
        render(); the user-supplied IRQ callback fires when one expires.
*/

#ifndef SCSP_MAME_SCSP_H
#define SCSP_MAME_SCSP_H

#pragma once

#include "scspdsp.h"

#include <cstdint>


// Compile-time switch retained from upstream MAME. 0 disables the FM
// pre-fetch delay; the upstream comment notes the driver-recommended
// value of 4 sounds distorted, so it stays at 0.
#define SCSP_FM_DELAY    0


// IRQ-line state values (replacements for MAME's ASSERT_LINE/CLEAR_LINE/HOLD_LINE).
enum scsp_irq_state {
	SCSP_IRQ_CLEAR  = 0,
	SCSP_IRQ_ASSERT = 1,
	SCSP_IRQ_HOLD   = 2,
};

class scsp_device
{
public:
	// Default clock matches Saturn SCSP (22.579200 MHz → 44.1 kHz output).
	scsp_device(uint32_t clock = 22'579'200);

	// Lifecycle. Call init() exactly once before any other operation; it
	// builds the envelope/pan/LFO tables and hooks the host-supplied
	// sound RAM into the DSP. ram must remain valid for the lifetime of
	// this scsp_device. ram_size must be a power of two.
	void init(uint8_t *ram, uint32_t ram_size);
	void set_clock(uint32_t clock);
	uint32_t clock() const { return m_clock; }

	// IRQ callbacks. line is the SCSP-internal interrupt index (decoded
	// via SCILV); state is one of SCSP_IRQ_*. main_irq fires only the
	// SCU-side level (state = 0 or 1).
	void set_irq_cb(void (*cb)(int line, int state)) { m_irq_cb = cb; }
	void set_main_irq_cb(void (*cb)(int state))      { m_main_irq_cb = cb; }

	// Master output gain (0.0 .. 1.0), per channel. Defaults to 1.0/1.0.
	// Applied as a final multiplier in render().
	void set_output_gain(int channel, float gain) { if (channel >= 0 && channel < 2) m_output_gain[channel] = gain; }

	// Render n_frames stereo sample-pairs to out[0..2*n_frames-1] as
	// interleaved L/R int16_t. Internal timers are advanced per frame.
	void render(int16_t *out, int n_frames);

	// SCSP register access. offset is a 16-bit-word index into the SCSP
	// register space (i.e. byte address >> 1), 0x000..0x7FF.
	uint16_t read(uint32_t offset);
	void     write(uint32_t offset, uint16_t data, uint16_t mem_mask = 0xFFFF);

	// MIDI I/O access (used for comms on Model 2/3).
	void     midi_in(uint8_t data);
	uint16_t midi_out_r();
	void     midi_out_w(uint8_t data);

private:
	enum SCSP_STATE { SCSP_ATTACK, SCSP_DECAY1, SCSP_DECAY2, SCSP_RELEASE };

	struct SCSP_EG_t
	{
		int volume; //
		SCSP_STATE state;
		int step;
		//step vals
		int AR;     //Attack
		int D1R;    //Decay1
		int D2R;    //Decay2
		int RR;     //Release

		int DL;     //Decay level
		uint8_t EGHOLD;
		uint8_t LPLINK;
	};

	struct SCSP_LFO_t
	{
		uint16_t phase;
		uint32_t phase_step;
		int *table;
		int *scale;
	};

public:
	struct SCSP_SLOT
	{
		union
		{
			uint16_t data[0x10];  //only 0x1a bytes used
			uint8_t  datab[0x20];
		} udata;

		uint8_t  Backwards;    //the wave is playing backwards
		uint8_t  active;       //this slot is currently playing
		uint32_t cur_addr;     //current play address (24.8)
		uint32_t nxt_addr;     //next play address
		uint32_t step;         //pitch step (24.8)
		SCSP_EG_t  EG;         //Envelope
		SCSP_LFO_t PLFO;       //Phase LFO
		SCSP_LFO_t ALFO;       //Amplitude LFO
		int slot;
		int16_t Prev;          //Previous sample (for interpolation)
	};

	// Public state — the bridge layer (extern/scsp_mame/scsp_bridge.cpp)
	// reads slot/common register snapshots and writes the DSP arrays
	// directly via these members.
	union
	{
		uint16_t data[0x30/2];
		uint8_t  datab[0x30];
	} m_udata;

	SCSP_SLOT m_Slots[32];
	SCSPDSP m_DSP;

private:
	// Per-instance clock + sound RAM.
	uint32_t m_clock;
	uint8_t *m_RAM;       // host-supplied
	uint32_t m_RAMMask;   // (ram_size - 1)

	// IRQ callbacks (nullptr until set).
	void (*m_irq_cb)(int line, int state)  = nullptr;
	void (*m_main_irq_cb)(int state)       = nullptr;

	// Master output gain (defaults set in ctor).
	float m_output_gain[2];

	int16_t m_RINGBUF[128];
	uint8_t m_BUFPTR;
#if SCSP_FM_DELAY
	int16_t m_DELAYBUF[SCSP_FM_DELAY];
	uint8_t m_DELAYPTR;
#endif

	uint32_t m_IrqTimA;
	uint32_t m_IrqTimBC;
	uint32_t m_IrqMidi;

	uint8_t m_MidiOutStack[32];
	uint8_t m_MidiOutW, m_MidiOutR;
	uint8_t m_MidiStack[32];
	uint8_t m_MidiW, m_MidiR;

	int32_t m_EG_TABLE[0x400];

	int m_LPANTABLE[0x10000];
	int m_RPANTABLE[0x10000];

	int m_TimPris[3];
	int m_TimCnt[3];

	// Sample-countdown timers (replaces emu_timer). -1 = inactive.
	// When a timer reaches 0 inside render(), the matching *_cb is fired.
	int m_TimCntdown[3];

	// DMA stuff
	struct
	{
		uint32_t dmea;
		uint16_t drga;
		uint16_t dtlg;
		uint8_t  dgate;
		uint8_t  ddir;
	} m_dma;

	uint16_t m_mcieb;
	uint16_t m_mcipd;

	int m_ARTABLE[64], m_DRTABLE[64];

	int16_t *m_RBUFDST;   //this points to where the sample will be stored in the RingBuf

	//LFO
	int m_PLFO_TRI[256], m_PLFO_SQR[256], m_PLFO_SAW[256], m_PLFO_NOI[256];
	int m_ALFO_TRI[256], m_ALFO_SQR[256], m_ALFO_SAW[256], m_ALFO_NOI[256];
	int m_PSCALES[8][256];
	int m_ASCALES[8][256];

	// Local PRNG (replaces machine().rand()). Linear-congruential, seeded
	// in init() — used by the noise generator and the noise LFO table.
	uint32_t m_rand_state;
	uint32_t scsp_rand();

	void exec_dma();       /*state DMA transfer function*/
	uint8_t DecodeSCI(uint8_t irq);
	void CheckPendingIRQ();
	void MainCheckPendingIRQ(uint16_t irq_type);
	void ResetInterrupts();
	void timerA_cb();
	void timerB_cb();
	void timerC_cb();
	int Get_AR(int base, int R);
	int Get_DR(int base, int R);
	void Compute_EG(SCSP_SLOT *slot);
	int EG_Update(SCSP_SLOT *slot);
	uint32_t Step(SCSP_SLOT *slot);
	void Compute_LFO(SCSP_SLOT *slot);
	void StartSlot(SCSP_SLOT *slot);
	void StopSlot(SCSP_SLOT *slot, int keyoff);
	void init_tables();
	void UpdateSlotReg(int s, int r);
	void UpdateReg(int reg);
	void UpdateSlotRegR(int slot, int reg);
	void UpdateRegR(int reg);
	void w16(uint32_t addr, uint16_t val);
	uint16_t r16(uint32_t addr);
	inline int32_t UpdateSlot(SCSP_SLOT *slot);
	void DoMasterSamples(int16_t *out, int n_frames);

	// Inline RAM accessors (host-supplied buffer; SCSP RAM is BE word-addressed).
	inline uint8_t  ram_read_byte(uint32_t addr) const { return m_RAM[addr & m_RAMMask]; }
	inline uint16_t ram_read_word(uint32_t addr) const
	{
		uint32_t a = addr & m_RAMMask;
		return uint16_t((uint16_t(m_RAM[a]) << 8) | m_RAM[(a + 1) & m_RAMMask]);
	}
	inline void ram_write_word(uint32_t addr, uint16_t val)
	{
		uint32_t a = addr & m_RAMMask;
		m_RAM[a]                    = uint8_t(val >> 8);
		m_RAM[(a + 1) & m_RAMMask]  = uint8_t(val & 0xFF);
	}

	// Schedule timer `idx` to expire after (TimPris[idx] * (255 - tval))
	// samples; tval is the low byte of the timer register.
	void schedule_timer(int idx, int tval);

	//LFO
	void LFO_Init();
	int32_t PLFO_Step(SCSP_LFO_t *LFO);
	int32_t ALFO_Step(SCSP_LFO_t *LFO);
	void LFO_ComputeStep(SCSP_LFO_t *LFO, uint32_t LFOF, uint32_t LFOWS, uint32_t LFOS, int ALFO);
};

#endif // SCSP_MAME_SCSP_H
