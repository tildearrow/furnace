/*
 * scsp_bridge.h — Public C API for the aosdk SCSP (YMF292-F) emulator.
 *
 * Use this header when integrating the SCSP core into a host application.
 * The implementation in scsp_bridge.c wraps the lower-level SCSP_0_w /
 * SCSP_Update / SCSP.Slots[] surface from scsp.c and scsp.h.
 *
 * Single-instance only: aosdk's SCSP uses a global `struct _SCSP SCSP`,
 * so only one chip can be active per process.
 */

#ifndef SCSP_BRIDGE_H
#define SCSP_BRIDGE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SCSP_RAM_SIZE (512 * 1024)

/* Initialise the chip and clear sound RAM. Must be called once before any
   other API. */
void scsp_init(void);

/* Sound RAM accessors. RAM is 512 KB; samples are stored as little-endian
   16-bit (or 8-bit when PCM8B is set on a slot). */
uint8_t  *scsp_get_ram_ptr(void);
uint32_t  scsp_get_ram_size(void);

/* Register access. addr is a byte offset in the SCSP register map
   (0x000–0xFFF); slots live at 0x000–0x3FF. */
void scsp_write_reg(uint32_t addr, uint16_t value);

/* Write a single 16-bit slot register word. slot 0..31, reg_word 0..0xF. */
void scsp_write_slot(int slot, int reg_word, uint16_t value);

/* Trigger key-on / key-off for a slot. Internally manages the
   KEYONEX/KEYONB sequence required by the chip. */
void scsp_key_on(int slot);
void scsp_key_off(int slot);

/* Render `num_samples` stereo sample pairs. Returns a pointer to an
   internal interleaved int16 L/R buffer. The buffer is reused on each
   call. Caller must consume before invoking again. */
int16_t *scsp_render(int num_samples);
int16_t *scsp_get_render_buf(void);

/* DSP program API. */
void scsp_dsp_load_exb(const uint8_t *exb, int size);
void scsp_dsp_load_arrays(const uint16_t *mpro, int mpro_len,
                          const int16_t  *coef, int coef_len,
                          const uint16_t *madrs, int madrs_len,
                          int rbl);
void scsp_dsp_stop(void);
void scsp_dsp_start(void);
void scsp_dsp_clear(void);

/* DSP live tweaking and metering. */
void    scsp_dsp_set_coef(int index, int16_t value);
int16_t scsp_dsp_get_coef(int index);
void    scsp_dsp_set_madrs(int index, uint16_t value);
uint16_t scsp_dsp_get_madrs(int index);
int16_t scsp_dsp_get_efreg(int index);

/* Slot routing helpers (do not touch unrelated bits in the same register). */
void scsp_slot_set_effect_send(int slot, int isel, int imxl);
void scsp_slot_set_effect_output(int slot, int efsdl, int efpan);
void scsp_slot_set_direct_output(int slot, int disdl, int dipan);

/* Read accessors for the register debugger. Read-only snapshots of the
 * current chip state — no side effects. Out-of-range indices return 0. */
uint16_t scsp_read_slot_reg(int slot, int reg);    /* slot 0..31, reg 0..15 */
uint16_t scsp_read_common_reg(int reg);             /* reg 0..23 (word index) */
uint16_t scsp_dsp_get_mpro(int index);              /* index 0..511 */

/* Optional per-slot output capture for host-side oscilloscope feeds.
 * When buf is non-NULL, the next scsp_render(N) writes each slot's
 * per-frame contribution to the LEFT direct-mix bus (post-EG, post-pan)
 * into buf with layout:  buf[frame * 32 + slot]  for frame in 0..N-1.
 * The buffer must hold at least N*32 int16_t entries. Modulator slots
 * (DISDL=0) emit zero through this path. Pass NULL to disable.
 * The setting persists across scsp_render() calls.
 *
 * Implemented only by the MAME backend. The aosdk backend's stub does
 * nothing (per-channel oscilloscope was a known TODO there). */
void scsp_set_slot_capture(int16_t *buf);

#ifdef __cplusplus
}
#endif

#endif /* SCSP_BRIDGE_H */
