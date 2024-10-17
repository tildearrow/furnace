/*
 * vic20sound.c - Implementation of VIC20 sound code.
 *
 * Written by
 *  Rami Rasanen <raipsu@users.sf.net>
 *  Ville-Matias Heikkila <viznut@iki.fi>
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

#include "vic20sound.h"

/* ---------------------------------------------------------------------*/

static float voltagefunction[] = {
        0.00f,   148.28f,   296.55f,   735.97f,   914.88f,  1126.89f,  1321.86f,  1503.07f,  1603.50f,
     1758.00f,  1913.98f,  2070.94f,  2220.36f,  2342.91f,  2488.07f,  3188.98f,  3285.76f,  3382.53f,
     3479.31f,  3576.08f,  3672.86f,  3769.63f,  3866.41f,  3963.18f,  4059.96f,  4248.10f,  4436.24f,
     4624.38f,  4812.53f,  5000.67f,  5188.81f,  5192.91f,  5197.00f,  5338.52f,  5480.04f,  5621.56f,
     5763.07f,  5904.59f,  6046.11f,  6187.62f,  6329.14f,  6609.31f,  6889.47f,  7169.64f,  7449.80f,
     7729.97f,  7809.36f,  7888.75f,  7968.13f,  8047.52f,  8126.91f,  8206.30f,  8285.69f,  8365.07f,
     8444.46f,  8523.85f,  8603.24f,  8905.93f,  9208.63f,  9511.32f,  9814.02f,  9832.86f,  9851.70f,
     9870.54f,  9889.38f,  9908.22f,  9927.07f,  9945.91f,  9964.75f,  9983.59f, 10002.43f, 10021.27f,
    10040.12f, 10787.23f, 11534.34f, 12281.45f, 12284.98f, 12288.50f, 12292.03f, 12295.56f, 12299.09f,
    12302.62f, 12306.15f, 12309.68f, 12313.21f, 12316.74f, 12320.26f, 12323.79f, 12327.32f, 13113.05f,
    13898.78f, 13910.58f, 13922.39f, 13934.19f, 13945.99f, 13957.80f, 13969.60f, 13981.40f, 13993.21f,
    14005.01f, 14016.81f, 14028.62f, 14040.42f, 14052.22f, 14064.03f, 16926.31f, 16987.04f, 17047.77f,
    17108.50f, 17169.23f, 17229.96f, 17290.69f, 17351.42f, 17412.15f, 17472.88f, 17533.61f, 17594.34f,
    17655.07f, 17715.80f, 17776.53f, 17837.26f, 18041.51f, 18245.77f, 18450.02f, 18654.28f, 18858.53f,
    19062.78f, 19267.04f, 19471.29f, 19675.55f, 19879.80f, 20084.05f, 20288.31f, 20417.74f, 20547.17f,
    20676.61f, 20774.26f, 20871.91f, 20969.55f, 21067.20f, 21164.85f, 21262.50f, 21360.15f, 21457.80f,
    21555.45f, 21653.09f, 21750.74f, 21848.39f, 21946.04f, 22043.69f, 22141.34f, 22212.33f, 22283.33f,
    22354.33f, 22425.33f, 22496.32f, 22567.32f, 22638.32f, 22709.32f, 22780.31f, 22851.31f, 22922.31f,
    22993.31f, 23064.30f, 23135.30f, 23206.30f, 23255.45f, 23304.60f, 23353.75f, 23402.91f, 23452.06f,
    23501.21f, 23550.36f, 23599.51f, 23648.67f, 23768.81f, 23888.96f, 24009.11f, 24129.26f, 24249.41f,
    24369.56f, 24451.92f, 24534.28f, 24616.63f, 24698.99f, 24781.35f, 24863.70f, 24946.06f, 25028.42f,
    25110.77f, 25193.13f, 25275.49f, 25357.84f, 25440.20f, 25522.56f, 25604.92f, 25658.87f, 25712.83f,
    25766.79f, 25820.75f, 25874.71f, 25928.66f, 25982.62f, 26036.58f, 26090.54f, 26144.49f, 26198.45f,
    26252.41f, 26306.37f, 26360.33f, 26414.28f, 26501.23f, 26588.17f, 26675.12f, 26762.06f, 26849.01f,
    26935.95f, 27022.90f, 27109.84f, 27196.78f, 27283.73f, 27370.67f, 27457.62f, 27544.56f, 27631.51f,
    27718.45f, 27726.89f, 27735.33f, 27743.78f, 27752.22f, 27760.66f, 27769.10f, 27777.54f, 27785.98f,
    27794.43f, 27802.87f, 27811.31f, 27819.75f, 27828.19f, 27836.63f, 27845.08f, 27853.52f, 27861.96f,
    27870.40f, 27878.84f, 27887.28f, 27895.73f, 27904.17f, 27912.61f, 27921.05f, 27929.49f, 27937.93f,
    27946.38f, 27954.82f, 27963.26f, 27971.70f, 27980.14f, 27988.58f, 27997.03f, 28005.47f, 28013.91f,
    28022.35f, 28030.79f, 28039.23f, 28047.68f, 28056.12f, 28064.56f, 28073.00f, 28081.44f, 28089.88f,
    28098.33f, 28106.77f, 28115.21f, 28123.65f, 28132.09f, 28140.53f, 28148.98f, 28157.42f, 28165.86f,
    28174.30f, 28182.74f, 28191.18f, 28199.63f, 28208.07f, 28216.51f, 28224.95f, 28233.39f, 28241.83f,
    28250.28f, 28258.72f, 28267.16f, 28275.60f, 28284.04f, 28292.48f, 28300.93f, 28309.37f, 28317.81f,
    28326.25f, 28334.69f, 28343.13f, 28351.58f, 28360.02f, 28368.46f, 28376.90f, 28385.34f, 28393.78f,
    28402.23f, 28410.67f, 28419.11f, 28427.55f, 28435.99f, 28444.43f, 28452.88f, 28461.32f, 28469.76f,
    28478.20f, 28486.64f, 28495.08f, 28503.53f, 28511.97f, 28520.41f, 28528.85f, 28537.29f, 28545.73f,
    28554.18f, 28562.62f, 28571.06f, 28579.50f, 28587.94f, 28596.38f, 28604.83f, 28613.27f, 28621.71f,
    28630.15f, 28638.59f, 28647.03f, 28655.48f, 28663.92f, 28672.36f, 28680.80f, 28689.24f, 28697.68f,
    28706.13f, 28714.57f, 28723.01f, 28731.45f, 28739.89f, 28748.33f, 28756.78f, 28765.22f, 28773.66f,
    28782.10f, 28790.54f, 28798.98f, 28807.43f, 28815.87f, 28824.31f, 28832.75f, 28841.19f, 28849.63f,
    28858.08f, 28866.52f, 28874.96f, 28883.40f, 28891.84f, 28900.28f, 28908.73f, 28917.17f, 28925.61f,
    28934.05f, 28942.49f, 28950.93f, 28959.38f, 28967.82f, 28976.26f, 28984.70f, 28993.14f, 29001.58f,
    29010.03f, 29018.47f, 29026.91f, 29035.35f, 29043.79f, 29052.23f, 29060.68f, 29069.12f, 29077.56f,
    29086.00f, 29094.44f, 29102.88f, 29111.33f, 29119.77f, 29128.21f, 29136.65f, 29145.09f, 29153.53f,
    29161.98f, 29170.42f, 29178.86f, 29187.30f, 29195.74f, 29204.18f, 29212.63f, 29221.07f, 29229.51f,
    29237.95f, 29246.39f, 29254.83f, 29263.28f, 29271.72f, 29280.16f, 29288.60f, 29297.04f, 29305.48f,
    29313.93f, 29322.37f, 29330.81f, 29339.25f, 29347.69f, 29356.13f, 29364.58f, 29373.02f, 29381.46f,
    29389.90f, 29398.34f, 29406.78f, 29415.23f, 29423.67f, 29432.11f, 29440.55f, 29448.99f, 29457.43f,
    29465.88f, 29474.32f, 29482.76f, 29491.20f
};

