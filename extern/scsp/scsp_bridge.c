/*
 * scsp_bridge.c — C wrapper around the aosdk SCSP (YMF292-F) emulator.
 *
 * Provides a clean C API (scsp_init, scsp_write_slot, scsp_render, ...)
 * for hosts that do not want to deal with the original MAME-style
 * SCSP_0_w/SCSP_Update interface directly.
 *
 * The EMSCRIPTEN_KEEPALIVE marker is a no-op outside of Emscripten, so the
 * same source compiles for native hosts without modification.
 */

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

#include "scsp_types.h"
#include "scsp.h"
#include "scsp_bridge.h"

/* ── Globals provided by this file ──────────────────────────────── */

/* sat_ram is declared extern in scsp_types.h — define it here */
uint8 sat_ram[512 * 1024];

/* stv_scu is referenced by scsp.c */
static UINT32 stv_scu_stub[256];
UINT32 *stv_scu = stv_scu_stub;

/* IRQ callback — no-op for standalone use */
static void dummy_irq_cb(int irq) { (void)irq; }

/* ── SCSP instance ─────────────────────────────────────────────── */

/* scsp.c declares: extern struct _SCSP SCSP; — we provide it */
/* (scsp.c also includes scsplfo.c internally) */

/* Output buffer for rendering */
#define MAX_RENDER_SAMPLES 8192
static int16_t render_buf[MAX_RENDER_SAMPLES * 2]; /* stereo interleaved */

/* ── Exported WASM API ─────────────────────────────────────────── */

EMSCRIPTEN_KEEPALIVE
void scsp_init(void) {
    /* Zero all state */
    memset(&SCSP, 0, sizeof(struct _SCSP));
    memset(sat_ram, 0, sizeof(sat_ram));

    /* Set up the interface */
    struct SCSPinterface intf;
    memset(&intf, 0, sizeof(intf));
    intf.num = 1;
    intf.region[0] = sat_ram;
    intf.mixing_level[0] = 0;
    intf.irq_callback[0] = dummy_irq_cb;

    scsp_start(&intf);

    /* Write to slot 0 register 0 — this was in the original init code.
     * While not MVOL (as originally commented), it may trigger necessary
     * side effects in SCSP_UpdateSlotReg. Restoring for safety. */
    SCSP_0_w(0, 0x000F, 0x0000);
}

EMSCRIPTEN_KEEPALIVE
uint8_t *scsp_get_ram_ptr(void) {
    return sat_ram;
}

EMSCRIPTEN_KEEPALIVE
uint32_t scsp_get_ram_size(void) {
    return sizeof(sat_ram);
}

/*
 * Write a 16-bit value to the SCSP register space.
 * addr: byte address in SCSP register map (0x000 - 0xFFF)
 *   Slots: 0x000-0x3FF (32 slots × 0x20 bytes each)
 *   Global: 0x400+
 */
EMSCRIPTEN_KEEPALIVE
void scsp_write_reg(uint32_t addr, uint16_t value) {
    SCSP_0_w(addr / 2, value, 0x0000);
}

/*
 * Write a slot register word directly.
 * slot: 0-31
 * reg_word: word offset within slot (0x0 - 0xF, maps to data[0x0]..data[0xF])
 * value: 16-bit register value
 */
EMSCRIPTEN_KEEPALIVE
void scsp_write_slot(int slot, int reg_word, uint16_t value) {
    int addr = slot * 0x20 + reg_word * 2;
    SCSP_0_w(addr / 2, value, 0x0000);
}

/*
 * Trigger key-on for a slot.
 * The SCSP requires EG.state == RELEASE before a slot can start.
 * We first key-off (to force RELEASE), then key-on.
 */
