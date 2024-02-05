/*
 * ted-sound.c
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Tibor Biczo <crown @ axelero . hu>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ted-sound.h"

/* ------------------------------------------------------------------------- */

/* FIXME: Find proper volume multiplier.  */
const int16_t volume_tab[16] = {
    0x0000, 0x0800, 0x1000, 0x1800, 0x2000, 0x2800, 0x3000, 0x3800,
    0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff
};

int ted_sound_machine_calculate_samples(struct plus4_sound_s* snd, int16_t *pbuf, int nr, int scc)
{
    int i;
    int j;
    int16_t volume;

    if (snd->digital) {
        for (i = 0; i < nr; i++) {
            pbuf[i] = (snd->volume * (snd->voice0_output_enabled + snd->voice1_output_enabled));
        }
    } else {
        for (i = 0; i < nr; i++) {
            snd->sample_position_remainder += snd->sample_length_remainder;
            if (snd->sample_position_remainder >= snd->speed) {
                snd->sample_position_remainder -= snd->speed;
                snd->sample_position_integer++;
            }
            snd->sample_position_integer += snd->sample_length_integer;
            if (snd->sample_position_integer >= 8) {
                /* Advance state engine */
                uint32_t ticks = snd->sample_position_integer >> 3;
                if (snd->voice0_accu <= ticks) {
                    uint32_t delay = ticks - snd->voice0_accu;
                    snd->voice0_sign ^= 1;
                    snd->voice0_accu = 1023 - snd->voice0_reload;
                    if (snd->voice0_accu == 0) {
                        snd->voice0_accu = 1024;
                    }
                    if (delay >= snd->voice0_accu) {
                        snd->voice0_sign = ((delay / snd->voice0_accu)
                                           & 1) ? snd->voice0_sign ^ 1
                                          : snd->voice0_sign;
                        snd->voice0_accu = snd->voice0_accu - (delay % snd->voice0_accu);
                    } else {
                        snd->voice0_accu -= delay;
                    }
                } else {
                    snd->voice0_accu -= ticks;
                }

                if (snd->voice1_accu <= ticks) {
                    uint32_t delay = ticks - snd->voice1_accu;
                    snd->voice1_sign ^= 1;
                    snd->noise_shift_register
                        = (snd->noise_shift_register << 1) +
                          ( 1 ^ ((snd->noise_shift_register >> 7) & 1) ^
                            ((snd->noise_shift_register >> 5) & 1) ^
                            ((snd->noise_shift_register >> 4) & 1) ^
                            ((snd->noise_shift_register >> 1) & 1));
                    snd->voice1_accu = 1023 - snd->voice1_reload;
                    if (snd->voice1_accu == 0) {
                        snd->voice1_accu = 1024;
                    }
                    if (delay >= snd->voice1_accu) {
                        snd->voice1_sign = ((delay / snd->voice1_accu)
                                           & 1) ? snd->voice1_sign ^ 1
                                          : snd->voice1_sign;
                        for (j = 0; j < (int)(delay / snd->voice1_accu);
                             j++) {
                            snd->noise_shift_register
                                = (snd->noise_shift_register << 1) +
                                  ( 1 ^ ((snd->noise_shift_register >> 7) & 1) ^
                                    ((snd->noise_shift_register >> 5) & 1) ^
                                    ((snd->noise_shift_register >> 4) & 1) ^
                                    ((snd->noise_shift_register >> 1) & 1));
                        }
                        snd->voice1_accu = snd->voice1_accu - (delay % snd->voice1_accu);
                    } else {
                        snd->voice1_accu -= delay;
                    }
                } else {
                    snd->voice1_accu -= ticks;
                }
            }
            snd->sample_position_integer = snd->sample_position_integer & 7;

            volume = 0;

            if (snd->voice0_output_enabled && snd->voice0_sign) {
                volume += snd->volume;
            }
            if (snd->voice1_output_enabled && !snd->noise && snd->voice1_sign) {
                volume += snd->volume;
            }
            if (snd->voice1_output_enabled && snd->noise && (!(snd->noise_shift_register & 1))) {
                volume += snd->volume;
            }

            pbuf[i] = volume;
        }
    }
    return nr;
}