void vic_sound_clock(sound_vic20_t *snd, uint32_t cycles);

int vic_sound_machine_calculate_samples(sound_vic20_t *snd, int16_t *pbuf, int nr, int soc, int scc, uint32_t delta_t)
{
    int s = 0;
    int i;
    float o;
    float v;
    int16_t vicbuf;
    int samples_to_do;

    while (s < nr && delta_t >= snd->cycles_per_sample - snd->leftover_cycles) {
        samples_to_do = (int)(snd->cycles_per_sample - snd->leftover_cycles);
        snd->leftover_cycles += samples_to_do - snd->cycles_per_sample;
        vic_sound_clock(snd, samples_to_do);

        v = voltagefunction[(((snd->accum * 7) / snd->accum_cycles) + 1) * snd->volume];
        if (snd->filter_off) {
            o = v;
        } else {
            o = snd->lowpassbuf - snd->highpassbuf;
            snd->highpassbuf += snd->highpassbeta * (snd->lowpassbuf - snd->highpassbuf);
            snd->lowpassbuf += snd->lowpassbeta * (v - snd->lowpassbuf);
        }

        if (o < -32768) {
            vicbuf = -32768;
        } else if (o > 32767) {
            vicbuf = 32767;
        } else {
            vicbuf = (int16_t)o;
        }

        for (i = 0; i < soc; i++) {
            pbuf[(s * soc) + i] = vicbuf;
        }
        s++;
        snd->accum = 0;
        snd->accum_cycles = 0;
        delta_t -= samples_to_do;
    }
    if (delta_t > 0) {
        snd->leftover_cycles += delta_t;
        vic_sound_clock(snd, delta_t);
        delta_t = 0;
    }
    return s;
}