EMSCRIPTEN_KEEPALIVE
void scsp_key_on(int slot) {
    int addr = slot * 0x20;
    uint16_t cur = SCSP.Slots[slot].udata.data[0x0];

    /* Step 1: ensure slot is in RELEASE state by writing KEYONB=0 + KEYONEX */
    SCSP_0_w(addr / 2, (cur & ~0x0800) | 0x1000, 0x0000);

    /* Step 2: now set KEYONB=1 + KEYONEX to start the slot */
    cur = SCSP.Slots[slot].udata.data[0x0];
    SCSP_0_w(addr / 2, cur | 0x0800 | 0x1000, 0x0000);
}

/*
 * Trigger key-off for a slot.
 * Clears KEYONB bit, writes KEYONEX to execute release.
 */
EMSCRIPTEN_KEEPALIVE
void scsp_key_off(int slot) {
    int addr = slot * 0x20;
    uint16_t cur = SCSP.Slots[slot].udata.data[0x0];
    uint16_t val = (cur & ~0x0800) | 0x1000;  /* clear KEYONB, set KEYONEX */
    SCSP_0_w(addr / 2, val, 0x0000);
}

/*
 * Render audio samples.
 * Returns pointer to interleaved stereo int16 buffer (L,R,L,R,...).
 * num_samples: number of stereo sample pairs to render (max MAX_RENDER_SAMPLES).
 */
EMSCRIPTEN_KEEPALIVE
int16_t *scsp_render(int num_samples) {
    if (num_samples > MAX_RENDER_SAMPLES) num_samples = MAX_RENDER_SAMPLES;

    for (int i = 0; i < num_samples; i++) {
        stereo_sample_t sample;
        SCSP_Update(NULL, NULL, &sample);
        render_buf[i * 2]     = sample.l;
        render_buf[i * 2 + 1] = sample.r;
    }
    return render_buf;
}

/*
 * Get pointer to the render buffer (for JS to read directly from WASM heap).
 */
EMSCRIPTEN_KEEPALIVE
int16_t *scsp_get_render_buf(void) {
    return render_buf;
}

/* ── DSP program API ─────────────────────────────────────────────── */

/*
 * Internal: set up ring buffer in sound RAM for the given RBL selector.
 *
 * Ring buffer placement: top of 512KB sound RAM.
 * RBP is in units of 4096 words (8KB pages) — the DSP addresses memory
 * as: ADDR += RBP << 12, then indexes DSP->SCSPRAM[ADDR].
 * RBL is the buffer size in words (must be power of 2) — used as wrap
 * mask: ADDR &= RBL - 1.
 */
static void dsp_setup_ringbuf(int rbl) {
    static const uint32_t rbl_words[] = { 0x2000, 0x4000, 0x8000, 0x10000 };
    uint32_t rb_words = rbl_words[rbl & 0x03];
    uint32_t rb_byte_offset = (512 * 1024) - (rb_words * 2);
    uint32_t rb_word_offset = rb_byte_offset / 2;

    SCSP.DSP.RBP = rb_word_offset >> 12;  /* in 4096-word pages */
    SCSP.DSP.RBL = rb_words;

    /* Clear the ring buffer region in sound RAM */
    memset(sat_ram + rb_byte_offset, 0, rb_words * 2);
}

/*
 * Load a DSP program from EXB binary format (1344 bytes).
 *
 * EXB layout:
 *   0x000-0x01F:  name (ignored here)
 *   0x020:        RBL  (ring buffer length: 0=8Kw, 1=16Kw, 2=32Kw, 3=64Kw)
 *   0x040-0x0BF:  COEF (64 × 16-bit big-endian)
 *   0x0C0-0x13F:  MADRS (64 × 16-bit big-endian, first 32 used by DSP)
 *   0x140-0x53F:  MPRO (128 × 64-bit big-endian instructions, stored as 4×16-bit)
 *
 * Ring buffer is placed at the top of sound RAM, below the 512KB ceiling.
 */
