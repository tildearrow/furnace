/* Nuked OPM
 * Copyright (C) 2022 Nuke.YKT
 *
 * This file is part of Nuked OPM.
 *
 * Nuked OPM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 2.1
 * of the License, or (at your option) any later version.
 *
 * Nuked OPM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Nuked OPM. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Nuked OPM emulator.
 *  Thanks:
 *      siliconpr0n.org(digshadow, John McMaster):
 *          YM2151 and other FM chip decaps and die shots.
 *
 * version: 0.9.3 beta
 */
#include <string.h>
#include <stdint.h>
#include "opm.h"

enum {
    eg_num_attack = 0,
    eg_num_decay = 1,
    eg_num_sustain = 2,
    eg_num_release = 3
};

/* logsin table */
static const uint16_t logsinrom[256] = {
    0x859, 0x6c3, 0x607, 0x58b, 0x52e, 0x4e4, 0x4a6, 0x471,
    0x443, 0x41a, 0x3f5, 0x3d3, 0x3b5, 0x398, 0x37e, 0x365,
    0x34e, 0x339, 0x324, 0x311, 0x2ff, 0x2ed, 0x2dc, 0x2cd,
    0x2bd, 0x2af, 0x2a0, 0x293, 0x286, 0x279, 0x26d, 0x261,
    0x256, 0x24b, 0x240, 0x236, 0x22c, 0x222, 0x218, 0x20f,
    0x206, 0x1fd, 0x1f5, 0x1ec, 0x1e4, 0x1dc, 0x1d4, 0x1cd,
    0x1c5, 0x1be, 0x1b7, 0x1b0, 0x1a9, 0x1a2, 0x19b, 0x195,
    0x18f, 0x188, 0x182, 0x17c, 0x177, 0x171, 0x16b, 0x166,
    0x160, 0x15b, 0x155, 0x150, 0x14b, 0x146, 0x141, 0x13c,
    0x137, 0x133, 0x12e, 0x129, 0x125, 0x121, 0x11c, 0x118,
    0x114, 0x10f, 0x10b, 0x107, 0x103, 0x0ff, 0x0fb, 0x0f8,
    0x0f4, 0x0f0, 0x0ec, 0x0e9, 0x0e5, 0x0e2, 0x0de, 0x0db,
    0x0d7, 0x0d4, 0x0d1, 0x0cd, 0x0ca, 0x0c7, 0x0c4, 0x0c1,
    0x0be, 0x0bb, 0x0b8, 0x0b5, 0x0b2, 0x0af, 0x0ac, 0x0a9,
    0x0a7, 0x0a4, 0x0a1, 0x09f, 0x09c, 0x099, 0x097, 0x094,
    0x092, 0x08f, 0x08d, 0x08a, 0x088, 0x086, 0x083, 0x081,
    0x07f, 0x07d, 0x07a, 0x078, 0x076, 0x074, 0x072, 0x070,
    0x06e, 0x06c, 0x06a, 0x068, 0x066, 0x064, 0x062, 0x060,
    0x05e, 0x05c, 0x05b, 0x059, 0x057, 0x055, 0x053, 0x052,
    0x050, 0x04e, 0x04d, 0x04b, 0x04a, 0x048, 0x046, 0x045,
    0x043, 0x042, 0x040, 0x03f, 0x03e, 0x03c, 0x03b, 0x039,
    0x038, 0x037, 0x035, 0x034, 0x033, 0x031, 0x030, 0x02f,
    0x02e, 0x02d, 0x02b, 0x02a, 0x029, 0x028, 0x027, 0x026,
    0x025, 0x024, 0x023, 0x022, 0x021, 0x020, 0x01f, 0x01e,
    0x01d, 0x01c, 0x01b, 0x01a, 0x019, 0x018, 0x017, 0x017,
    0x016, 0x015, 0x014, 0x014, 0x013, 0x012, 0x011, 0x011,
    0x010, 0x00f, 0x00f, 0x00e, 0x00d, 0x00d, 0x00c, 0x00c,
    0x00b, 0x00a, 0x00a, 0x009, 0x009, 0x008, 0x008, 0x007,
    0x007, 0x007, 0x006, 0x006, 0x005, 0x005, 0x005, 0x004,
    0x004, 0x004, 0x003, 0x003, 0x003, 0x002, 0x002, 0x002,
    0x002, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001,
    0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000
};

/* exp table */
static const uint16_t exprom[256] = {
    0x7fa, 0x7f5, 0x7ef, 0x7ea, 0x7e4, 0x7df, 0x7da, 0x7d4,
    0x7cf, 0x7c9, 0x7c4, 0x7bf, 0x7b9, 0x7b4, 0x7ae, 0x7a9,
    0x7a4, 0x79f, 0x799, 0x794, 0x78f, 0x78a, 0x784, 0x77f,
    0x77a, 0x775, 0x770, 0x76a, 0x765, 0x760, 0x75b, 0x756,
    0x751, 0x74c, 0x747, 0x742, 0x73d, 0x738, 0x733, 0x72e,
    0x729, 0x724, 0x71f, 0x71a, 0x715, 0x710, 0x70b, 0x706,
    0x702, 0x6fd, 0x6f8, 0x6f3, 0x6ee, 0x6e9, 0x6e5, 0x6e0,
    0x6db, 0x6d6, 0x6d2, 0x6cd, 0x6c8, 0x6c4, 0x6bf, 0x6ba,
    0x6b5, 0x6b1, 0x6ac, 0x6a8, 0x6a3, 0x69e, 0x69a, 0x695,
    0x691, 0x68c, 0x688, 0x683, 0x67f, 0x67a, 0x676, 0x671,
    0x66d, 0x668, 0x664, 0x65f, 0x65b, 0x657, 0x652, 0x64e,
    0x649, 0x645, 0x641, 0x63c, 0x638, 0x634, 0x630, 0x62b,
    0x627, 0x623, 0x61e, 0x61a, 0x616, 0x612, 0x60e, 0x609,
    0x605, 0x601, 0x5fd, 0x5f9, 0x5f5, 0x5f0, 0x5ec, 0x5e8,
    0x5e4, 0x5e0, 0x5dc, 0x5d8, 0x5d4, 0x5d0, 0x5cc, 0x5c8,
    0x5c4, 0x5c0, 0x5bc, 0x5b8, 0x5b4, 0x5b0, 0x5ac, 0x5a8,
    0x5a4, 0x5a0, 0x59c, 0x599, 0x595, 0x591, 0x58d, 0x589,
    0x585, 0x581, 0x57e, 0x57a, 0x576, 0x572, 0x56f, 0x56b,
    0x567, 0x563, 0x560, 0x55c, 0x558, 0x554, 0x551, 0x54d,
    0x549, 0x546, 0x542, 0x53e, 0x53b, 0x537, 0x534, 0x530,
    0x52c, 0x529, 0x525, 0x522, 0x51e, 0x51b, 0x517, 0x514,
    0x510, 0x50c, 0x509, 0x506, 0x502, 0x4ff, 0x4fb, 0x4f8,
    0x4f4, 0x4f1, 0x4ed, 0x4ea, 0x4e7, 0x4e3, 0x4e0, 0x4dc,
    0x4d9, 0x4d6, 0x4d2, 0x4cf, 0x4cc, 0x4c8, 0x4c5, 0x4c2,
    0x4be, 0x4bb, 0x4b8, 0x4b5, 0x4b1, 0x4ae, 0x4ab, 0x4a8,
    0x4a4, 0x4a1, 0x49e, 0x49b, 0x498, 0x494, 0x491, 0x48e,
    0x48b, 0x488, 0x485, 0x482, 0x47e, 0x47b, 0x478, 0x475,
    0x472, 0x46f, 0x46c, 0x469, 0x466, 0x463, 0x460, 0x45d,
    0x45a, 0x457, 0x454, 0x451, 0x44e, 0x44b, 0x448, 0x445,
    0x442, 0x43f, 0x43c, 0x439, 0x436, 0x433, 0x430, 0x42d,
    0x42a, 0x428, 0x425, 0x422, 0x41f, 0x41c, 0x419, 0x416,
    0x414, 0x411, 0x40e, 0x40b, 0x408, 0x406, 0x403, 0x400
};

/* Envelope generator */
static const uint32_t eg_stephi[4][4] = {
    { 0, 0, 0, 0 },
    { 1, 0, 0, 0 },
    { 1, 0, 1, 0 },
    { 1, 1, 1, 0 }
};

/* Phase generator */
static const uint32_t pg_detune[8] = { 16, 17, 19, 20, 22, 24, 27, 29 };

typedef struct {
    int32_t basefreq;
    int32_t approxtype;
    int32_t slope;
} freqtable_t;