void vic_sound_clock(sound_vic20_t *snd, uint32_t cycles)
{
    uint32_t i;
    int j, enabled;

    if (cycles <= 0) {
        return;
    }

    for (j = 0; j < 4; j++) {
        int chspeed = "\4\3\2\1"[j];

        if (snd->ch[j].ctr > cycles) {
            snd->accum += snd->ch[j].out * cycles;
            snd->ch[j].ctr -= cycles;
        } else {
            for (i = cycles; i; i--) {
                snd->ch[j].ctr--;
                if (snd->ch[j].ctr <= 0) {
                    int a = (~snd->ch[j].reg) & 127;
                    int edge_trigger;
                    a = a ? a : 128;
                    snd->ch[j].ctr += a << chspeed;
                    enabled = (snd->ch[j].reg & 128) >> 7;
                    edge_trigger = (snd->noise_LFSR & 1) & !snd->noise_LFSR0_old;
                    
                    if((j != 3) || ((j == 3) && edge_trigger)) {
                        uint8_t shift = snd->ch[j].shift;
                        shift = ((shift << 1) | (((((shift & 128) >> 7)) ^ 1) & enabled));
                        snd->ch[j].shift = shift;
                    }
                    if(j == 3) {
                        int bit3  = (snd->noise_LFSR >> 3) & 1;
                        int bit12 = (snd->noise_LFSR >> 12) & 1;
                        int bit14 = (snd->noise_LFSR >> 14) & 1;
                        int bit15 = (snd->noise_LFSR >> 15) & 1;
                        int gate1 = bit3 ^ bit12;
                        int gate2 = bit14 ^ bit15;
                        int gate3 = (gate1 ^ gate2) ^ 1;
                        int gate4 = (gate3 & enabled) ^ 1;
                        snd->noise_LFSR0_old = snd->noise_LFSR & 1;
                        snd->noise_LFSR = (snd->noise_LFSR << 1) | gate4;
                    }
                    snd->ch[j].out = snd->ch[j].shift & (j == 3 ? enabled : 1);                    
                }
                snd->accum += snd->ch[j].out; /* FIXME: doesn't take DC offset into account */
            }
        }
    }

    snd->accum_cycles += cycles;
}

void vic_sound_machine_store(sound_vic20_t *snd, uint16_t addr, uint8_t value)
{
    switch (addr) {
        case 0xA:
            snd->ch[0].reg = value;
            break;
        case 0xB:
            snd->ch[1].reg = value;
            break;
        case 0xC:
            snd->ch[2].reg = value;
            break;
        case 0xD:
            snd->ch[3].reg = value;
            break;
        case 0xE:
            snd->volume = value & 0x0f;
            break;
    }
}

int vic_sound_machine_init(sound_vic20_t *snd, int speed, int cycles_per_sec, bool filter_off)
{
    uint32_t i;
    float dt;

    memset(snd, 0, sizeof(sound_vic20_t));

    snd->cycles_per_sample = (float)cycles_per_sec / speed;
    snd->leftover_cycles = 0.0f;

    snd->lowpassbuf = 0.0f;
    snd->highpassbuf = 0.0f;
    snd->filter_off = filter_off;

    snd->speed = speed;

    dt = 1.f / speed;

    /*
     Audio output stage

                                5V
     -----+
     audio|      1k             |
          +---+---R---+--------(K)          +-----
      out |   |       |         |           |audio
     -----+   C .01   C .1      |    1 uF   |
              | uF    |  uF     +-----C-----+ 1K
                                |           |
             GND     GND        R 470       | amp
                                |           +-----

                               GND

    */

    /* Low-pass:  R = 1 kOhm, C = 100 nF; RC = 1e3*1e-7 = 1e-4 (1591 Hz) */
    snd->lowpassbeta  = dt / ( dt + 1e-4f );
    /* High-pass: R = 1 kOhm, C =   1 uF; RC = 1e3*1e-6 = 1e-3 ( 159 Hz) */
    snd->highpassbeta = dt / ( dt + 1e-3f );

    for (i = 0; i < 16; i++) {
        vic_sound_machine_store(snd, (uint16_t)i, 0);
    }

    return 1;
}