EMSCRIPTEN_KEEPALIVE
void scsp_dsp_load_exb(const uint8_t *exb, int size) {
    if (size < 0x540) return;  /* minimum valid EXB size */

    /* ── RBL and ring buffer allocation (also clears the region) ── */
    dsp_setup_ringbuf(exb[0x20] & 0x03);

    /* ── COEF (64 entries, 16-bit signed big-endian) ────────────── */
    for (int i = 0; i < 64; i++) {
        int off = 0x40 + i * 2;
        SCSP.DSP.COEF[i] = (int16_t)((exb[off] << 8) | exb[off + 1]);
    }

    /* ── MADRS (32 entries used by DSP, 16-bit unsigned big-endian) */
    for (int i = 0; i < 32; i++) {
        int off = 0xC0 + i * 2;
        SCSP.DSP.MADRS[i] = (uint16_t)((exb[off] << 8) | exb[off + 1]);
    }

    /* ── MPRO (128 instructions × 4 words, big-endian) ──────────── */
    for (int i = 0; i < 128; i++) {
        int off = 0x140 + i * 8;
        for (int w = 0; w < 4; w++) {
            int woff = off + w * 2;
            SCSP.DSP.MPRO[i * 4 + w] = (uint16_t)((exb[woff] << 8) | exb[woff + 1]);
        }
    }

    /* ── Clear DSP working state ─────────────────────────────────── */
    memset(SCSP.DSP.TEMP, 0, sizeof(SCSP.DSP.TEMP));
    memset(SCSP.DSP.MEMS, 0, sizeof(SCSP.DSP.MEMS));
    memset(SCSP.DSP.MIXS, 0, sizeof(SCSP.DSP.MIXS));
    memset(SCSP.DSP.EFREG, 0, sizeof(SCSP.DSP.EFREG));
    SCSP.DSP.DEC = 0;

    /* ── Find last active MPRO step ──────────────────────────────── */
    SCSP.DSP.LastStep = 0;
    for (int i = 127; i >= 0; i--) {
        if (SCSP.DSP.MPRO[i * 4] || SCSP.DSP.MPRO[i * 4 + 1] ||
            SCSP.DSP.MPRO[i * 4 + 2] || SCSP.DSP.MPRO[i * 4 + 3]) {
            SCSP.DSP.LastStep = i + 1;
            break;
        }
    }

    /* ── Start DSP ───────────────────────────────────────────────── */
    SCSP.DSP.Stopped = (SCSP.DSP.LastStep == 0) ? 1 : 0;
}

/*
 * Load DSP arrays directly (for live coding — avoids EXB serialization).
 * Arrays must be pre-allocated and sized correctly.
 * mpro: 128 × 4 = 512 uint16 values (big-endian instruction words)
 * coef: 64 int16 values
 * madrs: 32 uint16 values
 */
EMSCRIPTEN_KEEPALIVE
void scsp_dsp_load_arrays(const uint16_t *mpro, int mpro_len,
                          const int16_t *coef, int coef_len,
                          const uint16_t *madrs, int madrs_len,
                          int rbl) {
    /* RBL and ring buffer */
    if (rbl < 0) rbl = 0;
    if (rbl > 3) rbl = 3;
    dsp_setup_ringbuf(rbl);

    /* COEF */
    int nc = coef_len < 64 ? coef_len : 64;
    for (int i = 0; i < nc; i++) SCSP.DSP.COEF[i] = coef[i];
    for (int i = nc; i < 64; i++) SCSP.DSP.COEF[i] = 0;

    /* MADRS */
    int nm = madrs_len < 32 ? madrs_len : 32;
    for (int i = 0; i < nm; i++) SCSP.DSP.MADRS[i] = madrs[i];
    for (int i = nm; i < 32; i++) SCSP.DSP.MADRS[i] = 0;

    /* MPRO */
    int nw = mpro_len < 512 ? mpro_len : 512;
    for (int i = 0; i < nw; i++) SCSP.DSP.MPRO[i] = mpro[i];
    for (int i = nw; i < 512; i++) SCSP.DSP.MPRO[i] = 0;

    /* Clear working state */
    memset(SCSP.DSP.TEMP, 0, sizeof(SCSP.DSP.TEMP));
    memset(SCSP.DSP.MEMS, 0, sizeof(SCSP.DSP.MEMS));
    memset(SCSP.DSP.MIXS, 0, sizeof(SCSP.DSP.MIXS));
    memset(SCSP.DSP.EFREG, 0, sizeof(SCSP.DSP.EFREG));
    SCSP.DSP.DEC = 0;

    /* Find last active step */
    SCSP.DSP.LastStep = 0;
    for (int i = 127; i >= 0; i--) {
        if (SCSP.DSP.MPRO[i * 4] || SCSP.DSP.MPRO[i * 4 + 1] ||
            SCSP.DSP.MPRO[i * 4 + 2] || SCSP.DSP.MPRO[i * 4 + 3]) {
            SCSP.DSP.LastStep = i + 1;
            break;
        }
    }
    SCSP.DSP.Stopped = (SCSP.DSP.LastStep == 0) ? 1 : 0;
}

