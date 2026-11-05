
/* Nuked OPL3
 *
 * Copyright (C) 2013-2020 Nuke.YKT
 * Copyright (C) 2026 Tony Gies (Nuked-OPL3-fast modifications)
 *
 * This file is part of Nuked OPL3.
 *
 * Nuked OPL3 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 2.1
 * of the License, or (at your option) any later version.
 *
 * Nuked OPL3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Nuked OPL3. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Nuked OPL3 emulator.
 *  Thanks:
 *      MAME Development Team(Jarek Burczynski, Tatsuyuki Satoh):
 *          Feedback and Rhythm part calculation information.
 *      forums.submarine.org.uk(carbon14, opl3):
 *          Tremolo and phase generator calculation information.
 *      OPLx decapsulated(Matthew Gambrell, Olli Niemitalo):
 *          OPL2 ROMs.
 *      siliconpr0n.org(John McMaster, digshadow):
 *          YMF262 and VRC VII decaps and die shots.
 *
 * Upstream version: 1.8 (commit cfedb09)
 * Fork version:    1.8-fast.3
 * Fork home:       https://github.com/tgies/Nuked-OPL3-fast
 *
 * Nuked-OPL3-fast is a bit-exact performance-optimized fork of Nuked-OPL3.
 * Audio output is identical to upstream for the same register stream.
 *
 * Modifications vs. upstream visible in this header:
 *
 *   - Added cached fields to opl3_slot: eg_tl_ksl, eg_ks, pg_inc,
 *     pg_inc_vib[8], eg_rate_hi[4], eg_rate_lo[4], slot_num.
 *   - Added out_cnt to opl3_channel for mix-loop active-slot tracking, and
 *     out_left[4]/out_right[4] mix pointer lists (under
 *     OPL_QUIRK_CHANNELSAMPLEDELAY).
 *   - Reordered opl3_slot to put hot per-sample fields first; struct size
 *     shrank from 96 to 88 bytes.
 *   - Removed unused legacy fields (eg_inc, eg_rate) from opl3_slot.
 *   - Added dormant_gen to opl3_slot and write_gen to opl3_chip (dormant
 *     slot skip), plus mix_left/mix_right/nmix_left/nmix_right/mix_dirty
 *     to opl3_chip (mix-eligibility lists).
 *   - Added the OPL_WF_TABLE_RUNTIME build option (runtime-built waveform
 *     table in place of wf_rom.h).
 *   - Added the OPL_COMPAT_OLD_EG and OPL_COMPAT_DEFERRED_4OP_ALG build
 *     options (parity with older upstream commits).
 *
 * Furnace-specific additions on top of Nuked-OPL3-fast (see MODIFIED.md):
 *
 *   - `muted` field on opl3_channel for tracker-side channel muting
 *     (see https://github.com/tildearrow/furnace/issues/414).
 *   - OPL3_Resample(chip, samplerate) to change output rate without
 *     resetting chip state.
 */

#ifndef OPL_OPL3_H
#define OPL_OPL3_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

#ifndef OPL_ENABLE_STEREOEXT
#define OPL_ENABLE_STEREOEXT 0
#endif

#ifndef OPL_QUIRK_CHANNELSAMPLEDELAY
#define OPL_QUIRK_CHANNELSAMPLEDELAY (!OPL_ENABLE_STEREOEXT)
#endif

/* OPL_WF_TABLE_RUNTIME=1 builds the 16 KB logsin waveform table at the
 * first OPL3_Reset instead of compiling in wf_rom.h, trading read-only
 * data for zero-initialized RAM. The table data is identical either way.
 * The first OPL3_Reset in the process must not run concurrently with
 * another reset. */
#ifndef OPL_WF_TABLE_RUNTIME
#define OPL_WF_TABLE_RUNTIME 0
#endif