static const freqtable_t pg_freqtable[64] = {
    { 1299, 1, 19 },
    { 1318, 1, 19 },
    { 1337, 1, 19 },
    { 1356, 1, 20 },
    { 1376, 1, 20 },
    { 1396, 1, 20 },
    { 1416, 1, 21 },
    { 1437, 1, 20 },
    { 1458, 1, 21 },
    { 1479, 1, 21 },
    { 1501, 1, 22 },
    { 1523, 1, 22 },
    { 0,    0, 16 },
    { 0,    0, 16 },
    { 0,    0, 16 },
    { 0,    0, 16 },
    { 1545, 1, 22 },
    { 1567, 1, 22 },
    { 1590, 1, 23 },
    { 1613, 1, 23 },
    { 1637, 1, 23 },
    { 1660, 1, 24 },
    { 1685, 1, 24 },
    { 1709, 1, 24 },
    { 1734, 1, 25 },
    { 1759, 1, 25 },
    { 1785, 1, 26 },
    { 1811, 1, 26 },
    { 0,    0, 16 },
    { 0,    0, 16 },
    { 0,    0, 16 },
    { 0,    0, 16 },
    { 1837, 1, 26 },
    { 1864, 1, 27 },
    { 1891, 1, 27 },
    { 1918, 1, 28 },
    { 1946, 1, 28 },
    { 1975, 1, 28 },
    { 2003, 1, 29 },
    { 2032, 1, 30 },
    { 2062, 1, 30 },
    { 2092, 1, 30 },
    { 2122, 1, 31 },
    { 2153, 1, 31 },
    { 0,    0, 16 },
    { 0,    0, 16 },
    { 0,    0, 16 },
    { 0,    0, 16 },
    { 2185, 1, 31 },
    { 2216, 0, 31 },
    { 2249, 0, 31 },
    { 2281, 0, 31 },
    { 2315, 0, 31 },
    { 2348, 0, 31 },
    { 2382, 0, 30 },
    { 2417, 0, 30 },
    { 2452, 0, 30 },
    { 2488, 0, 30 },
    { 2524, 0, 30 },
    { 2561, 0, 30 },
    { 0,    0, 16 },
    { 0,    0, 16 },
    { 0,    0, 16 },
    { 0,    0, 16 }
};


/* FM algorithm */
static const uint32_t fm_algorithm[4][6][8] = {
    {
        { 1, 1, 1, 1, 1, 1, 1, 1 }, /* M1_0          */
        { 1, 1, 1, 1, 1, 1, 1, 1 }, /* M1_1          */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* C1            */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* Last operator */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* Last operator */
        { 0, 0, 0, 0, 0, 0, 0, 1 }  /* Out           */
    },
    {
        { 0, 1, 0, 0, 0, 1, 0, 0 }, /* M1_0          */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* M1_1          */
        { 1, 1, 1, 0, 0, 0, 0, 0 }, /* C1            */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* Last operator */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* Last operator */
        { 0, 0, 0, 0, 0, 1, 1, 1 }  /* Out           */
    },
    {
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* M1_0          */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* M1_1          */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* C1            */
        { 1, 0, 0, 1, 1, 1, 1, 0 }, /* Last operator */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* Last operator */
        { 0, 0, 0, 0, 1, 1, 1, 1 }  /* Out           */
    },
    {
        { 0, 0, 1, 0, 0, 1, 0, 0 }, /* M1_0          */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* M1_1          */
        { 0, 0, 0, 1, 0, 0, 0, 0 }, /* C1            */
        { 1, 1, 0, 1, 1, 0, 0, 0 }, /* Last operator */
        { 0, 0, 1, 0, 0, 0, 0, 0 }, /* Last operator */
        { 1, 1, 1, 1, 1, 1, 1, 1 }  /* Out           */
    }
};

static uint16_t lfo_counter2_table[] = {
    0x0000, 0x4000, 0x6000, 0x7000,
    0x7800, 0x7c00, 0x7e00, 0x7f00,
    0x7f80, 0x7fc0, 0x7fe0, 0x7ff0,
    0x7ff8, 0x7ffc, 0x7ffe, 0x7fff
};

static inline int32_t OPM_KCToFNum(int32_t kcode)
{
    int32_t kcode_h = (kcode >> 4) & 63;
    int32_t kcode_l = kcode & 15;
    int32_t i, slope, sum = 0;
    if (pg_freqtable[kcode_h].approxtype)
    {
        for (i = 0; i < 4; i++)
        {
            if (kcode_l & (1 << i))
            {
                sum += (pg_freqtable[kcode_h].slope >> (3 - i));
            }
        }
    }
    else
    {
        slope = pg_freqtable[kcode_h].slope | 1;
        if (kcode_l & 1)
        {
            sum += (slope >> 3) + 2;
        }
        if (kcode_l & 2)
        {
            sum += 8;
        }
        if (kcode_l & 4)
        {
            sum += slope >> 1;
        }
        if (kcode_l & 8)
        {
            sum += slope;
            sum++;
        }
        if ((kcode_l & 12) == 12 && (pg_freqtable[kcode_h].slope & 1) == 0)
        {
            sum += 4;
        }
    }
    return pg_freqtable[kcode_h].basefreq + (sum >> 1);
}

static inline int32_t OPM_LFOApplyPMS(int32_t lfo, int32_t pms)
{
    int32_t t, out;
    int32_t top = (lfo >> 4) & 7;
    if (pms != 7)
    {
        top >>= 1;
    }
    t = (top & 6) == 6 || ((top & 3) == 3 && pms >= 6);

    out = top + ((top >> 2) & 1) + t;
    out = out * 2 + ((lfo >> 4) & 1);

    if (pms == 7)
    {
        out >>= 1;
    }
    out &= 15;
    out = (lfo & 15) + out * 16;
    switch (pms)
    {
    case 0:
    default:
        out = 0;
        break;
    case 1:
        out = (out >> 5) & 3;
        break;
    case 2:
        out = (out >> 4) & 7;
        break;
    case 3:
        out = (out >> 3) & 15;
        break;
    case 4:
        out = (out >> 2) & 31;
        break;
    case 5:
        out = (out >> 1) & 63;
        break;
    case 6:
        out = (out & 255) << 1;
        break;
    case 7:
        out = (out & 255) << 2;
        break;
    }
    return out;
}

static inline int32_t OPM_CalcKCode(int32_t kcf, int32_t lfo, int32_t lfo_sign, int32_t dt)
{
    int32_t t2, t3, b0, b1, b2, b3, w2, w3, w6;
    int32_t overflow1 = 0;
    int32_t overflow2 = 0;
    int32_t negoverflow = 0;
    int32_t sum, cr;
    if (!lfo_sign)
    {
        lfo = ~lfo;
    }
    sum = (kcf & 8191) + (lfo&8191) + (!lfo_sign);
    cr = ((kcf & 255) + (lfo & 255) + (!lfo_sign)) >> 8;
    if (sum & (1 << 13))
    {
        overflow1 = 1;
    }
    sum &= 8191;
    if (lfo_sign && ((((sum >> 6) & 3) == 3) || cr))
    {
        sum += 64;
    }
    if (!lfo_sign && !cr)
    {
        sum += (-64)&8191;
        negoverflow = 1;
    }
    if (sum & (1 << 13))
    {
        overflow2 = 1;
    }
    sum &= 8191;
    if ((!lfo_sign && !overflow1) || (negoverflow && !overflow2))
    {
        sum = 0;
    }
    if (lfo_sign && (overflow1 || overflow2))
    {
        sum = 8127;
    }
        
    t2 = sum & 63;
    if (dt == 2)
        t2 += 20;
    if (dt == 2 || dt == 3)
        t2 += 32;

    b0 = (t2 >> 6) & 1;
    b1 = dt == 2;
    b2 = ((sum >> 6) & 1);
    b3 = ((sum >> 7) & 1);


    w2 = (b0 && b1 && b2);
    w3 = (b0 && b3);
    w6 = (b0 && !w2 && !w3) || (b3 && !b0 && b1);

    t2 &= 63;

    t3 = (sum >> 6) + w6 + b1 + (w2 || w3) * 2 + (dt == 3) * 4 + (dt != 0) * 8;
    if (t3 & 128)
    {
        t2 = 63;
        t3 = 126;
    }
    sum = t3 * 64 + t2;
    return sum;
}

static inline void OPM_PhaseCalcFNumBlock(opm_t *chip)
{
    uint32_t slot = (chip->cycles + 7) % 32;
    uint32_t channel = slot % 8;
    uint32_t kcf = (chip->ch_kc[channel] << 6) + chip->ch_kf[channel];
    uint32_t lfo = chip->lfo_pmd ? chip->lfo_pm_lock : 0;
    uint32_t pms = chip->ch_pms[channel];
    uint32_t dt = chip->sl_dt2[slot];
    int32_t lfo_pm = OPM_LFOApplyPMS(lfo & 127, pms);
    uint32_t kcode = OPM_CalcKCode(kcf, lfo_pm, (lfo & 0x80) != 0 && pms != 0 ? 0 : 1, dt);
    uint32_t fnum = OPM_KCToFNum(kcode);
    uint32_t kcode_h = kcode >> 8;
    chip->pg_fnum[slot] = fnum;
    chip->pg_kcode[slot] = kcode_h;
}