int ted_sound_machine_init(struct plus4_sound_s* snd, int speed, int cycles_per_sec)
{
    uint8_t val;
    memset(snd,0,sizeof(struct plus4_sound_s));

    snd->speed = speed;
    snd->sample_length_integer = cycles_per_sec / speed;
    snd->sample_length_remainder = cycles_per_sec % speed;
    snd->sample_position_integer = 0;
    snd->sample_position_remainder = 0;
    snd->noise_shift_register = 0;

    snd->voice0_reload = (snd->plus4_sound_data[0] | (snd->plus4_sound_data[4] << 8));
    snd->voice1_reload = (snd->plus4_sound_data[1] | (snd->plus4_sound_data[2] << 8));
    val = snd->plus4_sound_data[3];
    snd->volume = volume_tab[val & 0x0f];
    snd->voice0_output_enabled = (val & 0x10) ? 1 : 0;
    snd->voice1_output_enabled = (val & 0x60) ? 1 : 0;
    snd->noise = ((val & 0x60) == 0x40) ? 1 : 0;
    snd->digital = val & 0x80;
    if (snd->digital) {
        snd->voice0_sign = 1;
        snd->voice0_accu = 0;
        snd->voice1_sign = 1;
        snd->voice1_accu = 0;
        snd->noise_shift_register = 0;
    }

    return 1;
}

void ted_sound_machine_store(struct plus4_sound_s* snd, uint16_t addr, uint8_t val)
{
    switch (addr) {
        case 0x0e:
            snd->plus4_sound_data[0] = val;
            snd->voice0_reload = (snd->plus4_sound_data[0] | (snd->plus4_sound_data[4] << 8));
            break;
        case 0x0f:
            snd->plus4_sound_data[1] = val;
            snd->voice1_reload = (snd->plus4_sound_data[1] | (snd->plus4_sound_data[2] << 8));
            break;
        case 0x10:
            snd->plus4_sound_data[2] = val & 3;
            snd->voice1_reload = (snd->plus4_sound_data[1] | (snd->plus4_sound_data[2] << 8));
            break;
        case 0x11:
            snd->volume = volume_tab[val & 0x0f];
            snd->voice0_output_enabled = (val & 0x10) ? 1 : 0;
            snd->voice1_output_enabled = (val & 0x60) ? 1 : 0;
            snd->noise = ((val & 0x60) == 0x40) ? 1 : 0;
            snd->digital = val & 0x80;
            if (snd->digital) {
                snd->voice0_sign = 1;
                snd->voice0_accu = 0;
                snd->voice1_sign = 1;
                snd->voice1_accu = 0;
                snd->noise_shift_register = 0;
            }
            snd->plus4_sound_data[3] = val;
            break;
        case 0x12:
            snd->plus4_sound_data[4] = val & 3;
            snd->voice0_reload = (snd->plus4_sound_data[0] | (snd->plus4_sound_data[4] << 8));
            break;
    }
}

uint8_t ted_sound_machine_read(struct plus4_sound_s* snd, uint16_t addr)
{
    switch (addr) {
        case 0x0e:
            return snd->plus4_sound_data[0];
        case 0x0f:
            return snd->plus4_sound_data[1];
        case 0x10:
            return snd->plus4_sound_data[2] | 0xc0;
        case 0x11:
            return snd->plus4_sound_data[3];
        case 0x12:
            return snd->plus4_sound_data[4];
    }

    return 0;
}

void ted_sound_reset(struct plus4_sound_s* snd)
{
    uint16_t i;

    snd->noise_shift_register = 0;
    for (i = 0x0e; i <= 0x12; i++) {
        ted_sound_machine_store(snd,i,0);
    }
}