/* Compatibility switches for projects pinned to older upstream Nuked-OPL3
 * commits. Both default to 0, the behavior of current upstream master
 * (cfedb09). Enabled, they reproduce the older behavior exactly, so a
 * consumer can adopt this fork with bit-identical output to the copy it
 * already ships.
 *
 * OPL_COMPAT_OLD_EG=1 selects the envelope stepping from before upstream
 * commits e4afafc and cfedb09 (June/July 2024): the per-sample envelope
 * increment is derived from the raw envelope timer every sample and the
 * increment-step table is indexed with the live global timer instead of a
 * value latched on eg_state cycles. Matches upstream 730f8c2 (Nov 2023)
 * and earlier, the vintage vendored by e.g. AdPlug, libvgm, and Furnace.
 *
 * OPL_COMPAT_DEFERRED_4OP_ALG=1 selects the pre-f2c9873 (Nov 2022)
 * behavior: writes to the 4-op enable register 0x104 do not update a
 * channel's operator routing until the channel's next C0 write. */
#ifndef OPL_COMPAT_OLD_EG
#define OPL_COMPAT_OLD_EG 0
#endif
#ifndef OPL_COMPAT_DEFERRED_4OP_ALG
#define OPL_COMPAT_DEFERRED_4OP_ALG 0
#endif

#define OPL_WRITEBUF_SIZE   1024
#define OPL_WRITEBUF_DELAY  2

typedef struct _opl3_slot opl3_slot;
typedef struct _opl3_channel opl3_channel;
typedef struct _opl3_chip opl3_chip;

struct _opl3_slot {
    opl3_channel *channel;
    opl3_chip *chip;
    int16_t *mod;
    uint8_t *trem;
    uint32_t pg_reset;
    uint32_t pg_phase;
    uint32_t pg_inc;
    /* Equal to chip->write_gen while the slot is provably inert: fully
     * attenuated, key off, all-zero phase/output state, and mod/trem frozen
     * at zeromod. Set by the trivially-dead path in OPL3_ProcessSlotImpl;
     * invalidated by any register write (write_gen bump). 0 = not dormant. */
    uint32_t dormant_gen;
    int16_t out;
    int16_t fbmod;
    int16_t prout;
    uint16_t eg_rout;
    uint16_t eg_out;
    /* Cached (reg_tl << 2) + (eg_ksl >> kslshift[reg_ksl]); maintained by
     * OPL3_EnvelopeUpdateKSL whenever any of those inputs change. Hoists
     * a load + lookup + shift out of the per-sample envelope hot path. */
    uint16_t eg_tl_ksl;
    uint16_t pg_phase_out;
    uint8_t key;
    uint8_t eg_gen;
    uint8_t reg_vib;
    uint8_t reg_mult;
    uint8_t reg_wf;
    uint8_t slot_num;
    uint8_t eg_ksl;
    uint8_t eg_ks;
    uint8_t reg_type;
    uint8_t reg_ksr;
    uint8_t reg_ksl;
    uint8_t reg_tl;
    uint8_t reg_ar;
    uint8_t reg_dr;
    uint8_t reg_sl;
    uint8_t reg_rr;
    uint8_t eg_rates[4];
    uint8_t eg_rate_hi[4];
    uint8_t eg_rate_lo[4];
    /* Phase increment per vibrato position, maintained by
     * OPL3_PhaseUpdateInc (and rebuilt on vibshift changes); pg_inc_vib[pos]
     * equals the upstream per-sample vibrato f_num adjustment for that pos. */
    uint32_t pg_inc_vib[8];
};

struct _opl3_channel {
    opl3_slot *slotz[2];/*Don't use "slots" keyword to avoid conflict with Qt applications*/
    opl3_channel *pair;
    opl3_chip *chip;
    int16_t *out[4];
#if OPL_QUIRK_CHANNELSAMPLEDELAY
    /* Mix-pass pointer lists: identical to out[] except entries pointing at
     * a delayed slot's out are redirected to its prout, which holds the
     * previous sample's out once all 36 slots are processed. out_left delays
     * slots 15-35 and out_right delays 33-35, reproducing the
     * CHANNELSAMPLEDELAY snapshots without staging slot processing around
     * the mixes. */
    int16_t *out_left[4];
    int16_t *out_right[4];
#endif
    uint8_t out_cnt;

#if OPL_ENABLE_STEREOEXT
    int32_t leftpan;
    int32_t rightpan;
#endif