static inline void OPM_PhaseCalcIncrement(opm_t *chip)
{
    uint32_t slot = chip->cycles;
    uint32_t channel = slot % 8;
    uint32_t dt = chip->sl_dt1[slot];
    uint32_t dt_l = dt & 3;
    uint32_t detune = 0;
    uint32_t multi = chip->sl_mul[slot];
    uint32_t kcode = chip->pg_kcode[slot];
    uint32_t fnum = chip->pg_fnum[slot];
    uint32_t block = kcode >> 2;
    uint32_t basefreq = (fnum << block) >> 2;
    uint32_t note, sum, sum_h, sum_l, inc;
    /* Apply detune */
    if (dt_l)
    {
        if (kcode > 0x1c)
        {
            kcode = 0x1c;
        }
        block = kcode >> 2;
        note = kcode & 0x03;
        sum = block + 9 + ((dt_l == 3) | (dt_l & 0x02));
        sum_h = sum >> 1;
        sum_l = sum & 0x01;
        detune = pg_detune[(sum_l << 2) | note] >> (9 - sum_h);
    }
    if (dt & 0x04)
    {
        basefreq -= detune;
    }
    else
    {
        basefreq += detune;
    }
    basefreq &= 0x1ffff;
    if (multi)
    {
        inc = basefreq * multi;
    }
    else
    {
        inc = basefreq >> 1;
    }
    inc &= 0xfffff;
    chip->pg_inc[slot] = inc;
}

static inline void OPM_PhaseGenerate(opm_t *chip)
{
    uint32_t slot = (chip->cycles + 27) % 32;
    chip->pg_reset_latch[slot] = chip->pg_reset[slot];
    slot = (chip->cycles + 25) % 32;
    /* Mask increment */
    if (chip->pg_reset_latch[slot])
    {
        chip->pg_inc[slot] = 0;
    }
    /* Phase step */
    slot = (chip->cycles + 24) % 32;
    if (chip->pg_reset_latch[slot] || chip->mode_test[3])
    {
        chip->pg_phase[slot] = 0;
    }
    chip->pg_phase[slot] += chip->pg_inc[slot];
    chip->pg_phase[slot] &= 0xfffff;
}

static inline void OPM_PhaseDebug(opm_t *chip)
{
    chip->pg_serial >>= 1;
    if (chip->cycles == 5)
    {
        chip->pg_serial |= (chip->pg_phase[29] & 0x3ff);
    }
}

static inline void OPM_KeyOn1(opm_t *chip)
{
    uint32_t cycles = (chip->cycles + 1) % 32;
    chip->kon_chanmatch = 0;
    if (chip->mode_kon_channel + 24 == cycles)
    {
        chip->kon_chanmatch = 1;
    }
}

static inline void OPM_KeyOn2(opm_t *chip)
{
    uint32_t slot = (chip->cycles + 8) % 32;
    if (chip->kon_chanmatch)
    {
        chip->mode_kon[(slot + 0) % 32] = chip->mode_kon_operator[0];
        chip->mode_kon[(slot + 8) % 32] = chip->mode_kon_operator[2];
        chip->mode_kon[(slot + 16) % 32] = chip->mode_kon_operator[1];
        chip->mode_kon[(slot + 24) % 32] = chip->mode_kon_operator[3];
    }
}

static inline void OPM_EnvelopePhase1(opm_t *chip)
{
    uint32_t slot = (chip->cycles + 2) % 32;
    uint32_t kon = chip->mode_kon[slot] | chip->kon_csm;
    uint32_t konevent = !chip->kon[slot] && kon;
    if (konevent)
    {
        chip->eg_state[slot] = eg_num_attack;
    }

    chip->kon2[slot] = chip->kon[slot];
    chip->kon[slot] = kon;
}

static inline void OPM_EnvelopePhase2(opm_t *chip)
{
    uint32_t slot = chip->cycles;
    uint32_t chan = slot % 8;
    uint8_t rate = 0, ksv, zr, ams;
    switch (chip->eg_state[slot])
    {
    case eg_num_attack:
        rate = chip->sl_ar[slot];
        break;
    case eg_num_decay:
        rate = chip->sl_d1r[slot];
        break;
    case eg_num_sustain:
        rate = chip->sl_d2r[slot];
        break;
    case eg_num_release:
        rate = chip->sl_rr[slot] * 2 + 1;
        break;
    default:
        break;
    }
    if (chip->ic)
    {
        rate = 31;
    }
    
    zr = rate == 0;

    ksv = chip->pg_kcode[slot] >> (chip->sl_ks[slot] ^ 3);
    if (chip->sl_ks[slot] == 0 && zr)
    {
        ksv &= ~3;
    }
    rate = rate * 2 + ksv;
    if (rate & 64)
    {
        rate = 63;
    }

    chip->eg_tl[2] = chip->eg_tl[1];
    chip->eg_tl[1] = chip->eg_tl[0];
    chip->eg_tl[0] = chip->sl_tl[slot];
    chip->eg_sl[1] = chip->eg_sl[0];
    chip->eg_sl[0] = chip->sl_d1l[slot];
    if (chip->sl_d1l[slot] == 15)
    {
        chip->eg_sl[0] = 31;
    }
    chip->eg_zr[1] = chip->eg_zr[0];
    chip->eg_zr[0] = zr;
    chip->eg_rate[1] = chip->eg_rate[0];
    chip->eg_rate[0] = rate;
    chip->eg_ratemax[1] = chip->eg_ratemax[0];
    chip->eg_ratemax[0] = (rate >> 1) == 31;
    ams = chip->sl_am_e[slot] ? chip->ch_ams[chan] : 0;
    switch (ams)
    {
    default:
    case 0:
        chip->eg_am = 0;
        break;
    case 1:
        chip->eg_am = chip->lfo_am_lock << 0;
        break;
    case 2:
        chip->eg_am = chip->lfo_am_lock << 1;
        break;
    case 3:
        chip->eg_am = chip->lfo_am_lock << 2;
        break;
    }
}

static inline void OPM_EnvelopePhase3(opm_t *chip)
{
    uint32_t slot = (chip->cycles + 31) % 32;
    chip->eg_shift = (chip->eg_timershift_lock + (chip->eg_rate[0] >> 2)) & 15;
    chip->eg_inchi = eg_stephi[chip->eg_rate[0] & 3][chip->eg_timer_lock & 3];

    chip->eg_outtemp[1] = chip->eg_outtemp[0];
    chip->eg_outtemp[0] = chip->eg_level[slot] + chip->eg_am;
    if (chip->eg_outtemp[0] & 1024)
    {
        chip->eg_outtemp[0] = 1023;
    }
}

static inline void OPM_EnvelopePhase4(opm_t *chip)
{
    uint32_t slot = (chip->cycles + 30) % 32;
    uint8_t inc = 0;
    uint8_t kon, eg_off, eg_zero, slreach;
    if (chip->eg_clock & 2)
    {
        if (chip->eg_rate[1] >= 48)
        {
            inc = chip->eg_inchi + (chip->eg_rate[1] >> 2) - 11;
            if (inc > 4)
            {
                inc = 4;
            }
        }
        else if (!chip->eg_zr[1])
        {
            switch (chip->eg_shift)
            {
            case 12:
                inc = chip->eg_rate[1] != 0;
                break;
            case 13:
                inc = (chip->eg_rate[1] >> 1) & 1;
                break;
            case 14:
                inc = chip->eg_rate[1] & 1;
                break;
            }
        }
    }
    chip->eg_inc = inc;

    kon = chip->kon[slot] && !chip->kon2[slot];
    chip->pg_reset[slot] = kon;
    chip->eg_instantattack = chip->eg_ratemax[1] && (kon || !chip->eg_ratemax[1]);

    eg_off = (chip->eg_level[slot] & 0x3f0) == 0x3f0;
    slreach = (chip->eg_level[slot] >> 4) == (chip->eg_sl[1] << 1);
    eg_zero = chip->eg_level[slot] == 0;

    chip->eg_mute = eg_off && chip->eg_state[slot] != eg_num_attack && !kon;
    chip->eg_inclinear = 0;
    if (!kon && !eg_off)
    {
        switch (chip->eg_state[slot])
        {
        case eg_num_decay:
            if (!slreach)
                chip->eg_inclinear = 1;
            break;
        case eg_num_sustain:
        case eg_num_release:
            chip->eg_inclinear = 1;
            break;
        }
    }
    chip->eg_incattack = chip->eg_state[slot] == eg_num_attack && !chip->eg_ratemax[1] && chip->kon[slot] && !eg_zero;


    // Update state
    if (kon)
    {
        chip->eg_state[slot] = eg_num_attack;
    }
    else if (!chip->kon[slot])
    {
        chip->eg_state[slot] = eg_num_release;
    }
    else
    {
        switch (chip->eg_state[slot])
        {
        case eg_num_attack:
            if (eg_zero)
            {
                chip->eg_state[slot] = eg_num_decay;
            }
            break;
        case eg_num_decay:
            if (eg_off)
            {
                chip->eg_state[slot] = eg_num_release;
            }
            else if (slreach)
            {
                chip->eg_state[slot] = eg_num_sustain;
            }
            break;
        case eg_num_sustain:
            if (eg_off)
            {
                chip->eg_state[slot] = eg_num_release;
            }
            break;
        case eg_num_release:
            break;
        }
    }

    if (chip->ic)
    {
        chip->eg_state[slot] = eg_num_release;
    }
}

