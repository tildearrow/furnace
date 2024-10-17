/*
 * vic20sound.h - implementation of VIC20 sound code
 *
 * Written by
 *  Teemu Rantanen <tvr@cs.hut.fi>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#ifndef VICE_VIC20SOUND_H
#define VICE_VIC20SOUND_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

struct sound_vic20_s {
    unsigned char div;
    struct {
        unsigned char out;
        unsigned char reg;
        unsigned char shift;
        signed short ctr;
    } ch[4];
    unsigned short noisectr;
    unsigned char volume;
    int cyclecount;

    int accum;
    int accum_cycles;

    float cycles_per_sample;
    float leftover_cycles;
    int speed;

    float highpassbuf;
    float highpassbeta;
    float lowpassbuf;
    float lowpassbeta;
    bool filter_off;

    uint16_t noise_LFSR;
    uint8_t noise_LFSR0_old;
};
typedef struct sound_vic20_s sound_vic20_t;

int vic_sound_machine_init(sound_vic20_t *snd, int speed, int cycles_per_sec, bool filter_off);
void vic_sound_machine_store(sound_vic20_t *snd, uint16_t addr, uint8_t value);
int vic_sound_machine_calculate_samples(sound_vic20_t *snd, int16_t *pbuf, int nr, int soc, int scc, uint32_t delta_t);
void vic_sound_clock(sound_vic20_t *snd, uint32_t cycles);

#ifdef __cplusplus
};
#endif
#endif
