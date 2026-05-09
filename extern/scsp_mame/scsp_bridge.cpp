// license:BSD-3-Clause
// copyright-holders:João Felipe Santos
//
// scsp_bridge.cpp — C API shim around the MAME-extracted SCSP, providing
// the same surface as extern/scsp/scsp_bridge.c so platform/scsp.cpp
// (the Furnace DivPlatformSCSP wrapper) can use either backend
// interchangeably via the SCSP_BACKEND CMake option.
//
// Single-instance only: one static scsp_device per process. The aosdk
// bridge has the same constraint (it wraps a global struct), so this
// matches existing behavior.

#include "scsp.h"

extern "C" {
// The C-facing API header lives next to the aosdk backend; both bridges
// implement the same declarations.
#include "../scsp/scsp_bridge.h"
}

#include <cstdint>
#include <cstring>
#include <new>


// ── Single SCSP instance + host-owned sound RAM ───────────────────────────

static scsp_device SCSP;
static uint8_t sat_ram[SCSP_RAM_SIZE];

// Tracks whether dsp_setup_ringbuf() has placed a ring buffer in sat_ram.
// scsp_dsp_clear() consults this before zeroing memory — the chip's reset
// state has RBL=8192/RBP=0 (real hardware behavior), but that doesn't mean
// the host has actually allocated a ring buffer there. Without this guard,
// scsp_dsp_clear() would wipe the first 16 KB of sound RAM, which on
// Furnace's wrapper holds the built-in FM waveforms loaded by
// scsp_load_builtins() right before reloadDSP() runs.
static bool dsp_ringbuf_configured = false;

#define MAX_RENDER_SAMPLES 8192
static int16_t render_buf[MAX_RENDER_SAMPLES * 2]; // stereo interleaved


// ── Direct slot/common register accessors ─────────────────────────────────
//
// The aosdk bridge reads/writes slot.udata.data[] directly (rather than
// going through SCSP_0_w) so callers can patch register state without
// triggering the chip's UpdateSlotReg side effects. The MAME backend
// preserves that behavior — m_Slots and m_udata are public on scsp_device
// for exactly this reason.

static inline uint16_t slot_data(int slot, int word)
{
	return SCSP.m_Slots[slot].udata.data[word];
}

static inline void slot_data_set(int slot, int word, uint16_t value)
{
	SCSP.m_Slots[slot].udata.data[word] = value;
}


// ── Lifecycle ─────────────────────────────────────────────────────────────

extern "C" void scsp_init(void)
{
	SCSP.~scsp_device();
	new (&SCSP) scsp_device();
	memset(sat_ram, 0, sizeof(sat_ram));
	dsp_ringbuf_configured = false;
	SCSP.init(sat_ram, sizeof(sat_ram));
	SCSP.write(0, 0x0000, 0xFFFF);
}

extern "C" uint8_t  *scsp_get_ram_ptr(void)  { return sat_ram; }
extern "C" uint32_t  scsp_get_ram_size(void) { return sizeof(sat_ram); }


// ── Register access ───────────────────────────────────────────────────────

extern "C" void scsp_write_reg(uint32_t addr, uint16_t value)
{
	// addr is a byte address into the SCSP register map; scsp_device::write
	// takes a 16-bit-word index, so divide by 2.
	SCSP.write(addr / 2, value, 0xFFFF);
}

extern "C" void scsp_write_slot(int slot, int reg_word, uint16_t value)
{
	int addr = slot * 0x20 + reg_word * 2;
	SCSP.write(addr / 2, value, 0xFFFF);
}


// ── Key-on / key-off ──────────────────────────────────────────────────────

extern "C" void scsp_key_on(int slot)
{
	if (slot < 0 || slot > 31) return;
	int addr = slot * 0x20;

	// Step 1: KEYONB=0 + KEYONEX → force RELEASE.
	uint16_t cur = slot_data(slot, 0);
	SCSP.write(addr / 2, (cur & ~0x0800) | 0x1000, 0xFFFF);

	// Step 2: KEYONB=1 + KEYONEX → start the slot.
	cur = slot_data(slot, 0);
	SCSP.write(addr / 2, cur | 0x0800 | 0x1000, 0xFFFF);
}

extern "C" void scsp_key_off(int slot)
{
	if (slot < 0 || slot > 31) return;
	int addr = slot * 0x20;
	uint16_t cur = slot_data(slot, 0);
	uint16_t val = (cur & ~0x0800) | 0x1000;  // clear KEYONB, set KEYONEX
	SCSP.write(addr / 2, val, 0xFFFF);
}