static inline void OPM_EnvelopePhase5(opm_t *chip)
{
    uint32_t slot = (chip->cycles + 29) % 32;
    uint32_t level = chip->eg_level[slot];
    uint32_t step = 0;
    if (chip->eg_instantattack)
    {
        level = 0;
    }
    if (chip->eg_mute || chip->ic)
    {
        level = 0x3ff;
    }
    if (chip->eg_inc)
    {
        if (chip->eg_inclinear)
        {
            step |= 1 << (chip->eg_inc - 1);
        }
        if (chip->eg_incattack)
        {
            step |= ((~(int32_t)chip->eg_level[slot]) << chip->eg_inc) >> 5;
        }
    }
    level += step;
    chip->eg_level[slot] = (uint16_t)level;

    chip->eg_out[0] = chip->eg_outtemp[1] + (chip->eg_tl[2] << 3);
    if (chip->eg_out[0] & 1024)
    {
        chip->eg_out[0] = 1023;
    }

    if (chip->eg_test)
    {
        chip->eg_out[0] = 0;
    }

    chip->eg_test = chip->mode_test[5];
}

static inline void OPM_EnvelopePhase6(opm_t *chip)
{
    uint32_t slot = (chip->cycles + 28) % 32;
    chip->eg_serial_bit = (chip->eg_serial >> 9) & 1;
    if (chip->cycles == 3)
    {
        chip->eg_serial = chip->eg_out[0] ^ 1023;
    }
    else
    {
        chip->eg_serial <<= 1;
    }

    chip->eg_out[1] = chip->eg_out[0];
}

static void OPM_EnvelopeClock(opm_t *chip)
{
    chip->eg_clock <<= 1;
    if ((chip->eg_clockcnt & 2) != 0 || chip->mode_test[0])
    {
        chip->eg_clock |= 1;
    }
    if (chip->ic || (chip->cycles == 31 && (chip->eg_clockcnt & 2) != 0))
    {
        chip->eg_clockcnt = 0;
    }
    else if (chip->cycles == 31)
    {
        chip->eg_clockcnt++;
    }
}

static inline void OPM_EnvelopeTimer(opm_t *chip)
{
    uint32_t cycle = (chip->cycles + 31) % 16;
    uint32_t cycle2;
    uint8_t inc = ((chip->cycles + 31) % 32) < 16 && (chip->eg_clock & 1) != 0 && (cycle == 0 || chip->eg_timercarry);
    uint8_t timerbit = (chip->eg_timer >> cycle) & 1;
    uint8_t sum = timerbit + inc;
    uint8_t sum0 = (sum & 1) && !chip->ic;
    chip->eg_timercarry = sum >> 1;
    chip->eg_timer = (chip->eg_timer & (~(1 << cycle))) | (sum0 << cycle);

    cycle2 = (chip->cycles + 30) % 16;

    chip->eg_timer2 <<= 1;
    if ((chip->eg_timer & (1 << cycle2)) != 0 && !chip->eg_timerbstop)
    {
        chip->eg_timer2 |= 1;
    }

    if (chip->eg_timer & (1 << cycle2))
    {
        chip->eg_timerbstop = 1;
    }

    if (cycle == 0 || chip->ic2)
    {
        chip->eg_timerbstop = 0;
    }

    if (chip->cycles == 1 && (chip->eg_clock & 1) != 0)
    {
        chip->eg_timershift_lock = 0;
        if (chip->eg_timer2 & (8 + 32 + 128 + 512 + 2048 + 8192 + 32768))
        {
            chip->eg_timershift_lock |= 1;
        }
        if (chip->eg_timer2 & (4 + 32 + 64 + 512 + 1024 + 8192 + 16384))
        {
            chip->eg_timershift_lock |= 2;
        }
        if (chip->eg_timer2 & (4 + 8 + 16 + 512 + 1024 + 2048 + 4096))
        {
            chip->eg_timershift_lock |= 4;
        }
        if (chip->eg_timer2 & (4 + 8 + 16 + 32 + 64 + 128 + 256))
        {
            chip->eg_timershift_lock |= 8;
        }
        chip->eg_timer_lock = chip->eg_timer;
    }
}

static inline void OPM_OperatorPhase1(opm_t *chip)
{
    uint32_t slot = chip->cycles;
    int16_t mod = chip->op_mod[2];
    chip->op_phase_in = chip->pg_phase[slot] >> 10;
    if (chip->op_fbshift & 8)
    {
        if (chip->op_fb[1] == 0)
        {
            mod = 0;
        }
        else
        {
            mod = mod >> (9 - chip->op_fb[1]);
        }
    }
    chip->op_mod_in = mod;
}

static inline void OPM_OperatorPhase2(opm_t *chip)
{
    uint32_t slot = (chip->cycles + 31) % 32;
    chip->op_phase = (chip->op_phase_in + chip->op_mod_in) & 1023;
}

static inline void OPM_OperatorPhase3(opm_t *chip)
{
    uint32_t slot = (chip->cycles + 30) % 32;
    uint16_t phase = chip->op_phase & 255;
    if (chip->op_phase & 256)
    {
        phase ^= 255;
    }
    chip->op_logsin[0] = logsinrom[phase];
    chip->op_sign <<= 1;
    chip->op_sign |= (chip->op_phase >> 9) & 1;
}

static inline void OPM_OperatorPhase4(opm_t *chip)
{
    uint32_t slot = (chip->cycles + 29) % 32;
    chip->op_logsin[1] = chip->op_logsin[0];
}

static inline void OPM_OperatorPhase5(opm_t *chip)
{
    uint32_t slot = (chip->cycles + 28) % 32;
    chip->op_logsin[2] = chip->op_logsin[1];
}

static inline void OPM_OperatorPhase6(opm_t *chip)
{
    uint32_t slot = (chip->cycles + 27) % 32;
    chip->op_atten = chip->op_logsin[2] + (chip->eg_out[1] << 2);
    if (chip->op_atten & 4096)
    {
        chip->op_atten = 4095;
    }
}

static inline void OPM_OperatorPhase7(opm_t *chip)
{
    uint32_t slot = (chip->cycles + 26) % 32;
    chip->op_exp[0] = exprom[chip->op_atten & 255];
    chip->op_pow[0] = chip->op_atten >> 8;
}

static inline void OPM_OperatorPhase8(opm_t *chip)
{
    uint32_t slot = (chip->cycles + 25) % 32;
    chip->op_exp[1] = chip->op_exp[0];
    chip->op_pow[1] = chip->op_pow[0];
}

static inline void OPM_OperatorPhase9(opm_t *chip)
{
    uint32_t slot = (chip->cycles + 24) % 32;
    int16_t out = (chip->op_exp[1] << 2) >> (chip->op_pow[1]);
    if (chip->op_sign & 32)
    {
        out = -out;
    }
    chip->op_out[0] = out;
}

static inline void OPM_OperatorPhase10(opm_t *chip)
{
    uint32_t slot = (chip->cycles + 23) % 32;
    chip->op_out[1] = chip->op_out[0];
}

static inline void OPM_OperatorPhase11(opm_t *chip)
{
    uint32_t slot = (chip->cycles + 22) % 32;
    chip->op_out[2] = chip->op_out[1];
}

static inline void OPM_OperatorPhase12(opm_t *chip)
{
    uint32_t slot = (chip->cycles + 21) % 32;
    chip->op_out[3] = chip->op_out[2];
}

static inline void OPM_OperatorPhase13(opm_t *chip)
{
    uint32_t slot = (chip->cycles + 20) % 32;
    chip->op_out[4] = chip->op_out[3];
    chip->op_connect = chip->ch_connect[slot % 8];
}