/*
 * Stop/start DSP execution.
 */
EMSCRIPTEN_KEEPALIVE
void scsp_dsp_stop(void) {
    SCSP.DSP.Stopped = 1;
}

EMSCRIPTEN_KEEPALIVE
void scsp_dsp_start(void) {
    SCSP.DSP.Stopped = (SCSP.DSP.LastStep == 0) ? 1 : 0;
}

/*
 * Clear DSP working state (TEMP, MEMS, ring buffer) without reloading
 * the program.  Useful when switching songs or stopping playback to
 * kill lingering reverb tails.
 */
EMSCRIPTEN_KEEPALIVE
void scsp_dsp_clear(void) {
    memset(SCSP.DSP.TEMP, 0, sizeof(SCSP.DSP.TEMP));
    memset(SCSP.DSP.MEMS, 0, sizeof(SCSP.DSP.MEMS));
    memset(SCSP.DSP.MIXS, 0, sizeof(SCSP.DSP.MIXS));
    memset(SCSP.DSP.EFREG, 0, sizeof(SCSP.DSP.EFREG));
    SCSP.DSP.DEC = 0;
    /* Also zero the ring buffer in sound RAM.
     * RBP is in 4096-word pages, so byte offset = RBP * 4096 * 2. */
    uint32_t rb_bytes = SCSP.DSP.RBL * 2;
    uint32_t rb_offset = SCSP.DSP.RBP * 4096 * 2;
    if (rb_offset + rb_bytes <= sizeof(sat_ram)) {
        memset(sat_ram + rb_offset, 0, rb_bytes);
    }
}

/*
 * Set effect send routing on a slot.
 *   slot:  0-31
 *   isel:  0-15  (which MIXS channel to feed)
 *   imxl:  0-7   (input mixing level; 0 = no send, 7 = full)
 *
 * Writes to slot register word 0xA: bits [6:3] = ISEL, [2:0] = IMXL.
 */
EMSCRIPTEN_KEEPALIVE
void scsp_slot_set_effect_send(int slot, int isel, int imxl) {
    if (slot < 0 || slot > 31) return;
    uint16_t val = ((isel & 0xF) << 3) | (imxl & 0x7);
    SCSP.Slots[slot].udata.data[0xA] =
        (SCSP.Slots[slot].udata.data[0xA] & 0xFF80) | val;
}

/*
 * Set effect output level and pan for an EFREG channel.
 * This controls how DSP output gets mixed into the final stereo bus.
 *   slot:   0-15 (EFREG channel — mapped to slot 0-15's EFSDL/EFPAN)
 *   efsdl:  0-7  (effect send destination level; 0 = mute, 7 = full)
 *   efpan:  0-31 (pan; 0x00 = right, 0x0F = center, 0x1F = left)
 */