// ── Audio rendering ───────────────────────────────────────────────────────

extern "C" int16_t *scsp_render(int num_samples)
{
	if (num_samples > MAX_RENDER_SAMPLES) num_samples = MAX_RENDER_SAMPLES;
	if (num_samples <= 0) return render_buf;
	SCSP.render(render_buf, num_samples);
	return render_buf;
}

extern "C" int16_t *scsp_get_render_buf(void) { return render_buf; }


// ── DSP program API ───────────────────────────────────────────────────────
//
// Both backends store DSP state in the same SCSPDSP layout (COEF[64],
// MADRS[32], MPRO[128*4], RBP/RBL), so the body of these functions is
// near-identical to the aosdk bridge — only the field-access path changes
// (SCSP.DSP.X → SCSP.m_DSP.X).

static void dsp_setup_ringbuf(int rbl)
{
	static const uint32_t rbl_words[] = { 0x2000, 0x4000, 0x8000, 0x10000 };
	uint32_t rb_words = rbl_words[rbl & 0x03];
	uint32_t rb_byte_offset = SCSP_RAM_SIZE - (rb_words * 2);
	uint32_t rb_word_offset = rb_byte_offset / 2;

	SCSP.m_DSP.RBP = rb_word_offset >> 12;  // in 4096-word pages
	SCSP.m_DSP.RBL = rb_words;

	memset(sat_ram + rb_byte_offset, 0, rb_words * 2);
	dsp_ringbuf_configured = true;
}

static void dsp_finalize_program(void)
{
	memset(SCSP.m_DSP.TEMP, 0, sizeof(SCSP.m_DSP.TEMP));
	memset(SCSP.m_DSP.MEMS, 0, sizeof(SCSP.m_DSP.MEMS));
	memset(SCSP.m_DSP.MIXS, 0, sizeof(SCSP.m_DSP.MIXS));
	memset(SCSP.m_DSP.EFREG, 0, sizeof(SCSP.m_DSP.EFREG));
	SCSP.m_DSP.DEC = 0;

	SCSP.m_DSP.LastStep = 0;
	for (int i = 127; i >= 0; i--)
	{
		if (SCSP.m_DSP.MPRO[i * 4] || SCSP.m_DSP.MPRO[i * 4 + 1] ||
		    SCSP.m_DSP.MPRO[i * 4 + 2] || SCSP.m_DSP.MPRO[i * 4 + 3])
		{
			SCSP.m_DSP.LastStep = i + 1;
			break;
		}
	}
	SCSP.m_DSP.Stopped = (SCSP.m_DSP.LastStep == 0);
}

extern "C" void scsp_dsp_load_exb(const uint8_t *exb, int size)
{
	if (size < 0x540) return;

	dsp_setup_ringbuf(exb[0x20] & 0x03);

	for (int i = 0; i < 64; i++)
	{
		int off = 0x40 + i * 2;
		SCSP.m_DSP.COEF[i] = (int16_t)((exb[off] << 8) | exb[off + 1]);
	}
	for (int i = 0; i < 32; i++)
	{
		int off = 0xC0 + i * 2;
		SCSP.m_DSP.MADRS[i] = (uint16_t)((exb[off] << 8) | exb[off + 1]);
	}
	for (int i = 0; i < 128; i++)
	{
		int off = 0x140 + i * 8;
		for (int w = 0; w < 4; w++)
		{
			int woff = off + w * 2;
			SCSP.m_DSP.MPRO[i * 4 + w] = (uint16_t)((exb[woff] << 8) | exb[woff + 1]);
		}
	}
	dsp_finalize_program();
}

extern "C" void scsp_dsp_load_arrays(const uint16_t *mpro, int mpro_len,
                                     const int16_t  *coef, int coef_len,
                                     const uint16_t *madrs, int madrs_len,
                                     int rbl)
{
	if (rbl < 0) rbl = 0;
	if (rbl > 3) rbl = 3;
	dsp_setup_ringbuf(rbl);

	int nc = coef_len  < 64  ? coef_len  : 64;
	for (int i = 0; i < nc; i++)  SCSP.m_DSP.COEF[i]  = coef[i];
	for (int i = nc; i < 64; i++) SCSP.m_DSP.COEF[i]  = 0;

	int nm = madrs_len < 32  ? madrs_len : 32;
	for (int i = 0; i < nm; i++)  SCSP.m_DSP.MADRS[i] = madrs[i];
	for (int i = nm; i < 32; i++) SCSP.m_DSP.MADRS[i] = 0;

	int nw = mpro_len  < 512 ? mpro_len  : 512;
	for (int i = 0; i < nw; i++)   SCSP.m_DSP.MPRO[i] = mpro[i];
	for (int i = nw; i < 512; i++) SCSP.m_DSP.MPRO[i] = 0;

	dsp_finalize_program();
}