static inline void OPM_OperatorPhase14(opm_t *chip)
{
    uint32_t slot = (chip->cycles + 19) % 32;
    chip->op_mix = chip->op_out[5] = chip->op_out[4];
    chip->op_fbupdate = (chip->op_counter == 0);
    chip->op_c1update = (chip->op_counter == 2);
    chip->op_fbshift <<= 1;
    chip->op_fbshift |= (chip->op_counter == 2);

    chip->op_modtable[0] = fm_algorithm[(chip->op_counter+2)%4][0][chip->op_connect];
    chip->op_modtable[1] = fm_algorithm[(chip->op_counter+2)%4][1][chip->op_connect];
    chip->op_modtable[2] = fm_algorithm[(chip->op_counter+2)%4][2][chip->op_connect];
    chip->op_modtable[3] = fm_algorithm[(chip->op_counter+2)%4][3][chip->op_connect];
    chip->op_modtable[4] = fm_algorithm[(chip->op_counter+2)%4][4][chip->op_connect];
    chip->op_mixl = fm_algorithm[chip->op_counter][5][chip->op_connect] && (chip->ch_rl[slot % 8] & 1) != 0;
    chip->op_mixr = fm_algorithm[chip->op_counter][5][chip->op_connect] && (chip->ch_rl[slot % 8] & 2) != 0;
}

static inline void OPM_OperatorPhase15(opm_t *chip)
{
    uint32_t slot = (chip->cycles + 18) % 32;
    int16_t mod, mod1 = 0, mod2 = 0;
    if (chip->op_modtable[0])
    {
        mod2 |= chip->op_m1[slot % 8][0];
    }
    if (chip->op_modtable[1])
    {
        mod1 |= chip->op_m1[slot % 8][1];
    }
    if (chip->op_modtable[2])
    {
        mod1 |= chip->op_c1[slot % 8];
    }
    if (chip->op_modtable[3])
    {
        mod2 |= chip->op_out[5];
    }
    if (chip->op_modtable[4])
    {
        mod1 |= chip->op_out[5];
    }
    mod = (mod1 + mod2) >> 1;
    chip->op_mod[0] = mod;
    if (chip->op_fbupdate)
    {
        chip->op_m1[slot % 8][1] = chip->op_m1[slot % 8][0];
        chip->op_m1[slot % 8][0] = chip->op_out[5];
    }
    if (chip->op_c1update)
    {
        chip->op_c1[slot % 8] = chip->op_out[5];
    }
}

static inline void OPM_OperatorPhase16(opm_t *chip)
{
    uint32_t slot = (chip->cycles + 17) % 32;
    // hack
    chip->op_mod[2] = chip->op_mod[1];
    chip->op_fb[1] = chip->op_fb[0];

    chip->op_mod[1] = chip->op_mod[0];
    chip->op_fb[0] = chip->ch_fb[slot % 8];
}

static inline void OPM_OperatorCounter(opm_t *chip)
{
    if ((chip->cycles % 8) == 4)
    {
        chip->op_counter++;
    }
    if (chip->cycles == 12)
    {
        chip->op_counter = 0;
    }
}

static inline void OPM_Mixer2(opm_t *chip)
{
    uint32_t cycles = (chip->cycles + 30) % 32;
    uint8_t bit;
    uint8_t top, ex;
    if (cycles < 16)
    {
        bit = chip->mix_serial[0] & 1;
    }
    else
    {
        bit = chip->mix_serial[1] & 1;
    }
    if (chip->cycles % 16 == 1)
    {
        chip->mix_sign_lock = bit ^ 1;
        chip->mix_top_bits_lock = (chip->mix_bits >> 15) & 63;
    }
    chip->mix_bits >>= 1;
    chip->mix_bits |= bit << 20;
    if (chip->cycles % 16 == 10)
    {
        top = chip->mix_top_bits_lock;
        if (chip->mix_sign_lock)
        {
            top ^= 63;
        }
        if (top & 32)
        {
            ex = 7;
        }
        else if (top & 16)
        {
            ex = 6;
        }
        else if (top & 8)
        {
            ex = 5;
        }
        else if (top & 4)
        {
            ex = 4;
        }
        else if (top & 2)
        {
            ex = 3;
        }
        else if (top & 1)
        {
            ex = 2;
        }
        else
        {
            ex = 1;
        }
        chip->mix_sign_lock2 = chip->mix_sign_lock;
        chip->mix_exp_lock = ex;
    }
    chip->mix_out_bit <<= 1;
    switch ((chip->cycles + 1) % 16)
    {
    case 0:
        chip->mix_out_bit |= chip->mix_sign_lock2 ^ 1;
        break;
    case 1:
        chip->mix_out_bit |= (chip->mix_exp_lock >> 0) & 1;
        break;
    case 2:
        chip->mix_out_bit |= (chip->mix_exp_lock >> 1) & 1;
        break;
    case 3:
        chip->mix_out_bit |= (chip->mix_exp_lock >> 2) & 1;
        break;
    default:
        if (chip->mix_exp_lock)
        {
            chip->mix_out_bit |= (chip->mix_bits >> (chip->mix_exp_lock - 1)) & 1;
        }
        break;
    }
}

static inline void OPM_Output(opm_t *chip)
{
    uint32_t slot = (chip->cycles + 27) % 32;
    chip->smp_so = (chip->mix_out_bit & 4) != 0;
    chip->smp_sh1 = (slot & 24) == 8 && !chip->ic;
    chip->smp_sh2 = (slot & 24) == 24 && !chip->ic;
}

static inline void OPM_DAC(opm_t *chip)
{
    int32_t exp, mant;
    if (chip->dac_osh1 && !chip->smp_sh1)
    {
        exp = (chip->dac_bits >> 10) & 7;
        mant = (chip->dac_bits >> 0) & 1023;
        mant -= 512;
        chip->dac_output[1] = (mant << exp) >> 1;
    }
    if (chip->dac_osh2 && !chip->smp_sh2)
    {
        exp = (chip->dac_bits >> 10) & 7;
        mant = (chip->dac_bits >> 0) & 1023;
        mant -= 512;
        chip->dac_output[0] = (mant << exp) >> 1;
    }
    chip->dac_bits >>= 1;
    chip->dac_bits |= chip->smp_so << 12;
    chip->dac_osh1 = chip->smp_sh1;
    chip->dac_osh2 = chip->smp_sh2;
}

static inline void OPM_Mixer(opm_t *chip)
{
    uint32_t slot = (chip->cycles + 18) % 32;
    uint32_t channel = (slot % 8);
    // Right channel
    chip->mix_serial[1] >>= 1;
    if (chip->cycles == 13)
    {
        chip->mix_serial[1] |= (chip->mix[1] & 1023) << 4;
    }
    if (chip->cycles == 14)
    {
        chip->mix_serial[1] |= ((chip->mix2[1] >> 10) & 31) << 13;
        chip->mix_serial[1] |= (((chip->mix2[1] >> 17) & 1) ^ 1) << 18;
        chip->mix_clamp_low[1] = 0;
        chip->mix_clamp_high[1] = 0;
        switch ((chip->mix2[1]>>15) & 7)
        {
        case 0:
        default:
            break;
        case 1:
            chip->mix_clamp_high[1] = 1;
            break;
        case 2:
            chip->mix_clamp_high[1] = 1;
            break;
        case 3:
            chip->mix_clamp_high[1] = 1;
            break;
        case 4:
            chip->mix_clamp_low[1] = 1;
            break;
        case 5:
            chip->mix_clamp_low[1] = 1;
            break;
        case 6:
            chip->mix_clamp_low[1] = 1;
            break;
        case 7:
            break;
        }
    }
    if (chip->mix_clamp_low[1])
    {
        chip->mix_serial[1] &= ~2;
    }
    if (chip->mix_clamp_high[1])
    {
        chip->mix_serial[1] |= 2;
    }
    // Left channel
    chip->mix_serial[0] >>= 1;
    if (chip->cycles == 29)
    {
        chip->mix_serial[0] |= (chip->mix[0] & 1023) << 4;
    }
    if (chip->cycles == 30)
    {
        chip->mix_serial[0] |= ((chip->mix2[0] >> 10) & 31) << 13;
        chip->mix_serial[0] |= (((chip->mix2[0] >> 17) & 1) ^ 1) << 18;
        chip->mix_clamp_low[0] = 0;
        chip->mix_clamp_high[0] = 0;
        switch ((chip->mix2[0]>>15) & 7)
        {
        case 0:
        default:
            break;
        case 1:
            chip->mix_clamp_high[0] = 1;
            break;
        case 2:
            chip->mix_clamp_high[0] = 1;
            break;
        case 3:
            chip->mix_clamp_high[0] = 1;
            break;
        case 4:
            chip->mix_clamp_low[0] = 1;
            break;
        case 5:
            chip->mix_clamp_low[0] = 1;
            break;
        case 6:
            chip->mix_clamp_low[0] = 1;
            break;
        case 7:
            break;
        }
    }
    if (chip->mix_clamp_low[0])
    {
        chip->mix_serial[0] &= ~2;
    }
    if (chip->mix_clamp_high[0])
    {
        chip->mix_serial[0] |= 2;
    }
    chip->mix2[0] = chip->mix[0];
    chip->mix2[1] = chip->mix[1];
    if (chip->cycles == 13)
    {
        chip->mix[1] = 0;
    }
    if (chip->cycles == 29)
    {
        chip->mix[0] = 0;
    }
    chip->mix[0] += chip->op_mix * chip->op_mixl;
    chip->mix[1] += chip->op_mix * chip->op_mixr;

    if (slot<8) {
      chip->op_chmix[slot&7]=0;
    }
    chip->op_chmix[slot&7]+=chip->op_mix*(chip->op_mixl|chip->op_mixr);
    if (slot>=24) {
      chip->ch_out[slot&7]=chip->op_chmix[slot&7];
    }
}