EMSCRIPTEN_KEEPALIVE
void scsp_slot_set_effect_output(int slot, int efsdl, int efpan) {
    if (slot < 0 || slot > 15) return;
    uint16_t val = ((efsdl & 0x7) << 5) | (efpan & 0x1F);
    SCSP.Slots[slot].udata.data[0xB] =
        (SCSP.Slots[slot].udata.data[0xB] & 0xFF00) | val;
}

/*
 * Read an EFREG value (for UI metering / debugging).
 */
EMSCRIPTEN_KEEPALIVE
int16_t scsp_dsp_get_efreg(int index) {
    if (index < 0 || index > 15) return 0;
    return SCSP.DSP.EFREG[index];
}

/* ── Live parameter tweaking ─────────────────────────────────────── */

/*
 * Set a single DSP coefficient.  Takes effect on the next audio sample —
 * the DSP reads COEF[] every cycle (scspdsp.c line 218:
 *   Y = DSP->COEF[COEF] >> 3).
 *
 * value: 16-bit signed, pre-shifted (i.e. the raw value stored in COEF[],
 *        which the DSP right-shifts by 3 to get 13-bit precision).
 *        The JS assembler already applies the <<3 shift in getArrays().
 */
EMSCRIPTEN_KEEPALIVE
void scsp_dsp_set_coef(int index, int16_t value) {
    if (index < 0 || index > 63) return;
    SCSP.DSP.COEF[index] = value;
}

/*
 * Read a COEF value back (for UI display).
 */
EMSCRIPTEN_KEEPALIVE
int16_t scsp_dsp_get_coef(int index) {
    if (index < 0 || index > 63) return 0;
    return SCSP.DSP.COEF[index];
}

/*
 * Set a single MADRS entry (delay line tap offset in words).
 * Changing MADRS at runtime adjusts delay times.  May cause clicks if
 * the tap jumps discontinuously — for smooth changes, interpolate in JS.
 */
EMSCRIPTEN_KEEPALIVE
void scsp_dsp_set_madrs(int index, uint16_t value) {
    if (index < 0 || index > 31) return;
    SCSP.DSP.MADRS[index] = value;
}

/*
 * Read a MADRS value back (for UI display).
 */
EMSCRIPTEN_KEEPALIVE
uint16_t scsp_dsp_get_madrs(int index) {
    if (index < 0 || index > 31) return 0;
    return SCSP.DSP.MADRS[index];
}

uint16_t scsp_read_slot_reg(int slot, int reg) {
    if (slot < 0 || slot > 31) return 0;
    if (reg < 0 || reg > 15) return 0;
    return SCSP.Slots[slot].udata.data[reg];
}

uint16_t scsp_read_common_reg(int reg) {
    if (reg < 0 || reg > 23) return 0;
    return SCSP.udata.data[reg];
}

uint16_t scsp_dsp_get_mpro(int index) {
    if (index < 0 || index > 511) return 0;
    return SCSP.DSP.MPRO[index];
}

/*
 * Write DISDL/DIPAN (direct output level and pan) to a slot WITHOUT
 * touching EFSDL/EFPAN in the lower byte of register 0xB.
 *
 * On the real Saturn, the sound driver sets EFSDL/EFPAN once from the
 * TON mixer section, and only changes DISDL/DIPAN during note-on.
 * This function matches that behavior — call it during note-on instead
 * of writing the full register 0xB word.
 *
 *   disdl: 0-7  (direct send level; 7 = full, 0 = mute)
 *   dipan: 0-31 (pan; 0x00 = right, 0x0F = center, 0x1F = left)
 */
EMSCRIPTEN_KEEPALIVE
void scsp_slot_set_direct_output(int slot, int disdl, int dipan) {
    if (slot < 0 || slot > 31) return;
    uint16_t upper = ((disdl & 0x7) << 13) | ((dipan & 0x1F) << 8);
    SCSP.Slots[slot].udata.data[0xB] =
        (SCSP.Slots[slot].udata.data[0xB] & 0x00FF) | upper;
}