extern "C" void scsp_dsp_stop(void)  { SCSP.m_DSP.Stopped = true; }
extern "C" void scsp_dsp_start(void) { SCSP.m_DSP.Stopped = (SCSP.m_DSP.LastStep == 0); }

extern "C" void scsp_dsp_clear(void)
{
	memset(SCSP.m_DSP.TEMP, 0, sizeof(SCSP.m_DSP.TEMP));
	memset(SCSP.m_DSP.MEMS, 0, sizeof(SCSP.m_DSP.MEMS));
	memset(SCSP.m_DSP.MIXS, 0, sizeof(SCSP.m_DSP.MIXS));
	memset(SCSP.m_DSP.EFREG, 0, sizeof(SCSP.m_DSP.EFREG));
	SCSP.m_DSP.DEC = 0;

	// Only zero the ring buffer if a previous DSP program actually placed
	// one (via dsp_setup_ringbuf). Otherwise RBL/RBP reflect the chip's
	// reset state (RBL=8192, RBP=0), which would alias the built-in FM
	// waveform region at the start of sound RAM and silently zero it.
	if (dsp_ringbuf_configured) {
		uint32_t rb_bytes  = SCSP.m_DSP.RBL * 2;
		uint32_t rb_offset = SCSP.m_DSP.RBP * 4096 * 2;
		if (rb_offset + rb_bytes <= sizeof(sat_ram))
			memset(sat_ram + rb_offset, 0, rb_bytes);
	}
}


// ── Slot routing helpers ──────────────────────────────────────────────────

extern "C" void scsp_slot_set_effect_send(int slot, int isel, int imxl)
{
	if (slot < 0 || slot > 31) return;
	uint16_t val = ((isel & 0xF) << 3) | (imxl & 0x7);
	slot_data_set(slot, 0xA, (slot_data(slot, 0xA) & 0xFF80) | val);
}

extern "C" void scsp_slot_set_effect_output(int slot, int efsdl, int efpan)
{
	if (slot < 0 || slot > 15) return;
	uint16_t val = ((efsdl & 0x7) << 5) | (efpan & 0x1F);
	slot_data_set(slot, 0xB, (slot_data(slot, 0xB) & 0xFF00) | val);
}

extern "C" void scsp_slot_set_direct_output(int slot, int disdl, int dipan)
{
	if (slot < 0 || slot > 31) return;
	uint16_t upper = ((disdl & 0x7) << 13) | ((dipan & 0x1F) << 8);
	slot_data_set(slot, 0xB, (slot_data(slot, 0xB) & 0x00FF) | upper);
}


// ── Live DSP tweaking + readback ──────────────────────────────────────────

extern "C" void     scsp_dsp_set_coef(int index, int16_t  value) { if (index >= 0 && index < 64) SCSP.m_DSP.COEF[index]  = value; }
extern "C" int16_t  scsp_dsp_get_coef(int index)                 { return (index >= 0 && index < 64) ? SCSP.m_DSP.COEF[index] : 0; }

extern "C" void     scsp_dsp_set_madrs(int index, uint16_t value){ if (index >= 0 && index < 32) SCSP.m_DSP.MADRS[index] = value; }
extern "C" uint16_t scsp_dsp_get_madrs(int index)                { return (index >= 0 && index < 32) ? SCSP.m_DSP.MADRS[index] : 0; }

extern "C" int16_t  scsp_dsp_get_efreg(int index)                { return (index >= 0 && index < 16) ? SCSP.m_DSP.EFREG[index] : 0; }
extern "C" uint16_t scsp_dsp_get_mpro(int index)                 { return (index >= 0 && index < 512) ? SCSP.m_DSP.MPRO[index] : 0; }


// ── Register debugger snapshots ───────────────────────────────────────────

extern "C" uint16_t scsp_read_slot_reg(int slot, int reg)
{
	if (slot < 0 || slot > 31) return 0;
	if (reg  < 0 || reg  > 15) return 0;
	return SCSP.m_Slots[slot].udata.data[reg];
}

extern "C" uint16_t scsp_read_common_reg(int reg)
{
	if (reg < 0 || reg > 23) return 0;
	return SCSP.m_udata.data[reg];
}