static inline void OPM_Noise(opm_t *chip)
{
    uint8_t w1 = !chip->ic && !chip->noise_update;
    uint8_t xr = ((chip->noise_lfsr >> 2) & 1) ^ chip->noise_temp;
    uint8_t w2t = (chip->noise_lfsr & 0xffff) == 0xffff && chip->noise_temp == 0;
    uint8_t w2 = !w2t && !xr;
    uint8_t w3 = !chip->ic && !w1 && !w2;
    uint8_t w4 = ((chip->noise_lfsr & 1) == 0 || !w1) && !w3;
    if (!w1)
    {
        chip->noise_temp = (chip->noise_lfsr & 1) == 0;
    }
    chip->noise_lfsr >>= 1;
    chip->noise_lfsr |= w4 << 15;
}

static inline void OPM_NoiseTimer(opm_t *chip)
{
    uint32_t timer = chip->noise_timer;

    chip->noise_update = chip->noise_timer_of;

    if (chip->cycles % 16 == 15)
    {
        timer++;
        timer &= 31;
    }
    if (chip->ic || (chip->noise_timer_of && (chip->cycles % 16 == 15)))
    {
        timer = 0;
    }

    chip->noise_timer_of = chip->noise_timer == (chip->noise_freq ^ 31);
    chip->noise_timer = timer;
}

static inline void OPM_DoTimerA(opm_t *chip)
{
    uint16_t value = chip->timer_a_val;
    value += chip->timer_a_inc;
    chip->timer_a_of = (value >> 10) & 1;
    if (chip->timer_a_do_reset)
    {
        value = 0;
    }
    if (chip->timer_a_do_load)
    {
        value = chip->timer_a_reg;
    }

    chip->timer_a_val = value & 1023;
}

static inline void OPM_DoTimerA2(opm_t *chip)
{
    if (chip->cycles == 1)
    {
        chip->timer_a_load = chip->timer_loada;
    }
    chip->timer_a_inc = chip->mode_test[2] || (chip->timer_a_load && chip->cycles == 0);
    chip->timer_a_do_load = chip->timer_a_of || (chip->timer_a_load && chip->timer_a_temp);
    chip->timer_a_do_reset = chip->timer_a_temp;
    chip->timer_a_temp = !chip->timer_a_load;
    if (chip->timer_reseta || chip->ic)
    {
        chip->timer_a_status = 0;
    }
    else
    {
        chip->timer_a_status |= chip->timer_irqa && chip->timer_a_of;
    }
    chip->timer_reseta = 0;
}

static inline void OPM_DoTimerB(opm_t *chip)
{
    uint16_t value = chip->timer_b_val;
    value += chip->timer_b_inc;
    chip->timer_b_of = (value >> 8) & 1;
    if (chip->timer_b_do_reset)
    {
        value = 0;
    }
    if (chip->timer_b_do_load)
    {
        value = chip->timer_b_reg;
    }

    chip->timer_b_val = value & 255;

    if (chip->cycles == 0)
    {
        chip->timer_b_sub++;
    }

    chip->timer_b_sub_of = (chip->timer_b_sub >> 4) & 1;
    chip->timer_b_sub &= 15;
    if (chip->ic)
    {
        chip->timer_b_sub = 0;
    }
}

static inline void OPM_DoTimerB2(opm_t *chip)
{
    chip->timer_b_inc = chip->mode_test[2] || (chip->timer_loadb && chip->timer_b_sub_of);
    chip->timer_b_do_load = chip->timer_b_of || (chip->timer_loadb && chip->timer_b_temp);
    chip->timer_b_do_reset = chip->timer_b_temp;
    chip->timer_b_temp = !chip->timer_loadb;
    if (chip->timer_resetb || chip->ic)
    {
        chip->timer_b_status = 0;
    }
    else
    {
        chip->timer_b_status |= chip->timer_irqb && chip->timer_b_of;
    }
    chip->timer_resetb = 0;
}

static inline void OPM_DoTimerIRQ(opm_t *chip)
{
    chip->timer_irq = chip->timer_a_status || chip->timer_b_status;
}

static inline void OPM_DoLFOMult(opm_t *chip)
{
    uint8_t ampm_sel = (chip->lfo_bit_counter & 8) != 0;
    uint8_t dp = ampm_sel ? chip->lfo_pmd : chip->lfo_amd;
    uint8_t bit = 0, b1, b2;
    uint8_t sum;

    chip->lfo_out2_b = chip->lfo_out2;

    switch (chip->lfo_bit_counter & 7)
    {
    case 0:
        bit = (dp & 64) != 0 && (chip->lfo_out1 & 64) == 0;
        break;
    case 1:
        bit = (dp & 32) != 0 && (chip->lfo_out1 & 32) == 0;
        break;
    case 2:
        bit = (dp & 16) != 0 && (chip->lfo_out1 & 16) == 0;
        break;
    case 3:
        bit = (dp & 8) != 0 && (chip->lfo_out1 & 8) == 0;
        break;
    case 4:
        bit = (dp & 4) != 0 && (chip->lfo_out1 & 4) == 0;
        break;
    case 5:
        bit = (dp & 2) != 0 && (chip->lfo_out1 & 2) == 0;
        break;
    case 6:
        bit = (dp & 1) != 0 && (chip->lfo_out1 & 1) == 0;
        break;
    }

    b1 = (chip->lfo_out2 & 1) != 0;
    if ((chip->lfo_bit_counter & 7) == 0)
    {
        b1 = 0;
    }
    b2 = chip->lfo_mult_carry;
    if (chip->cycles % 16 == 15)
    {
        b2 = 0;
    }
    sum = bit + b1 + b2;
    chip->lfo_out2 >>= 1;
    chip->lfo_out2 |= (sum & 1) << 15;
    chip->lfo_mult_carry = sum >> 1;
}

