/* Nuked OPL3
 * Copyright (C) 2013-2020 Nuke.YKT
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
 * version: 1.8
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

#define OPL_WRITEBUF_SIZE   1024
#define OPL_WRITEBUF_DELAY  2

typedef struct _opl3_slot opl3_slot;
typedef struct _opl3_channel opl3_channel;
typedef struct _opl3_chip opl3_chip;

struct _opl3_slot {
    opl3_channel *channel;
    opl3_chip *chip;
    int16_t out;
    int16_t fbmod;
    int16_t *mod;
    int16_t prout;
    uint16_t eg_rout;
    uint16_t eg_out;
    uint8_t eg_inc;
    uint8_t eg_gen;
    uint8_t eg_rate;
    uint8_t eg_ksl;
    uint8_t *trem;
    uint8_t reg_vib;
    uint8_t reg_type;
    uint8_t reg_ksr;
    uint8_t reg_mult;
    uint8_t reg_ksl;
    uint8_t reg_tl;
    uint8_t reg_ar;
    uint8_t reg_dr;
    uint8_t reg_sl;
    uint8_t reg_rr;
    uint8_t reg_wf;
    uint8_t key;
    uint32_t pg_reset;
    uint32_t pg_phase;
    uint16_t pg_phase_out;
    uint8_t slot_num;
};

struct _opl3_channel {
    opl3_slot *slots[2];
    opl3_channel *pair;
    opl3_chip *chip;
    int16_t *out[4];

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
    uint8_t newm;
    uint8_t nts;
    uint8_t rhy;
    uint8_t vibpos;
    uint8_t vibshift;
    uint8_t tremolo;
    uint8_t tremolopos;
    uint8_t tremoloshift;
    uint32_t noise;
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
