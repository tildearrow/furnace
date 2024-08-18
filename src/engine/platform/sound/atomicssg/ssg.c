/*
 * Copyright (C) 2023-2024 nukeykt
 * Copyright (C) 2024 tildearrow
 *
 * This file is part of AtomicSSG.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *  SSG (AY-3-8910/YM2149) emulator.
 *  based on YM2608-LLE by nukeykt.
 *
 */
#include <string.h>
#include "ssg.h"

void SSG_Clock(ssg_t* chip, int clk)
{
    int i;

    chip->input.clk = clk;

    chip->clk1 = !chip->input.clk;
    chip->clk2 = chip->input.clk;

    chip->ic = !chip->input.ic;

    {
        int read = !chip->ic && !chip->input.rd && !chip->input.cs;
        chip->read0 = !chip->ic && !chip->input.rd && !chip->input.cs;
        int write = !chip->input.wr && !chip->input.cs;
        int writeaddr = chip->ic || (!chip->input.wr && !chip->input.cs);
        int writedata = !chip->ic && !chip->input.wr && !chip->input.cs;
        int read1 = !chip->ic && !chip->input.rd && !chip->input.cs;
        chip->ssg_write0 = writeaddr;
        chip->ssg_write1 = writedata || chip->ic;
        chip->ssg_read1 = read1;

        chip->o_data_d = !read;

        if (write)
            chip->data_l = (chip->input.data & 255);

        if (chip->ic)
            chip->data_bus2 |= 0x2f;
        else
        if (!read && !chip->ic)
            chip->data_bus2 = chip->data_l ^ 0x1ff;

        if (chip->ic_latch_fm[1])
        {
            chip->data_bus1 &= ~255;
        }
        else if (!read && !chip->ic)
        {
            chip->data_bus1 = chip->data_l;
        }
    }

    {
        chip->ssg_clk1 = chip->clk1;
        chip->ssg_clk2 = chip->clk2;

        if (chip->ic)
            chip->ssg_ssg_addr = 0;
        else if (chip->ssg_write0)
            chip->ssg_ssg_addr = 1;

        if (chip->ic)
            chip->ssg_address = 0;
        else if (chip->ssg_write0)
            chip->ssg_address = chip->input.a0 & 0x1f;
        
        int ssg_access = chip->ssg_ssg_addr && (chip->ssg_write1 || chip->ssg_read1);

        if (chip->ssg_egtrig_rst)
            chip->ssg_egtrig = 0;

        if (chip->ic)
        {
            chip->ssg_freq_a[0] = chip->data_bus1 & 255;
            chip->ssg_freq_a[1] = chip->data_bus1 & 15;
            chip->ssg_freq_b[0] = chip->data_bus1 & 255;
            chip->ssg_freq_b[1] = chip->data_bus1 & 15;
            chip->ssg_freq_c[0] = chip->data_bus1 & 255;
            chip->ssg_freq_c[1] = chip->data_bus1 & 15;
            chip->ssg_noise = chip->data_bus1 & 31;
            chip->ssg_mode = chip->data_bus1 & 255;
            chip->ssg_level_a = chip->data_bus1 & 31;
            chip->ssg_level_b = chip->data_bus1 & 31;
            chip->ssg_level_c = chip->data_bus1 & 31;
            chip->ssg_env[0] = chip->data_bus1 & 255;
            chip->ssg_env[1] = chip->data_bus1 & 255;
            chip->ssg_envmode = chip->data_bus1 & 15;
            chip->o_gpio_a = chip->data_bus1 & 255;
            chip->o_gpio_b = chip->data_bus1 & 255;

            chip->ssg_egtrig = 1;
        }
        else if (chip->ssg_ssg_addr && chip->ssg_write1)
        {
            switch (chip->ssg_address)
            {
                case 0x0:
                    chip->ssg_freq_a[0] = chip->data_bus1 & 255;
                    break;
                case 0x1:
                    chip->ssg_freq_a[1] = chip->data_bus1 & 15;
                    break;
                case 0x2:
                    chip->ssg_freq_b[0] = chip->data_bus1 & 255;
                    break;
                case 0x3:
                    chip->ssg_freq_b[1] = chip->data_bus1 & 15;
                    break;
                case 0x4:
                    chip->ssg_freq_c[0] = chip->data_bus1 & 255;
                    break;
                case 0x5:
                    chip->ssg_freq_c[1] = chip->data_bus1 & 15;
                    break;
                case 0x6:
                    chip->ssg_noise = chip->data_bus1 & 31;
                    break;
                case 0x7:
                    chip->ssg_mode = chip->data_bus1 & 255;
                    break;
                case 0x8:
                    chip->ssg_level_a = chip->data_bus1 & 31;
                    break;
                case 0x9:
                    chip->ssg_level_b = chip->data_bus1 & 31;
                    break;
                case 0xa:
                    chip->ssg_level_c = chip->data_bus1 & 31;
                    break;
                case 0xb:
                    chip->ssg_env[0] = chip->data_bus1 & 255;
                    break;
                case 0xc:
                    chip->ssg_env[1] = chip->data_bus1 & 255;
                    break;
                case 0xd:
                    chip->ssg_envmode = chip->data_bus1 & 15;
                    chip->ssg_egtrig = 1;
                    break;
                case 0xe:
                    chip->o_gpio_a = chip->data_bus1 & 255;
                    break;
                case 0xf:
                    chip->o_gpio_b = chip->data_bus1 & 255;
                    break;
            }
        }
        if (chip->ssg_ssg_addr && chip->ssg_read1)
        {
            switch (chip->ssg_address)
            {
                case 0x0:
                    chip->data_bus1 &= ~255;
                    chip->data_bus1 |= chip->ssg_freq_a[0] & 255;
                    break;
                case 0x1:
                    chip->data_bus1 &= ~15;
                    chip->data_bus1 |= chip->ssg_freq_a[1] & 15;
                    break;
                case 0x2:
                    chip->data_bus1 &= ~255;
                    chip->data_bus1 |= chip->ssg_freq_b[0] & 255;
                    break;
                case 0x3:
                    chip->data_bus1 &= ~15;
                    chip->data_bus1 |= chip->ssg_freq_b[1] & 15;
                    break;
                case 0x4:
                    chip->data_bus1 &= ~255;
                    chip->data_bus1 |= chip->ssg_freq_c[0] & 255;
                    break;
                case 0x5:
                    chip->data_bus1 &= ~15;
                    chip->data_bus1 |= chip->ssg_freq_c[1] & 15;
                    break;
                case 0x6:
                    chip->data_bus1 &= ~31;
                    chip->data_bus1 |= chip->ssg_noise & 31;
                    break;
                case 0x7:
                    chip->data_bus1 &= ~255;
                    chip->data_bus1 |= chip->ssg_mode & 255;
                    break;
                case 0x8:
                    chip->data_bus1 &= ~31;
                    chip->data_bus1 |= chip->ssg_level_a & 31;
                    break;
                case 0x9:
                    chip->data_bus1 &= ~31;
                    chip->data_bus1 |= chip->ssg_level_b & 31;
                    break;
                case 0xa:
                    chip->data_bus1 &= ~31;
                    chip->data_bus1 |= chip->ssg_level_c & 31;
                    break;
                case 0xb:
                    chip->data_bus1 &= ~255;
                    chip->data_bus1 |= chip->ssg_env[0] & 255;
                    break;
                case 0xc:
                    chip->data_bus1 &= ~255;
                    chip->data_bus1 |= chip->ssg_env[1] & 255;
                    break;
                case 0xd:
                    chip->data_bus1 &= ~15;
                    chip->data_bus1 |= chip->ssg_envmode & 15;
                    break;
                case 0xe:
                    chip->data_bus1 &= ~15;
                    chip->data_bus1 |= chip->input.gpio_a & 15;
                    break;
                case 0xf:
                    chip->data_bus1 &= ~255;
                    chip->data_bus1 |= chip->input.gpio_b & 255;
                    break;
            }
        }

        if (chip->ssg_clk1)
        {
            chip->ssg_envadd = (chip->ssg_eg_of[1] & 4) != 0 && (chip->ssg_sel[1] & 8) != 0;
            chip->ssg_envcnt[1] = chip->ssg_hold[0] ? 31 : chip->ssg_envcnt[0];
            chip->ssg_dir[1] = chip->ssg_dir[0];
            chip->ssg_hold[1] = chip->ssg_hold[0];
            chip->ssg_t2[1] = chip->ssg_t2[0];

            chip->ssg_egtrig_s = chip->ssg_egtrig;

            chip->ssg_eg_sel_l = (chip->ssg_sel[1] & 8) != 0;

            chip->ssg_sel[0] = chip->ssg_sel[1] << 1;
            chip->ssg_sel[0] |= (chip->ssg_sel[1] & 7) == 0 && !chip->ic;

            chip->ssg_sel_freq = 0;
            if (chip->ssg_sel[1] & 1)
                chip->ssg_sel_freq |= (chip->ssg_freq_c[1] << 8) | chip->ssg_freq_c[0];
            if (chip->ssg_sel[1] & 2)
                chip->ssg_sel_freq |= (chip->ssg_freq_b[1] << 8) | chip->ssg_freq_b[0];
            if (chip->ssg_sel[1] & 4)
                chip->ssg_sel_freq |= (chip->ssg_freq_a[1] << 8) | chip->ssg_freq_a[0];
            if (chip->ssg_sel[1] & 8)
                chip->ssg_sel_freq |= (chip->ssg_env[1] << 8) | chip->ssg_env[0];

            int cnt = chip->ssg_freq_cnt[7] + 1;
            int of = (cnt & 0x1000) != 0;
            chip->ssg_freq_cnt[0] = cnt & 0xfff;
            chip->ssg_freq_cnt[4] = chip->ssg_freq_cnt[3];
            chip->ssg_freq_cnt[6] = chip->ssg_freq_cnt[5];

            chip->ssg_cnt_of[1] = chip->ssg_cnt_of[0];

            if (chip->ssg_sel[1] & 8)
                chip->ssg_sign_toggle = chip->ssg_cnt_of[0] & 7;
            else
                chip->ssg_sign_toggle = 0;

            chip->ssg_freq_cnt2[1] = chip->ssg_freq_cnt2[0];
            chip->ssg_freq_cnt2[3] = chip->ssg_freq_cnt2[2];
            chip->ssg_freq_cnt2[5] = chip->ssg_freq_cnt2[4];
            chip->ssg_freq_cnt2[7] = chip->ssg_freq_cnt2[6];

            chip->ssg_sign[1] = chip->ssg_sign[0];

            int cnt_of = !chip->ssg_cnt_reload && (chip->ssg_sel_freq_l - chip->ssg_cnt_of_l < chip->ssg_freq_cnt2[0]);

            chip->ssg_cnt2_add = (chip->ssg_sel[1] & 8) != 0 && of;

            chip->ssg_sel_eg_l[0] = (chip->ssg_sel[1] & 8) != 0;

            chip->ssg_eg_of[0] = (chip->ssg_eg_of[1] << 1) | cnt_of;

            int fr_rst = chip->ssg_ch_of || ((cnt_of || chip->ssg_cnt_reload) && chip->ssg_sel_eg_l[1]);

            chip->ssg_freq_cnt[2] = fr_rst ? 0 : chip->ssg_freq_cnt[1];

            chip->ssg_fr_rst_l = fr_rst;

            chip->ssg_noise_add = (chip->ssg_sel[1] & 1) != 0;

            chip->ssg_noise_cnt[1] = chip->ssg_noise_cnt[0];

            int noise_of = chip->ssg_noise <= (chip->ssg_noise_cnt[0] >> 1);

            chip->ssg_noise_of = noise_of && chip->ssg_noise_of_low;

            chip->ssg_test = chip->input.test;

            chip->ssg_noise_step = chip->ssg_noise_of || !chip->ssg_test;
        }
        if (chip->ssg_clk2)
        {
            int sum = chip->ssg_envcnt[1] + chip->ssg_envadd;
            int of = (sum & 32) != 0;
            chip->ssg_envcnt[0] = chip->ssg_egtrig_s ? 0 : sum & 31;

            int dir = of && (chip->ssg_envmode & 2) != 0 && !chip->ssg_hold[1];
            chip->ssg_dir[0] = chip->ssg_egtrig_s ? 0 : chip->ssg_dir[1] ^ dir;

            chip->ssg_hold[0] = chip->ssg_egtrig_s ? 0 : chip->ssg_hold[1] || (of && (chip->ssg_envmode & 1) != 0);

            chip->ssg_t2[0] = chip->ssg_egtrig_s ? 0 : chip->ssg_t2[1] || of;

            chip->ssg_egtrig_rst = chip->ssg_egtrig_s && chip->ssg_eg_sel_l;


            chip->ssg_sel[1] = chip->ssg_sel[0];

            chip->ssg_freq_cnt[1] = chip->ssg_freq_cnt[0];
            chip->ssg_freq_cnt[3] = chip->ssg_freq_cnt[2];
            chip->ssg_freq_cnt[5] = chip->ssg_freq_cnt[4];
            chip->ssg_freq_cnt[7] = chip->ssg_freq_cnt[6];

            int cnt_of = (chip->ssg_sel_freq & 0xfff) <= chip->ssg_freq_cnt[0];

            chip->ssg_cnt_of_l = cnt_of;
            chip->ssg_sel_freq_l = (chip->ssg_sel_freq >> 12) & 0xf;

            chip->ssg_cnt_of[0] = (chip->ssg_cnt_of[1] << 1) | cnt_of;

            if (chip->ic)
                chip->ssg_sign[0] = 0;
            else
                chip->ssg_sign[0] = chip->ssg_sign[1] ^ chip->ssg_sign_toggle;

            int cnt = chip->ssg_freq_cnt2[7] + chip->ssg_cnt2_add;
            chip->ssg_freq_cnt2[0] = cnt & 0xf;
            chip->ssg_freq_cnt2[2] = chip->ssg_fr_rst_l ? 0 : chip->ssg_freq_cnt2[1];
            chip->ssg_freq_cnt2[4] = chip->ssg_freq_cnt2[3];
            chip->ssg_freq_cnt2[6] = chip->ssg_freq_cnt2[5];

            chip->ssg_sel_eg_l[1] = chip->ssg_sel_eg_l[0];

            chip->ssg_ch_of = (!chip->ssg_sel_eg_l[0] && cnt_of) || chip->ic;

            chip->ssg_cnt_reload = chip->ssg_sel_eg_l[0] && chip->ssg_egtrig_s;

            chip->ssg_eg_of[1] = chip->ssg_eg_of[0];


            chip->ssg_noise_cnt[0] = chip->ssg_noise_step ? 0 : ((chip->ssg_noise_cnt[1] + chip->ssg_noise_add) & 63);

            chip->ssg_noise_of_low = (chip->ssg_noise_cnt[1] & 1) != 0 && chip->ssg_noise_add;

        }

        if (!chip->ssg_noise_step)
        {
            int bit = ((chip->ssg_noise_lfsr[1] >> 16) ^ (chip->ssg_noise_lfsr[1] >> 13)) & 1;

            if ((chip->ssg_noise_lfsr[1] & 0x1ffff) == 0)
                bit |= 1;

            chip->ssg_noise_lfsr[0] = (chip->ssg_noise_lfsr[1] << 1) | bit;
        }
        else
        {
            chip->ssg_noise_lfsr[1] = chip->ic ? 0 : chip->ssg_noise_lfsr[0];
        }

        if (chip->ssg_clk2)
            chip->ssg_noise_bit = (chip->ssg_noise_lfsr[1] >> 16) & 1;

        int envlevel = chip->ssg_hold[0] ? 31 : chip->ssg_envcnt[0];
        envlevel = (chip->ssg_dir[0] ^ ((chip->ssg_envmode >> 2) & 1)) == 0 ? (envlevel ^ 31) : envlevel;
        envlevel = (chip->ssg_t2[0] && (chip->ssg_envmode & 8) == 0) ? 0 : envlevel;

        int vol_a = (chip->ssg_level_a & 0x10) != 0 ? envlevel : (((chip->ssg_level_a & 15) << 1) | 1);
        int vol_b = (chip->ssg_level_b & 0x10) != 0 ? envlevel : (((chip->ssg_level_b & 15) << 1) | 1);
        int vol_c = (chip->ssg_level_c & 0x10) != 0 ? envlevel : (((chip->ssg_level_c & 15) << 1) | 1);

        int sign_a = ((chip->ssg_mode & 1) == 0 && (chip->ssg_sign[0] & 1) != 0) || ((chip->ssg_mode & 8) == 0 && chip->ssg_noise_bit);
        int sign_b = ((chip->ssg_mode & 2) == 0 && (chip->ssg_sign[0] & 2) != 0) || ((chip->ssg_mode & 16) == 0 && chip->ssg_noise_bit);
        int sign_c = ((chip->ssg_mode & 4) == 0 && (chip->ssg_sign[0] & 4) != 0) || ((chip->ssg_mode & 32) == 0 && chip->ssg_noise_bit);

        static const float volume_lut_ay[32] = {
          0.0000,
          0.0000,
          0.0106,
          0.0106,
          0.0150,
          0.0150,
          0.0222,
          0.0222,
          0.0320,
          0.0320,
          0.0466,
          0.0466,
          0.0665,
          0.0665,
          0.1039,
          0.1039,
          0.1237,
          0.1237,
          0.1986,
          0.1986,
          0.2803,
          0.2803,
          0.3548,
          0.3548,
          0.4702,
          0.4702,
          0.6030,
          0.6030,
          0.7530,
          0.7530,
          0.9250,
          0.9250
        };

        static const float volume_lut_ssg[32] = {
            0.0000, 0.0000, 0.0049, 0.0075, 0.0105, 0.0131, 0.0156, 0.0183,
            0.0228, 0.0276, 0.0321, 0.0367, 0.0448, 0.0535, 0.0626, 0.0713,
            0.0884, 0.1057, 0.1225, 0.1392, 0.1691, 0.2013, 0.2348, 0.2670,
            0.3307, 0.3951, 0.4573, 0.5196, 0.6316, 0.7528, 0.8787, 1.0000
        };

        if (chip->type & 1) {
          chip->o_analog[0] = volume_lut_ay[sign_a ? 0 : vol_a] * 11806;
          chip->o_analog[1] = volume_lut_ay[sign_b ? 0 : vol_b] * 11806;
          chip->o_analog[2] = volume_lut_ay[sign_c ? 0 : vol_c] * 11806;
        } else {
          chip->o_analog[0] = volume_lut_ssg[sign_a ? 0 : vol_a] * 10922;
          chip->o_analog[1] = volume_lut_ssg[sign_b ? 0 : vol_b] * 10922;
          chip->o_analog[2] = volume_lut_ssg[sign_c ? 0 : vol_c] * 10922;
        }
    }

    {
        chip->read_bus = 0; // FIXME
        if (chip->ssg_read1)
            chip->read_bus = chip->data_bus1 & 255;

        chip->o_data = chip->read_bus;
    }

    if (clk)
        chip->input.cs=1;
}

void SSG_Reset(ssg_t* chip) {
  memset(chip,0,sizeof(ssg_t));

  chip->input.test=1;
  chip->input.ic=1;
  SSG_Clock(chip,0);
  SSG_Clock(chip,1);

  chip->input.ic=0;
  SSG_Clock(chip,0);
  SSG_Clock(chip,1);

  chip->input.ic=1;
  SSG_Clock(chip,0);
  SSG_Clock(chip,1);

  chip->input.cs=1;
}

void SSG_Write(ssg_t* chip, unsigned char addr, unsigned char val) {
  chip->input.cs=0;
  chip->input.rd=1;
  chip->input.wr=0;

  chip->input.a0=addr;
  chip->input.data=val;
}

void SSG_SetType(ssg_t* chip, int type) {
  chip->type=type;
}