static inline void OPM_DoLFO1(opm_t *chip)
{
    uint16_t counter2 = chip->lfo_counter2;
    uint8_t of_old = chip->lfo_counter2_of;
    uint8_t lfo_bit, noise, sum, carry, w[10];
    uint8_t lfo_pm_sign;
    uint8_t ampm_sel = (chip->lfo_bit_counter & 8) != 0;
    counter2 += (chip->lfo_counter1_of1 & 2) != 0 || chip->mode_test[3];
    chip->lfo_counter2_of = (counter2 >> 15) & 1;
    if (chip->ic)
    {
        counter2 = 0;
    }
    if (chip->lfo_counter2_load)
    {
        counter2 = lfo_counter2_table[chip->lfo_freq_hi];
    }
    chip->lfo_counter2 = counter2 & 32767;
    chip->lfo_counter2_load = chip->lfo_frq_update || of_old;
    chip->lfo_frq_update = 0;
    if ((chip->cycles % 16) == 12)
    {
        chip->lfo_counter1++;
    }
    chip->lfo_counter1_of1 <<= 1;
    chip->lfo_counter1_of1 |= (chip->lfo_counter1 >> 4) & 1;
    chip->lfo_counter1 &= 15;
    if (chip->ic)
    {
        chip->lfo_counter1 = 0;
    }

    if ((chip->cycles & 15) == 5)
    {
        chip->lfo_counter2_of_lock2 = chip->lfo_counter2_of_lock;
    }

    chip->lfo_counter3 += chip->lfo_counter3_clock;
    if (chip->ic)
    {
        chip->lfo_counter3 = 0;
    }

    chip->lfo_counter3_clock = (chip->cycles & 15) == 13 && chip->lfo_counter2_of_lock2;

    if ((chip->cycles & 15) == 15)
    {
        chip->lfo_trig_sign = (chip->lfo_val & 0x80) != 0;
        chip->lfo_saw_sign = (chip->lfo_val & 0x100) != 0;
    }

    lfo_pm_sign = chip->lfo_wave == 2 ? chip->lfo_trig_sign : chip->lfo_saw_sign;

    w[5] = ampm_sel ? chip->lfo_saw_sign : (chip->lfo_wave != 2 || !chip->lfo_trig_sign);

    w[1] = !chip->lfo_clock || chip->lfo_wave == 3 || (chip->cycles & 15) != 15;
    w[2] = chip->lfo_wave == 2 && !w[1];
    w[4] = chip->lfo_clock_lock && chip->lfo_wave == 3;
    w[3] = !chip->ic && !chip->mode_test[1] && !w[4] && (chip->lfo_val & 0x8000) != 0;

    w[7] = ((chip->cycles + 1) % 16) < 8;

    w[6] = w[5] ^ w[3];
    
    w[9] = ampm_sel ? ((chip->cycles % 16) == 6) : !chip->lfo_saw_sign;

    w[8] = chip->lfo_wave == 1 ? w[9] : w[6];

    w[8] &= w[7];

    chip->lfo_out1 <<= 1;
    chip->lfo_out1 |= !w[8];

    carry = !w[1] || ((chip->cycles & 15) != 15 && chip->lfo_val_carry != 0 && chip->lfo_wave != 3);
    sum = carry + w[2] + w[3];
    noise = chip->lfo_clock_lock && (chip->noise_lfsr & 1) != 0;
    lfo_bit = sum & 1;
    lfo_bit |= (chip->lfo_wave == 3) && noise;
    chip->lfo_val_carry = sum >> 1;
    chip->lfo_val <<= 1;
    chip->lfo_val |= lfo_bit;
    

    if (chip->cycles % 16 == 15 && (chip->lfo_bit_counter & 7) == 7)
    {
        if (ampm_sel)
        {
            chip->lfo_pm_lock = (chip->lfo_out2_b >> 8) & 255;
            chip->lfo_pm_lock ^= lfo_pm_sign << 7;
        }
        else
        {
            chip->lfo_am_lock = (chip->lfo_out2_b >> 8) & 255;
        }
    }

    if ((chip->cycles & 15) == 14)
    {
        chip->lfo_bit_counter++;
    }
    if ((chip->cycles & 15) != 12 && chip->lfo_counter1_of2)
    {
        chip->lfo_bit_counter = 0;
    }
    chip->lfo_counter1_of2 = chip->lfo_counter1 == 2;
}

static inline void OPM_DoLFO2(opm_t *chip)
{
    chip->lfo_clock_test = chip->lfo_clock;
    chip->lfo_clock = (chip->lfo_counter2_of || chip->lfo_test || chip->lfo_counter3_step);
    if ((chip->cycles & 15) == 14)
    {
        chip->lfo_counter2_of_lock = chip->lfo_counter2_of;
        chip->lfo_clock_lock = chip->lfo_clock;
    }
    chip->lfo_counter3_step = 0;
    if (chip->lfo_counter3_clock)
    {
        if ((chip->lfo_counter3 & 1) == 0)
        {
            chip->lfo_counter3_step = (chip->lfo_freq_lo & 8) != 0;
        }
        else if ((chip->lfo_counter3 & 2) == 0)
        {
            chip->lfo_counter3_step = (chip->lfo_freq_lo & 4) != 0;
        }
        else if ((chip->lfo_counter3 & 4) == 0)
        {
            chip->lfo_counter3_step = (chip->lfo_freq_lo & 2) != 0;
        }
        else if ((chip->lfo_counter3 & 8) == 0)
        {
            chip->lfo_counter3_step = (chip->lfo_freq_lo & 1) != 0;
        }
    }
    chip->lfo_test = chip->mode_test[2];
}

static inline void OPM_CSM(opm_t *chip)
{
    chip->kon_csm = chip->kon_csm_lock;
    if (chip->cycles == 1)
    {
        chip->kon_csm_lock = chip->timer_a_do_load && chip->mode_csm;
    }
}

static inline void OPM_NoiseChannel(opm_t *chip)
{
    chip->nc_active |= chip->eg_serial_bit & 1;
    if (chip->cycles == 13)
    {
        chip->nc_active = 0;
    }
    chip->nc_out <<= 1;
    chip->nc_out |= chip->nc_sign ^ chip->eg_serial_bit;
    chip->nc_sign = !chip->nc_sign_lock;
    if (chip->cycles == 12)
    {
        chip->nc_active_lock = chip->nc_active;
        chip->nc_sign_lock2 = chip->nc_active_lock && !chip->nc_sign_lock;
        chip->nc_sign_lock = (chip->noise_lfsr & 1);

        if (chip->noise_en)
        {
            if (chip->nc_sign_lock2)
            {
                chip->op_mix = ((chip->nc_out & ~1) << 2) | -4089;
            }
            else
            {
                chip->op_mix = ((chip->nc_out & ~1) << 2);
            }
        }
    }
}

static inline void OPM_DoIO(opm_t *chip)
{
    // Busy
    chip->write_busy_cnt += chip->write_busy;
    chip->write_busy = (!(chip->write_busy_cnt >> 5) && chip->write_busy && !chip->ic) | chip->write_d_en;
    chip->write_busy_cnt &= 0x1f;
    if (chip->ic)
    {
        chip->write_busy_cnt = 0;
    }
    // Write signal check
    chip->write_a_en = chip->write_a;
    chip->write_d_en = chip->write_d;
    chip->write_a = 0;
    chip->write_d = 0;
}

static inline void OPM_DoRegWrite(opm_t *chip)
{
    int32_t i;
    uint32_t channel = chip->cycles % 8;
    uint32_t slot = chip->cycles;

    // Register write
    if (chip->reg_data_ready)
    {
        // Channel
        if ((chip->reg_address & 0xe7) == (0x20 | channel))
        {
            switch (chip->reg_address & 0x18)
            {
            case 0x00: // RL, FB, CONNECT
                chip->ch_rl[channel] = chip->reg_data >> 6;
                chip->ch_fb[channel] = (chip->reg_data >> 3) & 0x07;
                chip->ch_connect[channel] = chip->reg_data & 0x07;
                break;
            case 0x08: // KC
                chip->ch_kc[channel] = chip->reg_data & 0x7f;
                break;
            case 0x10: // KF
                chip->ch_kf[channel] = chip->reg_data >> 2;
                break;
            case 0x18: // PMS, AMS
                chip->ch_pms[channel] = (chip->reg_data >> 4) & 0x07;
                chip->ch_ams[channel] = chip->reg_data & 0x03;
                break;
            default:
                break;
            }
        }
        // Slot
        if ((chip->reg_address & 0x1f) == slot)
        {
            switch (chip->reg_address & 0xe0)
            {
            case 0x40: // DT1, MUL
                chip->sl_dt1[slot] = (chip->reg_data >> 4) & 0x07;
                chip->sl_mul[slot] = chip->reg_data & 0x0f;
                break;
            case 0x60: // TL
                chip->sl_tl[slot] = chip->reg_data & 0x7f;
                break;
            case 0x80: // KS, AR
                chip->sl_ks[slot] = chip->reg_data >> 6;
                chip->sl_ar[slot] = chip->reg_data & 0x1f;
                break;
            case 0xa0: // AMS-EN, D1R
                chip->sl_am_e[slot] = chip->reg_data >> 7;
                chip->sl_d1r[slot] = chip->reg_data & 0x1f;
                break;
            case 0xc0: // DT2, D2R
                chip->sl_dt2[slot] = chip->reg_data >> 6;
                chip->sl_d2r[slot] = chip->reg_data & 0x1f;
                break;
            case 0xe0: // D1L, RR
                chip->sl_d1l[slot] = chip->reg_data >> 4;
                chip->sl_rr[slot] = chip->reg_data & 0x0f;
                break;
            default:
                break;
            }
        }
    }

    // Mode write
    if (chip->write_d_en)
    {
        switch (chip->mode_address)
        {
        case 0x01:
            for (i = 0; i < 8; i++)
            {
                chip->mode_test[i] = (chip->write_data >> i) & 0x01;
            }
            break;
        case 0x08:
            for (i = 0; i < 4; i++)
            {
                chip->mode_kon_operator[i] = (chip->write_data >> (i + 3)) & 0x01;
            }
            chip->mode_kon_channel = chip->write_data & 0x07;
            break;
        case 0x0f:
            chip->noise_en = chip->write_data >> 7;
            chip->noise_freq = chip->write_data & 0x1f;
            break;
        case 0x10:
            chip->timer_a_reg &= 0x03;
            chip->timer_a_reg |= chip->write_data << 2;
            break;
        case 0x11:
            chip->timer_a_reg &= 0x3fc;
            chip->timer_a_reg |= chip->write_data & 0x03;
            break;
        case 0x12:
            chip->timer_b_reg = chip->write_data;
            break;
        case 0x14:
            chip->mode_csm = (chip->write_data >> 7) & 1;
            chip->timer_irqb = (chip->write_data >> 3) & 1;
            chip->timer_irqa = (chip->write_data >> 2) & 1;
            chip->timer_resetb = (chip->write_data >> 5) & 1;
            chip->timer_reseta = (chip->write_data >> 4) & 1;
            chip->timer_loadb = (chip->write_data >> 1) & 1;
            chip->timer_loada = (chip->write_data >> 0) & 1;
            break;
        case 0x18:
            chip->lfo_freq_hi = chip->write_data >> 4;
            chip->lfo_freq_lo = chip->write_data & 0x0f;
            chip->lfo_frq_update = 1;
            break;
        case 0x19:
            if (chip->write_data & 0x80)
            {
                chip->lfo_pmd = chip->write_data & 0x7f;
            }
            else
            {
                chip->lfo_amd = chip->write_data;
            }
            break;
        case 0x1b:
            chip->lfo_wave = chip->write_data & 0x03;
            chip->io_ct1 = (chip->write_data >> 6) & 0x01;
            chip->io_ct2 = chip->write_data >> 7;
            break;
        }
    }

    // Register data write
    chip->reg_data_ready = chip->reg_data_ready && !chip->write_a_en;
    if (chip->reg_address_ready && chip->write_d_en)
    {
        chip->reg_data = chip->write_data;
        chip->reg_data_ready = 1;
    }

    // Register address write
    chip->reg_address_ready = chip->reg_address_ready && !chip->write_a_en;
    if (chip->write_a_en && (chip->write_data & 0xe0) != 0)
    {
        chip->reg_address = chip->write_data;
        chip->reg_address_ready = 1;
    }
    if (chip->write_a_en)
    {
        chip->mode_address = chip->write_data;
    }
}