    uint8_t chtype;
    uint16_t f_num;
    uint8_t block;
    uint8_t fb;
    uint8_t con;
    uint8_t alg;
    uint8_t ksv;
    uint16_t cha, chb;
    uint16_t chc, chd;
    uint8_t ch_num;
    uint8_t muted;
};

typedef struct _opl3_writebuf {
    uint64_t time;
    uint16_t reg;
    uint8_t data;
} opl3_writebuf;

struct _opl3_chip {
    opl3_channel channel[18];
    opl3_slot slot[36];
    uint16_t timer;
    uint64_t eg_timer;
    uint8_t eg_timerrem;
    uint8_t eg_state;
    uint8_t eg_add;
    uint8_t eg_timer_lo;
    uint8_t newm;
    uint8_t nts;
    uint8_t rhy;
    uint8_t vibpos;
    uint8_t vibshift;
    uint8_t tremolo;
    uint8_t tremolopos;
    uint8_t tremoloshift;
    uint8_t tremolo_dirty;
    /* Bumped on every OPL3_WriteReg call; never 0 after reset. A slot whose
     * dormant_gen matches is skipped without re-checking its dead-state
     * conditions. Wrap is handled by clearing all dormant_gen tags. */
    uint32_t write_gen;
    uint32_t noise;
    /* Bit 0 of the noise LFSR state as seen by the hh (slot 13) and sd
     * (slot 16) rhythm operators, precomputed per sample */
    uint32_t noise_hh;
    uint32_t noise_sd;
    /* Channels eligible for each mix pass: out_cnt > 0 and routed to at
     * least one output on that side. Eligibility only changes on register
     * writes; mix_dirty triggers a rebuild at the top of the next sample. */
    opl3_channel *mix_left[18];
    opl3_channel *mix_right[18];
    uint8_t nmix_left;
    uint8_t nmix_right;
    uint8_t mix_dirty;
    int16_t zeromod;
    int32_t mixbuff[4];
    uint8_t rm_hh_bit2;
    uint8_t rm_hh_bit3;
    uint8_t rm_hh_bit7;
    uint8_t rm_hh_bit8;
    uint8_t rm_tc_bit3;
    uint8_t rm_tc_bit5;

#if OPL_ENABLE_STEREOEXT
    uint8_t stereoext;
#endif

    /* OPL3L */
    int32_t rateratio;
    int32_t samplecnt;
    int16_t oldsamples[4];
    int16_t samples[4];

    uint64_t writebuf_samplecnt;
    uint32_t writebuf_cur;
    uint32_t writebuf_last;
    uint64_t writebuf_lasttime;
    opl3_writebuf writebuf[OPL_WRITEBUF_SIZE];
};

void OPL3_Generate(opl3_chip *chip, int16_t *buf);
void OPL3_GenerateResampled(opl3_chip *chip, int16_t *buf);
void OPL3_Reset(opl3_chip *chip, uint32_t samplerate);
void OPL3_Resample(opl3_chip *chip, uint32_t samplerate);
void OPL3_WriteReg(opl3_chip *chip, uint16_t reg, uint8_t v);
void OPL3_WriteRegBuffered(opl3_chip *chip, uint16_t reg, uint8_t v);
void OPL3_GenerateStream(opl3_chip *chip, int16_t *sndptr, uint32_t numsamples);

void OPL3_Generate4Ch(opl3_chip *chip, int16_t *buf4);
void OPL3_Generate4ChResampled(opl3_chip *chip, int16_t *buf4);
void OPL3_Generate4ChStream(opl3_chip *chip, int16_t *sndptr1, int16_t *sndptr2, uint32_t numsamples);

#ifdef __cplusplus
}
#endif

#endif
