/*
 * ted-sound.h
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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

#ifndef VICE_TEDSOUND_H
#define VICE_TEDSOUND_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct plus4_sound_s {
    /* Voice 0 collect number of cycles elapsed */
    uint32_t voice0_accu;
    /* Voice 0 toggle sign and reload accu if accu reached 0 */
    uint32_t voice0_reload;
    /* Voice 0 sign of the square wave */
    int16_t voice0_sign;
    uint8_t voice0_output_enabled;

    /* Voice 1 collect number of cycles elapsed */
    uint32_t voice1_accu;
    /* Voice 1 toggle sign and reload accu if accu reached 0 */
    uint32_t voice1_reload;
    /* Voice 1 sign of the square wave */
    int16_t voice1_sign;
    uint8_t voice1_output_enabled;

    /* Volume multiplier */
    int16_t volume;
    /* 8 cycles units per sample */
    uint32_t speed;
    uint32_t sample_position_integer;
    uint32_t sample_position_remainder;
    uint32_t sample_length_integer;
    uint32_t sample_length_remainder;
    /* Digital output? */
    uint8_t digital;
    /* Noise generator active? */
    uint8_t noise;
    uint8_t noise_shift_register;

    /* Registers */
    uint8_t plus4_sound_data[5];
};

int ted_sound_machine_init(struct plus4_sound_s* snd, int speed, int cycles_per_sec);
int ted_sound_machine_calculate_samples(struct plus4_sound_s* snd, int16_t *pbuf, int nr, int sound_chip_channels);
void ted_sound_machine_store(struct plus4_sound_s* snd, uint16_t addr, uint8_t val);
uint8_t ted_sound_machine_read(struct plus4_sound_s* snd, uint16_t addr);
void ted_sound_reset(struct plus4_sound_s* snd);

#ifdef __cplusplus
};
#endif
#endif