static inline void OPM_DoIC(opm_t *chip)
{
    uint32_t channel = chip->cycles % 8;
    uint32_t slot = chip->cycles;
    if (chip->ic)
    {
        chip->ch_rl[channel] = 0;
        chip->ch_fb[channel] = 0;
        chip->ch_connect[channel] = 0;
        chip->ch_kc[channel] = 0;
        chip->ch_kf[channel] = 0;
        chip->ch_pms[channel] = 0;
        chip->ch_ams[channel] = 0;

        chip->sl_dt1[slot] = 0;
        chip->sl_mul[slot] = 0;
        chip->sl_tl[slot] = 0;
        chip->sl_ks[slot] = 0;
        chip->sl_ar[slot] = 0;
        chip->sl_am_e[slot] = 0;
        chip->sl_d1r[slot] = 0;
        chip->sl_dt2[slot] = 0;
        chip->sl_d2r[slot] = 0;
        chip->sl_d1l[slot] = 0;
        chip->sl_rr[slot] = 0;

        chip->timer_a_reg = 0;
        chip->timer_b_reg = 0;
        chip->timer_irqa = 0;
        chip->timer_irqb = 0;
        chip->timer_loada = 0;
        chip->timer_loadb = 0;
        chip->mode_csm = 0;

        chip->mode_test[0] = 0;
        chip->mode_test[1] = 0;
        chip->mode_test[2] = 0;
        chip->mode_test[3] = 0;
        chip->mode_test[4] = 0;
        chip->mode_test[5] = 0;
        chip->mode_test[6] = 0;
        chip->mode_test[7] = 0;
        chip->noise_en = 0;
        chip->noise_freq = 0;

        chip->mode_kon_channel = 0;
        chip->mode_kon_operator[0] = 0;
        chip->mode_kon_operator[1] = 0;
        chip->mode_kon_operator[2] = 0;
        chip->mode_kon_operator[3] = 0;
        chip->mode_kon[(slot + 8) % 32] = 0;

        chip->lfo_pmd = 0;
        chip->lfo_amd = 0;
        chip->lfo_wave = 0;
        chip->lfo_freq_hi = 0;
        chip->lfo_freq_lo = 0;

        chip->io_ct1 = 0;
        chip->io_ct2 = 0;

        chip->reg_address = 0;
        chip->reg_data = 0;
    }
    chip->ic2 = chip->ic;
}

void OPM_Clock(opm_t *chip, int32_t *output, uint8_t *sh1, uint8_t *sh2, uint8_t *so)
{
    OPM_Mixer2(chip);
    OPM_Mixer(chip);

    OPM_OperatorPhase16(chip);
    OPM_OperatorPhase15(chip);
    OPM_OperatorPhase14(chip);
    OPM_OperatorPhase13(chip);
    OPM_OperatorPhase12(chip);
    OPM_OperatorPhase11(chip);
    OPM_OperatorPhase10(chip);
    OPM_OperatorPhase9(chip);
    OPM_OperatorPhase8(chip);
    OPM_OperatorPhase7(chip);
    OPM_OperatorPhase6(chip);
    OPM_OperatorPhase5(chip);
    OPM_OperatorPhase4(chip);
    OPM_OperatorPhase3(chip);
    OPM_OperatorPhase2(chip);
    OPM_OperatorPhase1(chip);
    OPM_OperatorCounter(chip);

    OPM_EnvelopeTimer(chip);
    OPM_EnvelopePhase6(chip);
    OPM_EnvelopePhase5(chip);
    OPM_EnvelopePhase4(chip);
    OPM_EnvelopePhase3(chip);
    OPM_EnvelopePhase2(chip);
    OPM_EnvelopePhase1(chip);

    OPM_PhaseDebug(chip);
    OPM_PhaseGenerate(chip);
    OPM_PhaseCalcIncrement(chip);
    OPM_PhaseCalcFNumBlock(chip);

    OPM_DoTimerIRQ(chip);
    OPM_DoTimerA(chip);
    OPM_DoTimerB(chip);
    OPM_DoLFOMult(chip);
    OPM_DoLFO1(chip);
    OPM_Noise(chip);
    OPM_KeyOn2(chip);
    OPM_DoRegWrite(chip);
    OPM_EnvelopeClock(chip);
    OPM_NoiseTimer(chip);
    OPM_KeyOn1(chip);
    OPM_DoIO(chip);
    OPM_DoTimerA2(chip);
    OPM_DoTimerB2(chip);
    OPM_DoLFO2(chip);
    OPM_CSM(chip);
    OPM_NoiseChannel(chip);
    OPM_Output(chip);
    OPM_DAC(chip);
    OPM_DoIC(chip);
    if (sh1)
    {
        *sh1 = chip->smp_sh1;
    }
    if (sh2)
    {
        *sh2 = chip->smp_sh2;
    }
    if (so)
    {
        *so = chip->smp_so;
    }
    if (output)
    {
        output[0] = chip->dac_output[0];
        output[1] = chip->dac_output[1];
    }
    chip->cycles = (chip->cycles + 1) % 32;
}

void OPM_Write(opm_t *chip, uint32_t port, uint8_t data)
{
    chip->write_data = data;
    if (chip->ic)
    {
        return;
    }
    if (port & 0x01)
    {
        chip->write_d = 1;
    }
    else
    {
        chip->write_a = 1;
    }
}

uint8_t OPM_Read(opm_t *chip, uint32_t port)
{
    uint16_t testdata;
    if (chip->mode_test[6])
    {
        testdata = chip->op_out[5] | ((chip->eg_serial_bit ^ 1) << 14) | ((chip->pg_serial & 1) << 15);
        if (chip->mode_test[7])
        {
            return testdata & 255;
        }
        else
        {
            return testdata >> 8;
        }
    }
    return (chip->write_busy << 7) | (chip->timer_b_status << 1) | chip->timer_a_status;
}

uint8_t OPM_ReadIRQ(opm_t *chip)
{
    return chip->timer_irq;
}

uint8_t OPM_ReadCT1(opm_t *chip)
{
    if(chip->mode_test[3])
    {
        return chip->lfo_clock_test;
    }
    return chip->io_ct1;
}

uint8_t OPM_ReadCT2(opm_t *chip)
{
    return chip->io_ct2;
}

void OPM_SetIC(opm_t *chip, uint8_t ic)
{
    if (chip->ic != ic)
    {
        chip->ic = ic;
        if (!ic)
        {
            chip->cycles = 0;
        }
    }
}

void OPM_Reset(opm_t *chip)
{
    uint32_t i;
    memset(chip, 0, sizeof(opm_t));
    OPM_SetIC(chip, 1);
    for (i = 0; i < 32 * 64; i++)
    {
        OPM_Clock(chip, NULL, NULL, NULL, NULL);
    }
    OPM_SetIC(chip, 0);
}
